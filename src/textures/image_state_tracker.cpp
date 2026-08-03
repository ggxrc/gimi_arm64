// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Image State Tracker Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "textures/image_state_tracker.h"
#include "utils/logger.h"

namespace gimi {

ImageStateTracker& ImageStateTracker::instance() noexcept {
    static ImageStateTracker s_tracker;
    return s_tracker;
}

void ImageStateTracker::register_image_hash(VkImage image, uint32_t hash) noexcept {
    if (image == VK_NULL_HANDLE) return;
    std::lock_guard lock(m_mtx);
    m_image_hashes[image] = hash;
    LOGD("ImageStateTracker: Registered image %p with hash 0x%08X", (void*)image, hash);
}

void ImageStateTracker::register_image_view(VkImageView view, VkImage image) noexcept {
    if (view == VK_NULL_HANDLE || image == VK_NULL_HANDLE) return;
    std::lock_guard lock(m_mtx);
    m_view_to_image[view] = image;
}

void ImageStateTracker::remove_image(VkImage image) noexcept {
    if (image == VK_NULL_HANDLE) return;
    std::lock_guard lock(m_mtx);
    m_image_hashes.erase(image);
}

void ImageStateTracker::remove_image_view(VkImageView view) noexcept {
    if (view == VK_NULL_HANDLE) return;
    std::lock_guard lock(m_mtx);
    m_view_to_image.erase(view);
}

std::optional<uint32_t> ImageStateTracker::get_hash_for_view(VkImageView view) noexcept {
    if (view == VK_NULL_HANDLE) return std::nullopt;
    std::lock_guard lock(m_mtx);
    
    auto it_img = m_view_to_image.find(view);
    if (it_img != m_view_to_image.end()) {
        auto it_hash = m_image_hashes.find(it_img->second);
        if (it_hash != m_image_hashes.end()) {
            return it_hash->second;
        }
    }
    return std::nullopt;
}

} // namespace gimi
