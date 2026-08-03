#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Shader Fix Engine (Orfix & Txfix)
//
// Intercepts shader module creation and applies Orfix.ini (illumination/shadow
// fixes) and Txfix.ini (transparency/UV fixes) by substituting the SPIR-V
// bytecode when the shader hash matches a rule in the ModDatabase.
//
// Usage: called from gimi_vkCreateShaderModule in vulkan_layer.cpp.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace gimi {

class ShaderFixEngine {
public:
    static ShaderFixEngine& instance() noexcept;

    // Process a shader module creation request.
    // If the shader hash matches a ShaderOverride rule in the ModDatabase:
    //   - Populates patched_code with the replacement SPIR-V bytecode.
    //   - Returns true (caller must use patched_code as the new pCode).
    // If no match:
    //   - Returns false (caller uses the original bytecode unchanged).
    bool process_shader_module(
        const VkShaderModuleCreateInfo& create_info,
        std::vector<uint32_t>& patched_code) noexcept;

    // Register a pre-compiled patched shader for a given hash.
    // Called during mod loading to prepare replacement SPIR-V bytecodes.
    void register_patched_shader(
        uint32_t hash,
        std::vector<uint32_t> spirv_code) noexcept;

private:
    struct PatchedShader {
        std::vector<uint32_t> code;
    };

    // Hash → pre-compiled replacement SPIR-V
    std::unordered_map<uint32_t, PatchedShader> m_patches;
};

} // namespace gimi
