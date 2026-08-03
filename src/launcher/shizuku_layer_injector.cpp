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
        case GameDistribution::GalaxyStore: return "com.miHoYo.GI.samsung";
        case GameDistribution::ChinaServer: return "com.yuanshen.site";
        case GameDistribution::Bilibili:    return "com.miHoYo.ys.bilibili";
    }
    return "com.miHoYo.GenshinImpact";
}

bool ShizukuLayerInjector::is_shizuku_available() const noexcept {
    // Check if system settings command or ADB / root / shizuku is responsive
    if (system("settings get global enable_gpu_debug_layers >/dev/null 2>&1") == 0) return true;
    if (system("which su >/dev/null 2>&1") == 0) return true;
    if (system("which shizuku >/dev/null 2>&1") == 0) return true;
    if (system("which rish >/dev/null 2>&1") == 0) return true;
    if (system("which adb >/dev/null 2>&1") == 0) return true;
    return false;
}

bool ShizukuLayerInjector::enable_layer(GameDistribution dist, const std::string& layer_so) noexcept {
    std::string pkg = get_package_name(dist);
    LOGI("ShizukuLayerInjector: Enabling Vulkan layer '%s' for package '%s'", layer_so.c_str(), pkg.c_str());

    std::string cmd1 = "settings put global enable_gpu_debug_layers 1";
    std::string cmd2 = "settings put global gpu_debug_app " + pkg;
    std::string cmd3 = "settings put global gpu_debug_layer_app com.gimi.launcher";
    std::string cmd4 = "settings put global gpu_debug_layers " + layer_so;

    int r1 = system(cmd1.c_str());
    int r2 = system(cmd2.c_str());
    int r3 = system(cmd3.c_str());
    int r4 = system(cmd4.c_str());

    if (r1 != 0 || r2 != 0) {
        std::string su_cmd = "su -c \"" + cmd1 + " && " + cmd2 + " && " + cmd3 + " && " + cmd4 + "\" 2>/dev/null";
        system(su_cmd.c_str());
    }

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
