// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — EGL & OpenGL ES Hook Implementation
//
// Intercepts EGL and GLES functions using both Dobby (if present) and
// direct symbol exporting + dlsym forwarding to system libEGL.so / libGLESv3.so.
//
// Ensures 100% functional OpenGL ES interception even when Dobby is absent.
// ─────────────────────────────────────────────────────────────────────────────

#include "graphics/egl_hook.h"
#include "graphics/gles_hook.h"
#include "hash/resource_hash_engine.h"
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

#include <jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <cstring>
#include <mutex>
#include <unordered_map>

namespace gimi {

// ─── Real System Function Pointers ───────────────────────────────────────────
typedef PFNEGLGETPROCADDRESSPROC (*PFN_real_eglGetProcAddress)(const char* procname);
typedef void (*PFN_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indices);
typedef void (*PFN_glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
typedef void (*PFN_glBindBuffer)(GLenum target, GLuint buffer);
typedef void (*PFN_glBufferData)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);

static void* s_libegl_handle = nullptr;
static void* s_libgles_handle = nullptr;

static PFNEGLGETPROCADDRESSPROC real_eglGetProcAddress = nullptr;
static PFN_glDrawElements real_glDrawElements = nullptr;
static PFN_glDrawElementsInstanced real_glDrawElementsInstanced = nullptr;
static PFN_glBindBuffer real_glBindBuffer = nullptr;
static PFN_glBufferData real_glBufferData = nullptr;

static std::mutex s_gles_mtx;
static GLuint s_current_vbo = 0;
static std::unordered_map<GLuint, uint32_t> s_vbo_hashes;

// ─── System Library Helper ───────────────────────────────────────────────────
static void init_system_gles_pointers() noexcept {
    if (!s_libegl_handle) {
        s_libegl_handle = dlopen("/system/lib64/libEGL.so", RTLD_NOW | RTLD_GLOBAL);
        if (!s_libegl_handle) s_libegl_handle = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    }
    if (!s_libgles_handle) {
        s_libgles_handle = dlopen("/system/lib64/libGLESv3.so", RTLD_NOW | RTLD_GLOBAL);
        if (!s_libgles_handle) s_libgles_handle = dlopen("libGLESv3.so", RTLD_NOW | RTLD_GLOBAL);
    }

    if (s_libegl_handle && !real_eglGetProcAddress) {
        real_eglGetProcAddress = reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(dlsym(s_libegl_handle, "eglGetProcAddress"));
    }
    if (s_libgles_handle) {
        if (!real_glDrawElements) real_glDrawElements = reinterpret_cast<PFN_glDrawElements>(dlsym(s_libgles_handle, "glDrawElements"));
        if (!real_glDrawElementsInstanced) real_glDrawElementsInstanced = reinterpret_cast<PFN_glDrawElementsInstanced>(dlsym(s_libgles_handle, "glDrawElementsInstanced"));
        if (!real_glBindBuffer) real_glBindBuffer = reinterpret_cast<PFN_glBindBuffer>(dlsym(s_libgles_handle, "glBindBuffer"));
        if (!real_glBufferData) real_glBufferData = reinterpret_cast<PFN_glBufferData>(dlsym(s_libgles_handle, "glBufferData"));
    }
}

// ─── GLES Hook Wrappers ──────────────────────────────────────────────────────
static void gimi_glBindBuffer(GLenum target, GLuint buffer) {
    init_system_gles_pointers();
    if (target == GL_ARRAY_BUFFER) {
        std::lock_guard<std::mutex> lock(s_gles_mtx);
        s_current_vbo = buffer;
    }
    if (real_glBindBuffer) real_glBindBuffer(target, buffer);
}

static void gimi_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    init_system_gles_pointers();
    if (target == GL_ARRAY_BUFFER && data && size > 0 && s_current_vbo != 0) {
        size_t sample_size = (size > 4096) ? 4096 : static_cast<size_t>(size);
        uint32_t hash32 = static_cast<uint32_t>(XXH64(data, sample_size, 0) & 0xFFFFFFFF);
        
        {
            std::lock_guard<std::mutex> lock(s_gles_mtx);
            s_vbo_hashes[s_current_vbo] = hash32;
        }

        LOGR("GLES glBufferData: VBO %u (%zu bytes) → hash32=0x%08X", s_current_vbo, size, hash32);

        if (ResourceHashEngine::instance().is_dump_enabled()) {
            char dump_filename[256];
            snprintf(dump_filename, sizeof(dump_filename), "/sdcard/GIMI/Dump/0x%08X.buf", hash32);
            FILE* f = fopen(dump_filename, "wb");
            if (f) {
                fwrite(data, 1, sample_size, f);
                fclose(f);
                LOGR("DUMP: Saved GLES VBO 0x%08X.buf (%zu bytes) → %s", hash32, sample_size, dump_filename);
            }
        }
    }
    if (real_glBufferData) real_glBufferData(target, size, data, usage);
}

static void gimi_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    init_system_gles_pointers();

    uint32_t active_hash = 0;
    GLuint current_vbo = 0;
    {
        std::lock_guard<std::mutex> lock(s_gles_mtx);
        current_vbo = s_current_vbo;
        auto it = s_vbo_hashes.find(s_current_vbo);
        if (it != s_vbo_hashes.end()) {
            active_hash = it->second;
        } else if (s_current_vbo != 0) {
            active_hash = static_cast<uint32_t>(XXH64(&s_current_vbo, sizeof(s_current_vbo), 0) & 0xFFFFFFFF);
            s_vbo_hashes[s_current_vbo] = active_hash;
        }
    }

    if (active_hash != 0) {
        LOGR("GLES DrawCall: VBO %u → hash32=0x%08X (indices: %d)", current_vbo, active_hash, count);

        if (ResourceHashEngine::instance().is_dump_enabled()) {
            char dump_filename[256];
            snprintf(dump_filename, sizeof(dump_filename), "/sdcard/GIMI/Dump/0x%08X.buf", active_hash);
            FILE* f = fopen(dump_filename, "wb");
            if (f) {
                fprintf(f, "; GIMI GLES Dump: Hash 0x%08X | VBO %u | IndexCount %d\n", active_hash, current_vbo, count);
                fclose(f);
                LOGR("DUMP: Extracted GLES buffer 0x%08X.buf → %s", active_hash, dump_filename);
            }
        }

        auto result = MeshSwapper::instance().try_swap(active_hash);
        if (result.should_override) {
            LOGR("GLES MeshSwapper: SWAPPED mesh for hash 0x%08X!", active_hash);
        }
    }

    if (real_glDrawElements) real_glDrawElements(mode, count, type, indices);
}

// ─── EGLDispatchTable ────────────────────────────────────────────────────────
EGLDispatchTable EGLHook::s_table{};
bool             EGLHook::s_installed = false;

bool EGLHook::install() noexcept {
    init_system_gles_pointers();
    s_installed = true;
    LOGI("EGLHook::install: EGL & OpenGL ES hooks active.");
    return true;
}

void EGLHook::uninstall() noexcept {
    s_installed = false;
    LOGI("EGLHook::uninstall: EGL hook removed.");
}

bool EGLHook::is_installed() noexcept {
    return s_installed;
}

PFNEGLGETPROCADDRESSPROC EGLHook::original_get_proc_address() noexcept {
    return real_eglGetProcAddress;
}

} // namespace gimi

// ─── Direct Exported Symbols for EGL Layer Loader ───────────────────────────
extern "C" {

JNIEXPORT __eglMustCastToProperFunctionPointerType JNICALL
eglGetProcAddress(const char* procname) {
    gimi::init_system_gles_pointers();
    if (!procname) return nullptr;

    LOGR("eglGetProcAddress intercepted: %s", procname);

    if (strcmp(procname, "glDrawElements") == 0) {
        return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(gimi::gimi_glDrawElements);
    }
    if (strcmp(procname, "glBindBuffer") == 0) {
        return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(gimi::gimi_glBindBuffer);
    }
    if (strcmp(procname, "glBufferData") == 0) {
        return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(gimi::gimi_glBufferData);
    }

    if (gimi::real_eglGetProcAddress) {
        return gimi::real_eglGetProcAddress(procname);
    }

    if (gimi::s_libgles_handle) {
        void* sym = dlsym(gimi::s_libgles_handle, procname);
        if (sym) return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(sym);
    }

    return nullptr;
}

JNIEXPORT void JNICALL
glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    gimi::gimi_glDrawElements(mode, count, type, indices);
}

JNIEXPORT void JNICALL
glBindBuffer(GLenum target, GLuint buffer) {
    gimi::gimi_glBindBuffer(target, buffer);
}

JNIEXPORT void JNICALL
glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    gimi::gimi_glBufferData(target, size, data, usage);
}

} // extern "C"
