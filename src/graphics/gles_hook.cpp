// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — OpenGL ES Hook Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "graphics/gles_hook.h"
#include "mesh/mesh_swapper.h"
#include "textures/texture_swapper.h"
#include "utils/logger.h"

#if __has_include(<dobby.h>)
#   include <dobby.h>
#   define GIMI_DOBBY_AVAILABLE 1
#else
#   define GIMI_DOBBY_AVAILABLE 0
#endif

#if __has_include(<xxhash.h>)
#   include <xxhash.h>
#else
extern "C" {
    unsigned long long XXH64(const void* input, size_t length, unsigned long long seed);
}
#endif

#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <cstdint>
#include <unordered_map>
#include <mutex>

namespace gimi {

static bool s_gles_installed = false;

// ─── Typedefs ────────────────────────────────────────────────────────────────
typedef void (*PFNGLDRAWELEMENTS)(GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void (*PFNGLDRAWELEMENTSINSTANCED)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
typedef void (*PFNGLBINDBUFFER)(GLenum target, GLuint buffer);
typedef void (*PFNGLBUFFERDATA)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (*PFNGLBUFFERSUBDATA)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void (*PFNGLTEXIMAGE2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (*PFNGLCOMPRESSEDTEXIMAGE2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
typedef void (*PFNGLBINDTEXTURE)(GLenum target, GLuint texture);

// ─── Original function pointers ──────────────────────────────────────────────
static PFNGLDRAWELEMENTS orig_glDrawElements = nullptr;
static PFNGLDRAWELEMENTSINSTANCED orig_glDrawElementsInstanced = nullptr;
static PFNGLBINDBUFFER orig_glBindBuffer = nullptr;
static PFNGLBUFFERDATA orig_glBufferData = nullptr;
static PFNGLBUFFERSUBDATA orig_glBufferSubData = nullptr;
static PFNGLTEXIMAGE2D orig_glTexImage2D = nullptr;
static PFNGLCOMPRESSEDTEXIMAGE2D orig_glCompressedTexImage2D = nullptr;
static PFNGLBINDTEXTURE orig_glBindTexture = nullptr;

// ─── Hook Symbols for DobbyDestroy ───────────────────────────────────────────
static void* sym_glDrawElements = nullptr;
static void* sym_glDrawElementsInstanced = nullptr;
static void* sym_glBindBuffer = nullptr;
static void* sym_glBufferData = nullptr;
static void* sym_glBufferSubData = nullptr;
static void* sym_glTexImage2D = nullptr;
static void* sym_glCompressedTexImage2D = nullptr;
static void* sym_glBindTexture = nullptr;

// ─── State Tracking ──────────────────────────────────────────────────────────
static GLuint s_active_vbo = 0;
static GLuint s_active_ibo = 0;
static GLuint s_active_texture_2d = 0;

static std::unordered_map<GLuint, uint32_t> s_buffer_hashes;
static std::unordered_map<GLuint, uint32_t> s_texture_hashes;
static std::mutex s_hash_mutex;

static GLuint get_bound_buffer(GLenum target) {
    if (target == GL_ARRAY_BUFFER) return s_active_vbo;
    if (target == GL_ELEMENT_ARRAY_BUFFER) return s_active_ibo;
    return 0;
}

// ─── Hooks ───────────────────────────────────────────────────────────────────
static void hooked_glBindBuffer(GLenum target, GLuint buffer) {
    if (target == GL_ARRAY_BUFFER) s_active_vbo = buffer;
    else if (target == GL_ELEMENT_ARRAY_BUFFER) s_active_ibo = buffer;
    if (orig_glBindBuffer) orig_glBindBuffer(target, buffer);
}

static void hooked_glBindTexture(GLenum target, GLuint texture) {
    if (target == GL_TEXTURE_2D) s_active_texture_2d = texture;
    if (orig_glBindTexture) orig_glBindTexture(target, texture);
}

static void hooked_glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
    if (orig_glBufferData) orig_glBufferData(target, size, data, usage);
    
    GLuint buffer = get_bound_buffer(target);
    if (buffer && data && size > 0) {
        size_t sample_size = (size > 4096) ? 4096 : size;
        uint32_t hash = static_cast<uint32_t>(XXH64(data, sample_size, 0));
        std::lock_guard<std::mutex> lock(s_hash_mutex);
        s_buffer_hashes[buffer] = hash;
    }
}

static void hooked_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
    if (orig_glBufferSubData) orig_glBufferSubData(target, offset, size, data);
}

static void try_swap_and_draw(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, bool is_instanced) {
    uint32_t active_vb_hash = 0;
    {
        std::lock_guard<std::mutex> lock(s_hash_mutex);
        auto it = s_buffer_hashes.find(s_active_vbo);
        if (it != s_buffer_hashes.end()) active_vb_hash = it->second;
    }

    if (active_vb_hash != 0) {
        auto override_result = MeshSwapper::instance().try_swap(active_vb_hash);
        if (override_result.should_override) {
            GLuint ov_vb = static_cast<GLuint>(reinterpret_cast<uintptr_t>(override_result.override_vb));
            GLuint ov_ib = static_cast<GLuint>(reinterpret_cast<uintptr_t>(override_result.override_ib));
            
            if (ov_vb) orig_glBindBuffer(GL_ARRAY_BUFFER, ov_vb);
            if (ov_ib) orig_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ov_ib);
            
            uint32_t override_count = override_result.index_count > 0 ? override_result.index_count : count;
            const void* override_indices = reinterpret_cast<const void*>(static_cast<uintptr_t>(override_result.first_index * (type == GL_UNSIGNED_SHORT ? 2 : 4)));
            
            if (is_instanced) {
                orig_glDrawElementsInstanced(mode, override_count, type, override_indices, instancecount);
            } else {
                orig_glDrawElements(mode, override_count, type, override_indices);
            }

            // Restore bindings
            if (ov_vb) orig_glBindBuffer(GL_ARRAY_BUFFER, s_active_vbo);
            if (ov_ib) orig_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_active_ibo);
            return;
        }
    }
    
    if (is_instanced) {
        if (orig_glDrawElementsInstanced) orig_glDrawElementsInstanced(mode, count, type, indices, instancecount);
    } else {
        if (orig_glDrawElements) orig_glDrawElements(mode, count, type, indices);
    }
}

static void hooked_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
    try_swap_and_draw(mode, count, type, indices, 0, false);
}

static void hooked_glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) {
    try_swap_and_draw(mode, count, type, indices, instancecount, true);
}

static void hooked_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
    if (target == GL_TEXTURE_2D && pixels && s_active_texture_2d != 0) {
        size_t sample_size = (width * height > 4096) ? 4096 : (width * height);
        uint32_t hash = static_cast<uint32_t>(XXH64(pixels, sample_size, 0));
        std::lock_guard<std::mutex> lock(s_hash_mutex);
        s_texture_hashes[s_active_texture_2d] = hash;
    }
    
    // TextureSwapper override logic can be applied here for GLES in the future.
    if (orig_glTexImage2D) orig_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

static void hooked_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) {
    if (orig_glCompressedTexImage2D) orig_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

bool GLESHook::install() noexcept {
    if (s_gles_installed) return true;

#if GIMI_DOBBY_AVAILABLE
    void* libgles = dlopen("libGLESv3.so", RTLD_NOW | RTLD_NOLOAD);
    if (!libgles) {
        libgles = dlopen("libGLESv2.so", RTLD_NOW | RTLD_NOLOAD);
    }
    
    if (!libgles) {
        LOGW("GLESHook::install: libGLESv3.so / libGLESv2.so not found.");
        return false;
    }

    auto hook_func = [&](const char* name, void* replacement, void** original, void** sym_out) {
        void* sym = dlsym(libgles, name);
        if (sym) {
            int ret = DobbyHook(sym, replacement, original);
            if (ret == 0) {
                LOGI("GLESHook: Hooked %s", name);
                if (sym_out) *sym_out = sym;
            } else {
                LOGE("GLESHook: Failed to hook %s (ret=%d)", name, ret);
            }
        } else {
            LOGW("GLESHook: Symbol %s not found in GLES library.", name);
        }
    };

    hook_func("glDrawElements", (void*)hooked_glDrawElements, (void**)&orig_glDrawElements, &sym_glDrawElements);
    hook_func("glDrawElementsInstanced", (void*)hooked_glDrawElementsInstanced, (void**)&orig_glDrawElementsInstanced, &sym_glDrawElementsInstanced);
    hook_func("glBindBuffer", (void*)hooked_glBindBuffer, (void**)&orig_glBindBuffer, &sym_glBindBuffer);
    hook_func("glBufferData", (void*)hooked_glBufferData, (void**)&orig_glBufferData, &sym_glBufferData);
    hook_func("glBufferSubData", (void*)hooked_glBufferSubData, (void**)&orig_glBufferSubData, &sym_glBufferSubData);
    hook_func("glTexImage2D", (void*)hooked_glTexImage2D, (void**)&orig_glTexImage2D, &sym_glTexImage2D);
    hook_func("glCompressedTexImage2D", (void*)hooked_glCompressedTexImage2D, (void**)&orig_glCompressedTexImage2D, &sym_glCompressedTexImage2D);
    hook_func("glBindTexture", (void*)hooked_glBindTexture, (void**)&orig_glBindTexture, &sym_glBindTexture);

    dlclose(libgles);
    s_gles_installed = true;
    return true;
#else
    LOGW("GLESHook::install: Dobby not available.");
    return false;
#endif
}

void GLESHook::uninstall() noexcept {
    if (!s_gles_installed) return;
#if GIMI_DOBBY_AVAILABLE
    auto destroy_hook = [](void* sym) {
        if (sym) {
            DobbyDestroy(sym);
        }
    };
    destroy_hook(sym_glDrawElements);
    destroy_hook(sym_glDrawElementsInstanced);
    destroy_hook(sym_glBindBuffer);
    destroy_hook(sym_glBufferData);
    destroy_hook(sym_glBufferSubData);
    destroy_hook(sym_glTexImage2D);
    destroy_hook(sym_glCompressedTexImage2D);
    destroy_hook(sym_glBindTexture);
#endif
    s_gles_installed = false;
    LOGI("GLESHook::uninstall: GLES hooks removed.");
}

bool GLESHook::is_installed() noexcept {
    return s_gles_installed;
}

} // namespace gimi
