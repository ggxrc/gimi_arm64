#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — 3dmigoto Mod Data Types
//
// Represents the in-memory model of a loaded set of 3dmigoto mod rules.
// These types mirror the sections of Orfix.ini / Txfix.ini / override .ini:
//
//   [TextureOverride.*]  →  TextureOverrideRule
//   [ShaderOverride.*]   →  ShaderOverrideRule
//   [Resource.*]         →  ResourceDeclaration
//   [Constants]          →  ConstantRule
//
// All types are plain data structs with no virtual methods so they are
// trivially copyable / movable and safe to use in concurrent read paths.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace gimi {

// ─── TextureOverrideRule ─────────────────────────────────────────────────────
// Represents one [TextureOverride.<name>] section.
// Example:
//   [TextureOverride.LumineFace]
//   hash = a1b2c3d4
//   ps-t0 = ResourceLumineFaceNew
struct TextureOverrideRule {
    uint32_t    hash        = 0;        // 3dmigoto CRC32 hash (8 hex digits)
    std::string section_name;           // e.g. "LumineFace"
    // Pixel-shader texture slot overrides: slot index → resource name
    std::unordered_map<int, std::string> ps_slots;   // ps-t0, ps-t1, …
    std::unordered_map<int, std::string> vs_slots;   // vs-t0, …
    // Optional: direct resource reference for simple single-slot overrides
    std::string resource_name;
};

// ─── ShaderOverrideRule ───────────────────────────────────────────────────────
// Represents one [ShaderOverride.<name>] section.
// Example:
//   [ShaderOverride.ShadowFix]
//   hash = deadbeef
//   ps = ResourcePatchedShader
struct ShaderOverrideRule {
    uint32_t    hash        = 0;
    std::string section_name;
    std::string ps_resource;  // pixel shader replacement resource name
    std::string vs_resource;  // vertex shader replacement resource name
    // Arbitrary extra key=value pairs for future extensibility
    std::unordered_map<std::string, std::string> extra;
};

// ─── ResourceDeclaration ─────────────────────────────────────────────────────
// Represents one [Resource.<name>] section.
// Example:
//   [Resource.ResourceLumineFaceNew]
//   filename = LumineFaceNew.dds
struct ResourceDeclaration {
    std::string name;         // resource identifier (e.g. "ResourceLumineFaceNew")
    std::string filename;     // relative path to the asset file
    std::string type;         // "Buffer" | "Texture2D" | "StructuredBuffer" (optional)
    // Raw key=value map for forward compatibility with 3dmigoto extensions
    std::unordered_map<std::string, std::string> raw;
};

// ─── ConstantRule ─────────────────────────────────────────────────────────────
// Represents an entry in the [Constants] section.
// Example: global $active = 1
struct ConstantRule {
    std::string name;   // variable name including sigil, e.g. "$active"
    std::string value;  // raw string value, e.g. "1"
};

// ─── ModDatabase ─────────────────────────────────────────────────────────────
// Aggregate of all rules loaded from a mod's .ini files.
// Indexed by hash (uint32_t) for O(1) lookup during the render loop.
struct ModDatabase {
    // Primary lookup tables (keyed by resource hash for render-hot paths)
    std::unordered_map<uint32_t, TextureOverrideRule>  texture_overrides;
    std::unordered_map<uint32_t, ShaderOverrideRule>   shader_overrides;

    // Resource declarations (keyed by resource name for cross-reference)
    std::unordered_map<std::string, ResourceDeclaration> resources;

    // Global constants
    std::vector<ConstantRule> constants;

    // ── Accessors ────────────────────────────────────────────────────────────
    bool has_texture_override(uint32_t hash) const noexcept {
        return texture_overrides.count(hash) > 0;
    }
    bool has_shader_override(uint32_t hash) const noexcept {
        return shader_overrides.count(hash) > 0;
    }
    const ResourceDeclaration* find_resource(const std::string& name) const noexcept {
        auto it = resources.find(name);
        return it != resources.end() ? &it->second : nullptr;
    }

    void clear() noexcept {
        texture_overrides.clear();
        shader_overrides.clear();
        resources.clear();
        constants.clear();
    }
};

} // namespace gimi
