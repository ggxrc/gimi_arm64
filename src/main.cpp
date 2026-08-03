// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Library Entry Point
//
// This translation unit is loaded when the Android runtime dlopen(3)s
// libgimi_arm64.so.  The constructor attribute fires before any application
// code runs, allowing us to install hooks synchronously during library init
// without requiring JNI or an explicit Java-side call.
//
// Non-destructive guarantee: nothing in this file modifies APK assets,
// on-disk ELF binaries, or any file belonging to the game.
// ─────────────────────────────────────────────────────────────────────────────

#include "utils/logger.h"

// ─── Build Metadata ───────────────────────────────────────────────────────────
#define GIMI_BUILD_ARCH  "arm64-v8a"
#define GIMI_BUILD_DATE  __DATE__

namespace gimi {

// ─── Forward Declarations ─────────────────────────────────────────────────────
// These will be defined in Phase 1 Plan 01-02 (Vulkan/EGL dispatcher).
// Declared here so main.cpp remains the single orchestration point.
void graphics_dispatcher_init() noexcept;
void graphics_dispatcher_shutdown() noexcept;

// ─── Library Constructor / Destructor ─────────────────────────────────────────
__attribute__((constructor))
static void gimi_on_load() noexcept {
    LOGI("╔══════════════════════════════════════════════════╗");
    LOGI("║        gimi_arm64  v%-28s║", GIMI_VERSION);
    LOGI("║  Vulkan/GLES Graphics Hook — %s     ║", GIMI_BUILD_ARCH);
    LOGI("║  Build: %-40s║", GIMI_BUILD_DATE);
    LOGI("╚══════════════════════════════════════════════════╝");
    LOGI("Initializing graphics dispatcher…");

    graphics_dispatcher_init();

    LOGI("gimi_arm64 ready. Non-destructive interception active.");
}

__attribute__((destructor))
static void gimi_on_unload() noexcept {
    LOGI("gimi_arm64: shutting down graphics dispatcher…");
    graphics_dispatcher_shutdown();
    LOGI("gimi_arm64: unloaded cleanly.");
}

} // namespace gimi
