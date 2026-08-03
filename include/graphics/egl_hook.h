#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — EGL / OpenGL ES Hook
//
// Intercepts eglGetProcAddress via Dobby (ARM64 inline hooking library)
// to reroute GLES function lookups through our dispatch table.
//
// This provides transparent support for Genshin Impact devices that
// run the game via OpenGL ES instead of Vulkan.
//
// Usage:
//   gimi::EGLHook::install();   // called from GraphicsDispatcher
//   gimi::EGLHook::uninstall(); // called on library unload
// ─────────────────────────────────────────────────────────────────────────────

#include <EGL/egl.h>
#include <functional>

namespace gimi {

// ─── EGL Function Override Table ─────────────────────────────────────────────
// Holds original function pointers restored on uninstall.
struct EGLDispatchTable {
    // Original eglGetProcAddress obtained before hooking.
    PFNEGLGETPROCADDRESSPROC orig_eglGetProcAddress = nullptr;
};

// ─── EGL Hook Manager ────────────────────────────────────────────────────────
class EGLHook {
public:
    // Install the eglGetProcAddress hook via Dobby.
    // Returns true on success, false if libEGL.so is not found or
    // Dobby hook installation fails (game may not use GLES).
    static bool install() noexcept;

    // Remove the hook and restore original function pointer.
    static void uninstall() noexcept;

    static bool is_installed() noexcept;

    // Retrieve the original (pre-hook) eglGetProcAddress.
    // Used internally to forward non-overridden symbols.
    static PFNEGLGETPROCADDRESSPROC original_get_proc_address() noexcept;

private:
    static EGLDispatchTable s_table;
    static bool s_installed;
};

} // namespace gimi
