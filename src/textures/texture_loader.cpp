// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Texture Loader Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "textures/texture_loader.h"
#include "utils/logger.h"

namespace gimi {

TextureLoader& TextureLoader::instance() noexcept {
    static TextureLoader s_loader;
    return s_loader;
}

VkImageView TextureLoader::load_texture(uint32_t hash) noexcept {
    // MVP: In a real implementation this would:
    // 1. Check ModDatabase for the file path associated with `hash`.
    // 2. Read the file (ASTC/ETC2).
    // 3. vkCreateImage, allocate memory, vkBindImageMemory.
    // 4. Staging buffer upload via vkCmdCopyBufferToImage.
    // 5. vkCreateImageView.
    
    // For now, we just log that we would load it.
    LOGI("TextureLoader: Mock load for texture hash 0x%08X", hash);
    return VK_NULL_HANDLE; // Return null until full VRAM management is added
}

} // namespace gimi
