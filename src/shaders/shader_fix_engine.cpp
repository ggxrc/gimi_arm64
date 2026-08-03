// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Shader Fix Engine Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "shaders/shader_fix_engine.h"
#include "shaders/spirv_hasher.h"
#include "config/mod_config.h"
#include "utils/logger.h"

#include <unordered_map>

namespace gimi {

ShaderFixEngine& ShaderFixEngine::instance() noexcept {
    static ShaderFixEngine s_engine;
    return s_engine;
}

bool ShaderFixEngine::process_shader_module(
        const VkShaderModuleCreateInfo& create_info,
        std::vector<uint32_t>& patched_code) noexcept {

    if (!create_info.pCode || create_info.codeSize == 0) return false;

    // ── Compute SPIR-V hash ──────────────────────────────────────────────────
    uint32_t hash = compute_spirv_hash(create_info.pCode, create_info.codeSize);

    LOGD("ShaderFixEngine: shader module hash=0x%08X (size=%zu bytes)",
         hash, create_info.codeSize);

    // ── Check pre-registered patches first ───────────────────────────────────
    auto it = m_patches.find(hash);
    if (it != m_patches.end()) {
        patched_code = it->second.code;
        LOGI("ShaderFixEngine: applied pre-compiled shader patch for hash 0x%08X "
             "(original %zu bytes → patched %zu bytes)",
             hash, create_info.codeSize, patched_code.size() * sizeof(uint32_t));
        return true;
    }

    // ── Check ModDatabase for ShaderOverride rules ───────────────────────────
    const auto& db = ModConfig::instance().db();
    if (db.has_shader_override(hash)) {
        const auto& rule = db.shader_overrides.at(hash);
        LOGI("ShaderFixEngine: ShaderOverride rule exists for hash 0x%08X "
             "(section='%s') but no pre-compiled SPIR-V registered yet.",
             hash, rule.section_name.c_str());

        // Future: load SPIR-V from rule's resource file on first encounter.
        // For now, log the match so modders know the hash is recognized.
    }

    return false;
}

void ShaderFixEngine::register_patched_shader(
        uint32_t hash,
        std::vector<uint32_t> spirv_code) noexcept {
    PatchedShader patch;
    patch.code = std::move(spirv_code);
    m_patches[hash] = std::move(patch);

    LOGI("ShaderFixEngine: registered patched shader for hash 0x%08X (%zu words)",
         hash, m_patches[hash].code.size());
}

} // namespace gimi
