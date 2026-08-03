// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — 3dmigoto .ini Parser Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "config/ini_parser.h"
#include "utils/logger.h"

// inih C library (third_party/inih)
extern "C" {
#include "ini.h"
}

#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <string_view>

namespace gimi {

namespace fs = std::filesystem;

// ─── Utility: normalize key to lowercase, trim whitespace ─────────────────────
static std::string normalize_key(const char* s) noexcept {
    std::string out = s ? s : "";
    // Trim leading
    size_t start = out.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return {};
    out = out.substr(start);
    // Trim trailing
    size_t end = out.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) out = out.substr(0, end + 1);
    // Lowercase
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return out;
}

// ─── Utility: parse 8-digit hex hash string → uint32_t ───────────────────────
static bool parse_hash(const std::string& s, uint32_t& out) noexcept {
    if (s.size() != 8) return false;
    try {
        out = static_cast<uint32_t>(std::stoul(s, nullptr, 16));
        return true;
    } catch (...) {
        return false;
    }
}

// ─── Utility: section prefix match ────────────────────────────────────────────
static bool starts_with(std::string_view str, std::string_view prefix) noexcept {
    return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
}

// ─── IniParser::ini_handler ────────────────────────────────────────────────────
// Called by inih for every key=value pair (and section changes via empty name).
int IniParser::ini_handler(void* user, const char* section,
                            const char* name_raw, const char* value_raw) noexcept {
    auto* ctx = static_cast<ParseContext*>(user);
    ModDatabase& db = *ctx->db;

    if (!section || !name_raw || !value_raw) return 1; // skip null callbacks

    std::string sec(section);
    std::string key  = normalize_key(name_raw);
    std::string val  = value_raw ? std::string(value_raw) : "";

    // Trim leading/trailing whitespace from value
    {
        size_t s = val.find_first_not_of(" \t\r\n");
        size_t e = val.find_last_not_of(" \t\r\n");
        val = (s == std::string::npos) ? "" : val.substr(s, e - s + 1);
    }

    // ── $include directive ────────────────────────────────────────────────────
    if (key == "$include" || (sec.empty() && key == "include")) {
        if (ctx->depth >= IniParser::kMaxIncludeDepth) {
            LOGW("IniParser: $include depth limit reached (%d), skipping '%s'",
                 IniParser::kMaxIncludeDepth, val.c_str());
            return 1;
        }
        std::string include_path = ctx->base_dir + "/" + val;
        LOGD("IniParser: $include → %s", include_path.c_str());
        ctx->self->load_internal(include_path, *ctx->db, ctx->depth + 1);
        return 1;
    }

    // ── [TextureOverride.*] ────────────────────────────────────────────────────
    if (starts_with(sec, "TextureOverride.") || starts_with(sec, "textureoverride.")) {
        std::string rule_name = sec.substr(sec.find('.') + 1);
        auto& rule = db.texture_overrides[0]; // placeholder — insert by hash after

        // We need the hash first; build a temporary rule keyed by section name.
        // Use a staging map: section_name → partial rule, then finalize on hash= line.
        // Simplified: build the rule and re-key when hash is found.
        auto it = std::find_if(db.texture_overrides.begin(), db.texture_overrides.end(),
            [&](const auto& p){ return p.second.section_name == rule_name; });

        TextureOverrideRule* r = nullptr;
        if (it != db.texture_overrides.end()) {
            r = &it->second;
        } else {
            // Stage with hash=0 until hash= line is parsed
            TextureOverrideRule new_rule;
            new_rule.section_name = rule_name;
            db.texture_overrides[0xFFFFFFFF ^ static_cast<uint32_t>(
                std::hash<std::string>{}(rule_name))] = std::move(new_rule);
            auto it2 = std::find_if(db.texture_overrides.begin(), db.texture_overrides.end(),
                [&](const auto& p){ return p.second.section_name == rule_name; });
            r = &it2->second;
        }

        if (key == "hash") {
            uint32_t h = 0;
            if (parse_hash(val, h)) {
                // Re-key: remove staging entry, insert under real hash
                TextureOverrideRule moved = std::move(*r);
                // erase staging entry
                for (auto it3 = db.texture_overrides.begin();
                     it3 != db.texture_overrides.end(); ++it3) {
                    if (&it3->second == r) {
                        db.texture_overrides.erase(it3);
                        break;
                    }
                }
                moved.hash = h;
                db.texture_overrides[h] = std::move(moved);
                LOGD("IniParser: TextureOverride[%s] hash=0x%08X", rule_name.c_str(), h);
            } else {
                LOGW("IniParser: invalid hash '%s' in [%s]", val.c_str(), sec.c_str());
            }
        } else if (starts_with(key, "ps-t")) {
            // ps-t0, ps-t1, … — pixel shader texture slot override
            int slot = std::atoi(key.c_str() + 4);
            r->ps_slots[slot] = val;
        } else if (starts_with(key, "vs-t")) {
            int slot = std::atoi(key.c_str() + 4);
            r->vs_slots[slot] = val;
        } else {
            r->resource_name = val; // simple resource= assignment
        }
        return 1;
    }

    // ── [ShaderOverride.*] ─────────────────────────────────────────────────────
    if (starts_with(sec, "ShaderOverride.") || starts_with(sec, "shaderoverride.")) {
        std::string rule_name = sec.substr(sec.find('.') + 1);

        auto get_or_create = [&]() -> ShaderOverrideRule& {
            auto it = std::find_if(db.shader_overrides.begin(), db.shader_overrides.end(),
                [&](const auto& p){ return p.second.section_name == rule_name; });
            if (it != db.shader_overrides.end()) return it->second;
            ShaderOverrideRule r; r.section_name = rule_name;
            uint32_t staging = 0xFFFFFFFE ^ static_cast<uint32_t>(
                std::hash<std::string>{}(rule_name));
            db.shader_overrides[staging] = std::move(r);
            return db.shader_overrides[staging];
        };

        ShaderOverrideRule& r = get_or_create();
        if (key == "hash") {
            uint32_t h = 0;
            if (parse_hash(val, h)) {
                ShaderOverrideRule moved = std::move(r);
                for (auto it2 = db.shader_overrides.begin();
                     it2 != db.shader_overrides.end(); ++it2) {
                    if (&it2->second == &r) {
                        db.shader_overrides.erase(it2);
                        break;
                    }
                }
                moved.hash = h;
                db.shader_overrides[h] = std::move(moved);
                LOGD("IniParser: ShaderOverride[%s] hash=0x%08X", rule_name.c_str(), h);
            }
        } else if (key == "ps") {
            r.ps_resource = val;
        } else if (key == "vs") {
            r.vs_resource = val;
        } else {
            r.extra[key] = val;
        }
        return 1;
    }

    // ── [Resource.*] ──────────────────────────────────────────────────────────
    if (starts_with(sec, "Resource.") || starts_with(sec, "resource.")) {
        std::string res_name = sec.substr(sec.find('.') + 1);
        auto& decl = db.resources[res_name];
        decl.name = res_name;
        if (key == "filename") {
            decl.filename = val;
            LOGD("IniParser: Resource[%s] filename=%s", res_name.c_str(), val.c_str());
        } else if (key == "type") {
            decl.type = val;
        } else {
            decl.raw[key] = val;
        }
        return 1;
    }

    // ── [Constants] ───────────────────────────────────────────────────────────
    if (sec == "Constants" || sec == "constants") {
        ConstantRule cr;
        // Support "global $name = value" syntax — key contains "global $name"
        std::string cname = key;
        if (starts_with(cname, "global ")) cname = cname.substr(7);
        cr.name  = cname;
        cr.value = val;
        db.constants.push_back(std::move(cr));
        LOGD("IniParser: Constant %s = %s", cname.c_str(), val.c_str());
        return 1;
    }

    // Unknown section — skip silently (forward compatibility)
    LOGD("IniParser: unknown section=[%s] key=%s — skipped", sec.c_str(), key.c_str());
    return 1;
}

// ─── IniParser::load ──────────────────────────────────────────────────────────
bool IniParser::load(const std::string& filepath, ModDatabase& db) noexcept {
    return load_internal(filepath, db, 0);
}

bool IniParser::load_internal(const std::string& filepath,
                               ModDatabase& db, int depth) noexcept {
    ParseContext ctx;
    ctx.self     = this;
    ctx.db       = &db;
    ctx.depth    = depth;

    // Determine base directory for relative $include resolution
    fs::path p(filepath);
    ctx.base_dir = p.parent_path().string();
    if (ctx.base_dir.empty()) ctx.base_dir = ".";

    LOGI("IniParser: loading %s (depth=%d)", filepath.c_str(), depth);

    int ret = ini_parse(filepath.c_str(), ini_handler, &ctx);
    if (ret == -1) {
        LOGE("IniParser: cannot open file '%s'", filepath.c_str());
        return false;
    }
    if (ret > 0) {
        LOGW("IniParser: parse warning at line %d in '%s' (non-fatal)",
             ret, filepath.c_str());
    }

    LOGI("IniParser: loaded %s — %zu texture rules, %zu shader rules, %zu resources",
         filepath.c_str(),
         db.texture_overrides.size(),
         db.shader_overrides.size(),
         db.resources.size());
    return true;
}

} // namespace gimi
