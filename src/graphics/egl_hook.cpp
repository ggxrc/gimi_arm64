// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — EGL / OpenGL ES Hook Implementation
//
// Uses Dobby (ARM64 inline hook library) to intercept eglGetProcAddress
// from libEGL.so at runtime.  All pointer patching happens in RAM only.
//
// Non-destructive guarantee:
//   - Dobby patches the PLT/GOT entry in our own process memory.
//   - No ELF file on disk is modified.
//   - Original pointer is stored in EGLDispatchTable and restored on uninstall.
// ─────────────────────────────────────────────────────────────────────────────

#include "graphics/egl_hook.h"
#include "utils/logger.h"

// Dobby ARM64 inline hooking library (submodule or NDK prebuilt).
// If Dobby is not yet integrated, the install() stub returns false gracefully.
#if __has_include(<dobby.h>)
#   include <dobby.h>
#   define GIMI_DOBBY_AVAILABLE 1
#else
#   define GIMI_DOBBY_AVAILABLE 0
#endif

#include <dlfcn.h>   // dlopen / dlsym
#include <cstring>

namespace gimi {

// ─── Static Storage ───────────────────────────────────────────────────────────
EGLDispatchTable EGLHook::s_table{};
bool             EGLHook::s_installed = false;

// ─── Hooked eglGetProcAddress ─────────────────────────────────────────────────
// This function replaces eglGetProcAddress in the process.
// For now we forward all symbols transparently; Phase 3/4 will
// intercept specific GLES draw-call and texture entry-points here.
static __eglMustCastToProperFunctionPointerType
hooked_eglGetProcAddress(const char* procname) noexcept {
    LOGD("eglGetProcAddress intercepted: %s", procname);

    // Forward to the original eglGetProcAddress.
    auto orig = EGLHook::original_get_proc_address();
    if (orig) return orig(procname);

    LOGW("eglGetProcAddress: no original function pointer, returning nullptr for '%s'", procname);
    return nullptr;
}

// ─── EGLHook::install ─────────────────────────────────────────────────────────
bool EGLHook::install() noexcept {
    if (s_installed) {
        LOGD("EGLHook::install: already installed, skipping.");
        return true;
    }

#if GIMI_DOBBY_AVAILABLE
    // Resolve the real eglGetProcAddress from libEGL.so.
    void* libegl = dlopen("libEGL.so", RTLD_NOW | RTLD_NOLOAD);
    if (!libegl) {
        LOGW("EGLHook::install: libEGL.so not found — device may use Vulkan only.");
        return false;
    }

    auto* orig = reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(
            dlsym(libegl, "eglGetProcAddress"));
    dlclose(libegl);

    if (!orig) {
        LOGW("EGLHook::install: eglGetProcAddress symbol not found in libEGL.so.");
        return false;
    }

    // Install Dobby hook: replaces orig with hooked_eglGetProcAddress.
    int ret = DobbyHook(reinterpret_cast<void*>(orig),
                        reinterpret_cast<void*>(hooked_eglGetProcAddress),
                        reinterpret_cast<void**>(&s_table.orig_eglGetProcAddress));
    if (ret != 0) {
        LOGE("EGLHook::install: DobbyHook failed (ret=%d).", ret);
        return false;
    }

    s_installed = true;
    LOGI("EGLHook::install: eglGetProcAddress hook installed successfully.");
    return true;
#else
    // Dobby not yet integrated; log a warning and continue.
    LOGW("EGLHook::install: Dobby not available — EGL interception skipped. "
         "Add Dobby as a submodule and rebuild to enable GLES support.");
    return false;
#endif
}

// ─── EGLHook::uninstall ───────────────────────────────────────────────────────
void EGLHook::uninstall() noexcept {
    if (!s_installed) return;

#if GIMI_DOBBY_AVAILABLE
    if (s_table.orig_eglGetProcAddress) {
        DobbyDestroy(reinterpret_cast<void*>(s_table.orig_eglGetProcAddress));
    }
#endif
    s_installed = false;
    s_table = {};
    LOGI("EGLHook::uninstall: EGL hook removed.");
}

bool EGLHook::is_installed() noexcept {
    return s_installed;
}

PFNEGLGETPROCADDRESSPROC EGLHook::original_get_proc_address() noexcept {
    return s_table.orig_eglGetProcAddress;
}

} // namespace gimi
