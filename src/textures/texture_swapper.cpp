// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Texture Swapper Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "textures/texture_swapper.h"
#include "config/mod_config.h"
#include "textures/texture_loader.h"
#include "utils/logger.h"

namespace gimi {

TextureSwapper& TextureSwapper::instance() noexcept {
    static TextureSwapper s_swapper;
    return s_swapper;
}

void TextureSwapper::register_override(uint32_t hash, VkImageView mod_view) noexcept {
    if (mod_view != VK_NULL_HANDLE) {
        m_overrides[hash] = mod_view;
        LOGI("TextureSwapper: Registered override for hash 0x%08X (View: %p)", hash, (void*)mod_view);
    }
}

std::optional<VkImageView> TextureSwapper::try_swap(uint32_t active_texture_hash) noexcept {
    // Check if we have a VRAM-resident replacement
    auto it = m_overrides.find(active_texture_hash);
    if (it != m_overrides.end()) {
        return it->second;
    }

    // Check the database to see if a rule exists but hasn't been loaded yet
    const auto& db = ModConfig::instance().db();
    if (db.has_texture_override(active_texture_hash)) {
        LOGI("TextureSwapper: Rule exists for 0x%08X, loading texture (MVP mock)", active_texture_hash);
        // Attempt JIT load (mock for now)
        VkImageView new_view = TextureLoader::instance().load_texture(active_texture_hash);
        if (new_view != VK_NULL_HANDLE) {
            register_override(active_texture_hash, new_view);
            return new_view;
        }
    }

    return std::nullopt;
}

} // namespace gimi
