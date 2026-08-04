#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — OpenGL ES Hook
//
// Intercepts OpenGL ES API calls using Dobby (ARM64 inline hooking) to
// support modding and asset swapping for GLES-based games.
// ─────────────────────────────────────────────────────────────────────────────

namespace gimi {

class GLESHook {
public:
    // Install the OpenGL ES hooks via Dobby.
    // Returns true on success, false if Dobby is not available or libGLES is missing.
    static bool install() noexcept;

    // Remove the hooks.
    static void uninstall() noexcept;

    static bool is_installed() noexcept;
};

} // namespace gimi
