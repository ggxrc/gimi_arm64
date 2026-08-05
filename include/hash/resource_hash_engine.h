#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Resource Hash Engine
//
// Computes xxHash64 digests for Vulkan buffers and images on first access,
// caches results in HashRegistry, and returns the 3dmigoto-compatible
// truncated uint32_t hash for ModDatabase lookups.
//
// Dump mode: When enabled, saves original buffer/image data to
// /sdcard/GIMI/Dump/ for mod creation and hash identification.
//
// Anti-stutter contract:
//   - Buffer hashing samples at most GIMI_HASH_SAMPLE_BYTES bytes.
//   - Image hashing uses metadata fingerprinting only (no VRAM readback).
//   - Cache hits return immediately with zero compute overhead.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>
#include <atomic>
#include <string>

namespace gimi {

// Maximum bytes sampled per buffer hash computation.
// Override at compile time: -DGIMI_HASH_SAMPLE_BYTES=<N>
#ifndef GIMI_HASH_SAMPLE_BYTES
#define GIMI_HASH_SAMPLE_BYTES 4096
#endif

class ResourceHashEngine {
public:
    static ResourceHashEngine& instance() noexcept;

    // Hash a VkBuffer by sampling its mapped memory.
    // Returns 0 if the buffer cannot be mapped (e.g. device-local without staging).
    // Result is 3dmigoto-compatible: truncated uint32_t (lower 32 bits of XXH64).
    uint32_t hash_buffer(VkDevice         device,
                         VkBuffer         buffer,
                         VkDeviceMemory   memory,
                         VkDeviceSize     offset,
                         VkDeviceSize     size) noexcept;

    // Hash a VkImage using its creation metadata (no VRAM readback).
    // Deterministic for identical VkImageCreateInfo fields.
    uint32_t hash_image(VkDevice                 device,
                        VkImage                  image,
                        const VkImageCreateInfo& create_info) noexcept;

    // ─── Dump Mode Controls ──────────────────────────────────────────────────
    void set_dump_enabled(bool enabled) noexcept { m_dump_enabled.store(enabled, std::memory_order_relaxed); }
    bool is_dump_enabled() const noexcept { return m_dump_enabled.load(std::memory_order_relaxed); }

    void set_dump_path(const std::string& path) noexcept { m_dump_path = path; }
    const std::string& get_dump_path() const noexcept { return m_dump_path; }

    // Reload mod configs (clear cache and re-scan mods directory)
    void invalidate_caches() noexcept;

private:
    std::atomic<bool> m_dump_enabled{false};
    std::string m_dump_path = "/sdcard/GIMI/Dump";

    // Dump buffer data to file
    void dump_buffer(uint32_t hash, const void* data, size_t size) noexcept;
};

} // namespace gimi
