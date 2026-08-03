#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Android Launcher: Mod Manager Service
//
// Scans /sdcard/GIMI/Mods/ directory, parses .ini files, manages active/disabled
// states for each mod directory, and builds active mod list configurations.
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <vector>
#include <filesystem>

namespace gimi {

struct ModItem {
    std::string name;
    std::string path;
    bool enabled;
    std::string author;
    size_t resource_count;
};

class ModManagerService {
public:
    static ModManagerService& instance() noexcept;

    // Scan /sdcard/GIMI/Mods/ and return discovered mods
    std::vector<ModItem> scan_mods(const std::string& mods_root = "/sdcard/GIMI/Mods") noexcept;

    // Toggle mod state (enabled / disabled)
    bool set_mod_enabled(const std::string& mod_path, bool enable) noexcept;

    // Generate active mod configuration for the injector
    std::string generate_active_config() const noexcept;

private:
    std::vector<ModItem> m_mods;
};

} // namespace gimi
