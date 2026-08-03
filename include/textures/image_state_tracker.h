#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Image State Tracker
//
// Thread-safe registry linking Vulkan Image and ImageView handles to their
// calculated 3dmigoto hash. Used to identify textures during descriptor set
// updates so they can be swapped.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <optional>

namespace gimi {

class ImageStateTracker {
public:
    static ImageStateTracker& instance() noexcept;

    // Link a VkImage to its calculated content hash (from vkCmdCopyBufferToImage)
    void register_image_hash(VkImage image, uint32_t hash) noexcept;

    // Link a VkImageView to a VkImage (from vkCreateImageView)
    void register_image_view(VkImageView view, VkImage image) noexcept;

    // Remove tracking when destroyed
    void remove_image(VkImage image) noexcept;
    void remove_image_view(VkImageView view) noexcept;

    // Get the hash associated with a VkImageView (used during vkUpdateDescriptorSets)
    std::optional<uint32_t> get_hash_for_view(VkImageView view) noexcept;

private:
    std::mutex m_mtx;
    std::unordered_map<VkImage, uint32_t> m_image_hashes;
    std::unordered_map<VkImageView, VkImage> m_view_to_image;
};

} // namespace gimi
