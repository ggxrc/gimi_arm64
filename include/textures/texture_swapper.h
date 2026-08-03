#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Texture Swapper
//
// O(1) matching engine that determines if an active texture (identified by hash)
// should be replaced with a modded texture (VkImageView) during
// vkUpdateDescriptorSets.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>
#include <unordered_map>
#include <optional>

namespace gimi {

class TextureSwapper {
public:
    static TextureSwapper& instance() noexcept;

    // Register a modded texture view for a specific hash.
    // Called when a mod is loaded and its texture is uploaded to VRAM.
    void register_override(uint32_t hash, VkImageView mod_view) noexcept;

    // Try to swap a texture. If the hash has a registered override, returns
    // the replacement VkImageView. Otherwise returns std::nullopt.
    std::optional<VkImageView> try_swap(uint32_t active_texture_hash) noexcept;

private:
    std::unordered_map<uint32_t, VkImageView> m_overrides;
};

} // namespace gimi
