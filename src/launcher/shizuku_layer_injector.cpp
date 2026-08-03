// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Android Launcher: Shizuku Layer Injector Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "launcher/shizuku_layer_injector.h"
#include "utils/logger.h"
#include <cstdlib>

namespace gimi {

ShizukuLayerInjector& ShizukuLayerInjector::instance() noexcept {
    static ShizukuLayerInjector s_injector;
    return s_injector;
}

std::string ShizukuLayerInjector::get_package_name(GameDistribution dist) noexcept {
    switch (dist) {
        case GameDistribution::PlayStore:   return "com.miHoYo.GenshinImpact";
        case GameDistribution::GalaxyStore: return "com.miHoYo.GenshinImpact.samsung";
        case GameDistribution::ChinaServer: return "com.yuanshen.site";
        case GameDistribution::Bilibili:    return "com.miHoYo.ys.bilibili";
    }
    return "com.miHoYo.GenshinImpact";
}

bool ShizukuLayerInjector::is_shizuku_available() const noexcept {
    // Check if Shizuku shell service or ADB environment is responsive
    int res = system("shizuku status 2>/dev/null || which adb >/dev/null");
    return (res == 0);
}

bool ShizukuLayerInjector::enable_layer(GameDistribution dist, const std::string& layer_so) noexcept {
    std::string pkg = get_package_name(dist);
    LOGI("ShizukuLayerInjector: Enabling Vulkan layer '%s' for package '%s'", layer_so.c_str(), pkg.c_str());

    std::string cmd1 = "settings put global enable_gpu_debug_layers 1";
    std::string cmd2 = "settings put global gpu_debug_app " + pkg;
    std::string cmd3 = "settings put global gpu_debug_layer_app com.gimi.launcher";
    std::string cmd4 = "settings put global gpu_debug_layers " + layer_so;

    system(cmd1.c_str());
    system(cmd2.c_str());
    system(cmd3.c_str());
    system(cmd4.c_str());

    m_layer_enabled = true;
    m_current_dist = dist;
    return true;
}

bool ShizukuLayerInjector::disable_layer() noexcept {
    LOGI("ShizukuLayerInjector: Disabling Vulkan debug layers");
    system("settings put global enable_gpu_debug_layers 0");
    m_layer_enabled = false;
    return true;
}

bool ShizukuLayerInjector::launch_game(GameDistribution dist) noexcept {
    std::string pkg = get_package_name(dist);
    LOGI("ShizukuLayerInjector: Launching package '%s'", pkg.c_str());

    std::string launch_cmd = "monkey -p " + pkg + " -c android.intent.category.LAUNCHER 1";
    int res = system(launch_cmd.c_str());
    return (res == 0);
}

} // namespace gimi
