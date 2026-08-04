// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Unified Graphics Dispatcher Implementation
//
// Non-destructive guarantee:
//   All detection and hooking happens in RAM only.
//   No APK, ELF, or on-disk file is modified.
// ─────────────────────────────────────────────────────────────────────────────

#include "graphics/graphics_dispatcher.h"
#include "graphics/egl_hook.h"
#include "graphics/gles_hook.h"
#include "config/mod_config.h"    // Phase 2: load mod .ini files
#include "utils/logger.h"

#include <dlfcn.h>   // dlopen / RTLD_NOW / RTLD_NOLOAD

namespace gimi {

// ─── Singleton ────────────────────────────────────────────────────────────────
GraphicsDispatcher& GraphicsDispatcher::instance() noexcept {
    static GraphicsDispatcher s_dispatcher;
    return s_dispatcher;
}

// ─── Helper: probe if a shared library is already resident ───────────────────
static bool is_library_loaded(const char* libname) noexcept {
    // RTLD_NOLOAD never loads the library — it just checks whether it is
    // already mapped into this process.  Returns nullptr if absent.
    void* handle = dlopen(libname, RTLD_NOW | RTLD_NOLOAD);
    if (handle) {
        dlclose(handle);
        return true;
    }
    return false;
}

// ─── GraphicsDispatcher::init ─────────────────────────────────────────────────
void GraphicsDispatcher::init() noexcept {
    LOGI("GraphicsDispatcher: probing loaded graphics libraries…");

    // Detect which API libraries are present in the process.
    // Vulkan layer hooking is handled by the loader via vk_layer.json;
    // we only need to actively install Dobby hooks for EGL.
    bool vulkan_present = is_library_loaded("libvulkan.so");
    bool egl_present    = is_library_loaded("libEGL.so");

    LOGI("GraphicsDispatcher: libvulkan.so=%s  libEGL.so=%s",
         vulkan_present ? "loaded" : "absent",
         egl_present    ? "loaded" : "absent");

    // ── Vulkan path ───────────────────────────────────────────────────────────
    // The Vulkan layer intercept is installed automatically by the Android
    // Vulkan loader when it reads our layer's VkLayer_gimi_arm64.json manifest.
    // Nothing additional to install here at runtime for Vulkan.
    if (vulkan_present) {
        m_vulkan_ok = true;
        LOGI("GraphicsDispatcher: Vulkan layer active (loader-managed).");
    }

    // ── Phase 2: Load mod configuration ─────────────────────────────────────
    // Scan GIMI_MODS_DIR for .ini files and populate ModDatabase.
    // Must happen before any render frames so the hash lookups work from frame 1.
    ModConfig::instance().load(GIMI_MODS_DIR);

    // EGL / OpenGL ES path
    // Dobby-based hook on eglGetProcAddress is installed manually.
    if (egl_present) {
        m_egl_ok = EGLHook::install();
        if (m_egl_ok) {
            LOGI("GraphicsDispatcher: EGL hook active.");
            // Also install GLESHook for direct libGLES interception
            GLESHook::install();
        } else {
            LOGW("GraphicsDispatcher: EGL hook installation failed — "
                 "GLES interception unavailable (Dobby may be missing).");
        }
    }

    // ── Determine active API ──────────────────────────────────────────────────
    if (m_vulkan_ok && m_egl_ok) {
        m_api = GraphicsAPI::Both;
        LOGI("GraphicsDispatcher: dual-API mode (Vulkan + GLES).");
    } else if (m_vulkan_ok) {
        m_api = GraphicsAPI::Vulkan;
        LOGI("GraphicsDispatcher: Vulkan-only mode.");
    } else if (m_egl_ok) {
        m_api = GraphicsAPI::GLES;
        LOGI("GraphicsDispatcher: GLES-only mode.");
    } else {
        m_api = GraphicsAPI::Unknown;
        LOGW("GraphicsDispatcher: no known graphics API detected — "
             "interception inactive. This device may not be supported yet.");
    }
}

// ─── GraphicsDispatcher::shutdown ────────────────────────────────────────────
void GraphicsDispatcher::shutdown() noexcept {
    LOGI("GraphicsDispatcher: shutting down (API=%d)…", static_cast<int>(m_api));

    if (m_egl_ok) {
        GLESHook::uninstall();
        EGLHook::uninstall();
        m_egl_ok = false;
    }
    // Vulkan layer tear-down is handled by the Vulkan loader when it
    // destroys instances/devices through vkDestroyInstance / vkDestroyDevice.
    m_vulkan_ok = false;
    m_api = GraphicsAPI::Unknown;

    LOGI("GraphicsDispatcher: shutdown complete.");
}

// ─── Free-function Thunks ─────────────────────────────────────────────────────
void graphics_dispatcher_init() noexcept {
    GraphicsDispatcher::instance().init();
}

void graphics_dispatcher_shutdown() noexcept {
    GraphicsDispatcher::instance().shutdown();
}

} // namespace gimi
