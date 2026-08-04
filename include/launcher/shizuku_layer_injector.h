#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Android Launcher: Shizuku Layer Injector
//
// Manages Vulkan layer injection settings via Shizuku ADB / Android System Settings
// without requiring root access.
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <vector>

namespace gimi {

enum class GameDistribution {
    PlayStore,    // com.miHoYo.GenshinImpact
    GalaxyStore,  // com.miHoYo.GI.samsung
    ChinaServer,  // com.yuanshen.site
    Bilibili      // com.miHoYo.ys.bilibili
};

class ShizukuLayerInjector {
public:
    static ShizukuLayerInjector& instance() noexcept;

    // Check if Shizuku binder service is active
    bool is_shizuku_available() const noexcept;

    // Check if the layer is currently enabled in settings
    bool is_layer_enabled() const noexcept;

    // Enable/Disable Vulkan Layer for target game package
    bool enable_layer(GameDistribution dist, const std::string& layer_so = "libgimi_arm64.so") noexcept;
    bool disable_layer() noexcept;

    // Launch the target game application
    bool launch_game(GameDistribution dist) noexcept;

    // Convert GameDistribution to package name string
    static std::string get_package_name(GameDistribution dist) noexcept;

private:
    bool m_layer_enabled = false;
    GameDistribution m_current_dist = GameDistribution::PlayStore;
};

} // namespace gimi
