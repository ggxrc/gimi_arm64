// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Resource Hash Engine Implementation
//
// Computes xxHash64 digests and supports dump mode for mod creation.
// ─────────────────────────────────────────────────────────────────────────────

#include "hash/resource_hash_engine.h"
#include "hash/hash_registry.h"
#include "utils/logger.h"

#include "xxhash.h"

#include <algorithm>  // std::min
#include <cstring>    // memcpy
#include <cstdio>     // fopen, fwrite
#include <sys/stat.h> // mkdir

namespace gimi {

ResourceHashEngine& ResourceHashEngine::instance() noexcept {
    static ResourceHashEngine s_engine;
    return s_engine;
}

// ─── hash_buffer ──────────────────────────────────────────────────────────────
uint32_t ResourceHashEngine::hash_buffer(
        VkDevice       device,
        VkBuffer       buffer,
        VkDeviceMemory memory,
        VkDeviceSize   offset,
        VkDeviceSize   size) noexcept {

    // ── Cache hit path (lock-free on shared_mutex) ────────────────────────────
    auto cached = HashRegistry::instance().get_buffer(buffer);
    if (cached.has_value()) {
        return cached->hash32;
    }

    // ── Cache miss: map memory and compute hash ───────────────────────────────
    VkDeviceSize sample_size = std::min(size, static_cast<VkDeviceSize>(GIMI_HASH_SAMPLE_BYTES));

    void* data = nullptr;
    VkResult res = vkMapMemory(device, memory, offset, sample_size, 0, &data);
    if (res != VK_SUCCESS || !data) {
        LOGW("ResourceHashEngine: vkMapMemory failed for buffer %p (result=%d) — "
             "buffer may be device-local only. Hash will be 0.", (void*)buffer, res);
        return 0;
    }

    uint64_t h64 = XXH64(data, static_cast<size_t>(sample_size), /*seed=*/0);
    uint32_t h32 = static_cast<uint32_t>(h64 & 0xFFFFFFFF);

    // ── Dump mode: save buffer data to /sdcard/GIMI/Dump/ ────────────────────
    if (m_dump_enabled.load(std::memory_order_relaxed)) {
        dump_buffer(h32, data, static_cast<size_t>(sample_size));
    }

    vkUnmapMemory(device, memory);

    HashEntry entry{h64, h32};
    HashRegistry::instance().store_buffer(buffer, entry);

    LOGR("ResourceHashEngine: buffer %p → hash32=0x%08X (sampled %zu bytes)",
         (void*)buffer, h32, static_cast<size_t>(sample_size));

    return h32;
}

// ─── hash_image ───────────────────────────────────────────────────────────────
// Uses a metadata fingerprint (no VRAM readback — avoids GPU sync stall).
// Fields: width, height, depth, format, mipLevels, arrayLayers.
uint32_t ResourceHashEngine::hash_image(
        [[maybe_unused]] VkDevice device,
        VkImage                   image,
        const VkImageCreateInfo&  ci) noexcept {

    // ── Cache hit ─────────────────────────────────────────────────────────────
    auto cached = HashRegistry::instance().get_image(image);
    if (cached.has_value()) {
        return cached->hash32;
    }

    // ── Fingerprint of creation metadata ─────────────────────────────────────
    struct ImageFingerprint {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        int32_t  format;     // VkFormat (int32)
        uint32_t mip_levels;
        uint32_t array_layers;
        int32_t  image_type; // VkImageType
    } fp{
        ci.extent.width,
        ci.extent.height,
        ci.extent.depth,
        static_cast<int32_t>(ci.format),
        ci.mipLevels,
        ci.arrayLayers,
        static_cast<int32_t>(ci.imageType),
    };

    uint64_t h64 = XXH64(&fp, sizeof(fp), /*seed=*/1);
    uint32_t h32 = static_cast<uint32_t>(h64 & 0xFFFFFFFF);

    HashEntry entry{h64, h32};
    HashRegistry::instance().store_image(image, entry);

    LOGR("ResourceHashEngine: image %p %ux%u fmt=%d → hash32=0x%08X",
         (void*)image, ci.extent.width, ci.extent.height,
         static_cast<int>(ci.format), h32);

    return h32;
}

// ─── dump_buffer ──────────────────────────────────────────────────────────────
void ResourceHashEngine::dump_buffer(uint32_t hash, const void* data, size_t size) noexcept {
    // Create dump directory if needed
    mkdir(m_dump_path.c_str(), 0755);

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/0x%08X.buf", m_dump_path.c_str(), hash);

    // Check if file already exists (avoid re-dumping)
    struct stat st;
    if (stat(filename, &st) == 0) {
        return;  // Already dumped
    }

    FILE* f = fopen(filename, "wb");
    if (f) {
        fwrite(data, 1, size, f);
        fclose(f);
        LOGR("DUMP: Saved buffer hash=0x%08X (%zu bytes) → %s", hash, size, filename);
    } else {
        LOGE("DUMP: Failed to write %s", filename);
    }
}

// ─── invalidate_caches ───────────────────────────────────────────────────────
void ResourceHashEngine::invalidate_caches() noexcept {
    HashRegistry::instance().clear();
    LOGR("ResourceHashEngine: caches invalidated (hot-reload)");
}

} // namespace gimi
