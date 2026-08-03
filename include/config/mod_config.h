#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Mod Configuration Singleton
//
// Global access point to the loaded ModDatabase.
// Initialized during graphics_dispatcher_init() before any render frames.
//
// Usage:
//   const gimi::ModDatabase& db = gimi::ModConfig::instance().db();
//   if (db.has_texture_override(hash)) { ... }
// ─────────────────────────────────────────────────────────────────────────────

#include "config/mod_types.h"
#include <string>

namespace gimi {

// Default mods directory on Android external storage.
// Override at compile time: -DGIMI_MODS_DIR='"/data/local/tmp/gimi_mods"'
#ifndef GIMI_MODS_DIR
#define GIMI_MODS_DIR "/sdcard/gimi_mods"
#endif

class ModConfig {
public:
    static ModConfig& instance() noexcept;

    // Load all .ini files found in mods_dir (non-recursive, *.ini at root).
    // Safe to call multiple times — clears and reloads each call.
    void load(const std::string& mods_dir = GIMI_MODS_DIR) noexcept;

    const ModDatabase& db() const noexcept { return m_db; }
    ModDatabase&       db()       noexcept { return m_db; }

    bool is_loaded() const noexcept { return m_loaded; }

private:
    ModDatabase m_db;
    bool        m_loaded = false;
};

} // namespace gimi
