#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — 3dmigoto .ini Parser
//
// Loads one or more 3dmigoto-format .ini files into a ModDatabase.
// Uses inih (benhoyt/inih) as the low-level parsing engine.
//
// Supported sections:
//   [TextureOverride.*]  →  TextureOverrideRule
//   [ShaderOverride.*]   →  ShaderOverrideRule
//   [Resource.*]         →  ResourceDeclaration
//   [Constants]          →  ConstantRule
//
// Supports the $include directive for auxiliary files (Txfix.ini, etc.).
//
// Usage:
//   gimi::IniParser parser;
//   gimi::ModDatabase db;
//   parser.load("/sdcard/gimi_mods/Orfix.ini", db);
// ─────────────────────────────────────────────────────────────────────────────

#include "config/mod_types.h"
#include <string>

namespace gimi {

class IniParser {
public:
    // Load a single .ini file into db (merges with any existing entries).
    // Returns true on success; false on file-open error.
    // Unknown sections/keys are silently ignored (logged at LOGW level).
    bool load(const std::string& filepath, ModDatabase& db) noexcept;

    // Maximum recursive $include depth to prevent infinite loops.
    static constexpr int kMaxIncludeDepth = 8;

private:
    // Internal: load with tracked include depth.
    bool load_internal(const std::string& filepath, ModDatabase& db,
                       int depth) noexcept;

    // inih handler — called once per key=value line.
    static int ini_handler(void* user, const char* section,
                           const char* name, const char* value) noexcept;

    struct ParseContext {
        IniParser*   self     = nullptr;
        ModDatabase* db       = nullptr;
        std::string  base_dir;   // directory of the .ini file being parsed
        int          depth    = 0;
    };
};

} // namespace gimi
