// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Hash Registry Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "hash/hash_registry.h"
#include "utils/logger.h"

namespace gimi {

HashRegistry& HashRegistry::instance() noexcept {
    static HashRegistry s_registry;
    return s_registry;
}

// ─── Buffer Cache ─────────────────────────────────────────────────────────────
std::optional<HashEntry> HashRegistry::get_buffer(VkBuffer buf) const noexcept {
    std::shared_lock lock(m_buf_mtx);
    auto it = m_buffer_hashes.find(buf);
    if (it != m_buffer_hashes.end()) return it->second;
    return std::nullopt;
}

void HashRegistry::store_buffer(VkBuffer buf, HashEntry entry) noexcept {
    std::unique_lock lock(m_buf_mtx);
    m_buffer_hashes[buf] = entry;
}

void HashRegistry::evict_buffer(VkBuffer buf) noexcept {
    std::unique_lock lock(m_buf_mtx);
    m_buffer_hashes.erase(buf);
    LOGD("HashRegistry: evicted buffer %p", (void*)buf);
}

// ─── Image Cache ──────────────────────────────────────────────────────────────
std::optional<HashEntry> HashRegistry::get_image(VkImage img) const noexcept {
    std::shared_lock lock(m_img_mtx);
    auto it = m_image_hashes.find(img);
    if (it != m_image_hashes.end()) return it->second;
    return std::nullopt;
}

void HashRegistry::store_image(VkImage img, HashEntry entry) noexcept {
    std::unique_lock lock(m_img_mtx);
    m_image_hashes[img] = entry;
}

void HashRegistry::evict_image(VkImage img) noexcept {
    std::unique_lock lock(m_img_mtx);
    m_image_hashes.erase(img);
    LOGD("HashRegistry: evicted image %p", (void*)img);
}

// ─── Diagnostics ──────────────────────────────────────────────────────────────
size_t HashRegistry::buffer_cache_size() const noexcept {
    std::shared_lock lock(m_buf_mtx);
    return m_buffer_hashes.size();
}

size_t HashRegistry::image_cache_size() const noexcept {
    std::shared_lock lock(m_img_mtx);
    return m_image_hashes.size();
}

} // namespace gimi
