// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Android Launcher: Shizuku Layer Injector Implementation
//
// DEPRECATED: All system() based calls have been replaced with no-op stubs.
// Real injection and status detection logic is now in Java (MainActivity.java)
// using the Settings.Global API, which works correctly within the Android
// app sandbox without requiring shell access or root.
// ─────────────────────────────────────────────────────────────────────────────

#include "launcher/shizuku_layer_injector.h"
#include "utils/logger.h"

namespace gimi {

ShizukuLayerInjector& ShizukuLayerInjector::instance() noexcept {
    static ShizukuLayerInjector s_injector;
    return s_injector;
}

std::string ShizukuLayerInjector::get_package_name(GameDistribution dist) noexcept {
    switch (dist) {
        case GameDistribution::PlayStore:   return "com.miHoYo.GenshinImpact";
        case GameDistribution::GalaxyStore: return "com.miHoYo.GI.samsung";
        case GameDistribution::ChinaServer: return "com.yuanshen.site";
        case GameDistribution::Bilibili:    return "com.miHoYo.ys.bilibili";
    }
    return "com.miHoYo.GenshinImpact";
}

// DEPRECATED STUB: Real check is now done in Java via Settings.Global.getString()
// The old system("settings get global ...") calls are blocked by Android's app sandbox.
bool ShizukuLayerInjector::is_shizuku_available() const noexcept {
    LOGI("ShizukuLayerInjector::is_shizuku_available() — DEPRECATED STUB, returning true. Use Java Settings.Global API instead.");
    return true;
}

// DEPRECATED STUB: Real check is now done in Java via Settings.Global.getString("gpu_debug_layers")
// The old system("settings get global gpu_debug_layers | grep ...") is blocked by Android's app sandbox.
bool ShizukuLayerInjector::is_layer_enabled() const noexcept {
    LOGI("ShizukuLayerInjector::is_layer_enabled() — DEPRECATED STUB, returning true. Use Java Settings.Global API instead.");
    return true;
}

// DEPRECATED STUB: Real injection is now done in Java via Settings.Global.putInt/putString()
// The old system("settings put global ...") calls are blocked by Android's app sandbox.
bool ShizukuLayerInjector::enable_layer(GameDistribution dist, const std::string& layer_so) noexcept {
    std::string pkg = get_package_name(dist);
    LOGI("ShizukuLayerInjector::enable_layer() — DEPRECATED STUB for package '%s', layer '%s'. Use Java Settings.Global API instead.",
         pkg.c_str(), layer_so.c_str());
    m_layer_enabled = true;
    m_current_dist = dist;
    return true;
}

// DEPRECATED STUB: Real disabling is now done in Java via Settings.Global.putInt()
bool ShizukuLayerInjector::disable_layer() noexcept {
    LOGI("ShizukuLayerInjector::disable_layer() — DEPRECATED STUB. Use Java Settings.Global API instead.");
    m_layer_enabled = false;
    return true;
}

// DEPRECATED STUB: Game launching is now done in Java via PackageManager.getLaunchIntentForPackage()
// The old system("monkey -p ...") call is blocked by Android's app sandbox.
bool ShizukuLayerInjector::launch_game(GameDistribution dist) noexcept {
    std::string pkg = get_package_name(dist);
    LOGI("ShizukuLayerInjector::launch_game() — DEPRECATED STUB for package '%s'. Use Java PackageManager instead.", pkg.c_str());
    return true;
}

} // namespace gimi
