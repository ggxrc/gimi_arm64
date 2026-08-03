// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Mod Configuration Singleton Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "config/mod_config.h"
#include "config/ini_parser.h"
#include "utils/logger.h"

#include <filesystem>
#include <algorithm>

namespace gimi {
namespace fs = std::filesystem;

ModConfig& ModConfig::instance() noexcept {
    static ModConfig s_instance;
    return s_instance;
}

void ModConfig::load(const std::string& mods_dir) noexcept {
    m_db.clear();
    m_loaded = false;

    LOGI("ModConfig: loading mods from '%s'", mods_dir.c_str());

    // Check directory exists
    std::error_code ec;
    if (!fs::is_directory(mods_dir, ec)) {
        LOGW("ModConfig: mods directory '%s' not found or not accessible (%s). "
             "No mods will be active.", mods_dir.c_str(), ec.message().c_str());
        return;
    }

    // Collect .ini files at the root of mods_dir (non-recursive)
    std::vector<std::string> ini_files;
    for (const auto& entry : fs::directory_iterator(mods_dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".ini") {
            ini_files.push_back(entry.path().string());
        }
    }

    if (ini_files.empty()) {
        LOGW("ModConfig: no .ini files found in '%s'.", mods_dir.c_str());
        return;
    }

    // Sort so loading order is deterministic (alphabetical)
    std::sort(ini_files.begin(), ini_files.end());

    IniParser parser;
    int loaded_count = 0;
    for (const auto& path : ini_files) {
        if (parser.load(path, m_db)) {
            ++loaded_count;
        }
    }

    m_loaded = (loaded_count > 0);
    LOGI("ModConfig: loaded %d/%zu .ini file(s) — "
         "%zu texture rules, %zu shader rules, %zu resources, %zu constants",
         loaded_count, ini_files.size(),
         m_db.texture_overrides.size(),
         m_db.shader_overrides.size(),
         m_db.resources.size(),
         m_db.constants.size());
}

} // namespace gimi
