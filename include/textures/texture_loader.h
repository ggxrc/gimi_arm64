#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Texture Loader
//
// Mock MVP for loading mobile texture formats (ASTC, ETC2, RGBA8) from mod
// files into VRAM. Real implementation would queue Vulkan copy commands via
// a staging buffer.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>

namespace gimi {

class TextureLoader {
public:
    static TextureLoader& instance() noexcept;

    // Load a texture file (.ktx, .astc, etc) associated with a 3dmigoto hash.
    // Returns a VkImageView pointing to the newly allocated VRAM, or
    // VK_NULL_HANDLE if loading fails.
    VkImageView load_texture(uint32_t hash) noexcept;

private:
    // In a full implementation, this class would manage a VkCommandPool and
    // staging VkBuffers to upload file data to VkImages.
};

} // namespace gimi
