#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Unified Graphics Dispatcher
//
// Detects at runtime which graphics API (Vulkan or OpenGL ES) has been
// initialised by the host application, then activates the appropriate
// interception layer.
//
// Design:
//   - Called from main.cpp's __attribute__((constructor)) before any
//     application render code executes.
//   - Detection is performed by probing for loaded shared libraries
//     (libvulkan.so vs libEGL.so) and checking which one is resident.
//   - Activates VulkanLayer or EGLHook accordingly; both can coexist
//     if the driver performs late API selection.
//
// API:
//   gimi::graphics_dispatcher_init();     // called on library load
//   gimi::graphics_dispatcher_shutdown(); // called on library unload
// ─────────────────────────────────────────────────────────────────────────────

namespace gimi {

// ─── API Detection Result ─────────────────────────────────────────────────────
enum class GraphicsAPI {
    Unknown = 0,
    Vulkan  = 1,
    GLES    = 2,
    Both    = 3,  // Rare: some games switch API mid-session
};

// ─── GraphicsDispatcher ───────────────────────────────────────────────────────
class GraphicsDispatcher {
public:
    static GraphicsDispatcher& instance() noexcept;

    // Probe loaded libraries and install the appropriate hooks.
    void init() noexcept;

    // Remove all hooks and free resources.
    void shutdown() noexcept;

    GraphicsAPI active_api() const noexcept { return m_api; }

private:
    GraphicsAPI m_api       = GraphicsAPI::Unknown;
    bool        m_vulkan_ok = false;
    bool        m_egl_ok    = false;
};

// ─── Free-function Thunks ─────────────────────────────────────────────────────
// Declared here so main.cpp can call them without including GraphicsDispatcher.
void graphics_dispatcher_init()     noexcept;
void graphics_dispatcher_shutdown() noexcept;

} // namespace gimi
