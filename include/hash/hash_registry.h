#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Hash Registry
//
// Thread-safe cache mapping Vulkan resource handles (VkBuffer / VkImage)
// to their pre-computed xxHash64 digests.
//
// Design goals:
//   - O(1) lookup per draw call (cache hit path)
//   - std::shared_mutex: many concurrent readers, rare exclusive writes
//   - Event-driven eviction: entries removed on vkDestroyBuffer/Image
//
// Thread safety: all public methods are safe to call from any thread.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

namespace gimi {

// ─── HashEntry ────────────────────────────────────────────────────────────────
struct HashEntry {
    uint64_t hash64 = 0;  // Full xxHash64 digest
    uint32_t hash32 = 0;  // Truncated to 32 bits — 3dmigoto-compatible format
};

// ─── HashRegistry ─────────────────────────────────────────────────────────────
class HashRegistry {
public:
    static HashRegistry& instance() noexcept;

    // ── Buffer cache ──────────────────────────────────────────────────────────
    std::optional<HashEntry> get_buffer(VkBuffer buf)  const noexcept;
    void store_buffer(VkBuffer buf, HashEntry entry)   noexcept;
    void evict_buffer(VkBuffer buf)                    noexcept; // call on vkDestroyBuffer

    // ── Image cache ───────────────────────────────────────────────────────────
    std::optional<HashEntry> get_image(VkImage img)   const noexcept;
    void store_image(VkImage img, HashEntry entry)    noexcept;
    void evict_image(VkImage img)                     noexcept; // call on vkDestroyImage

    // ── Diagnostics ───────────────────────────────────────────────────────────
    size_t buffer_cache_size() const noexcept;
    size_t image_cache_size()  const noexcept;

private:
    mutable std::shared_mutex m_buf_mtx;
    mutable std::shared_mutex m_img_mtx;

    std::unordered_map<VkBuffer, HashEntry> m_buffer_hashes;
    std::unordered_map<VkImage,  HashEntry> m_image_hashes;
};

} // namespace gimi
