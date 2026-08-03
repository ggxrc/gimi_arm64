#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Mesh Swapper Engine
//
// Intercepts draw calls and replaces Vertex/Index Buffer handles when the
// active geometry hash matches a TextureOverride rule in the ModDatabase.
//
// Contract:
//   - Called from gimi_vkCmdDrawIndexed in the Vulkan Layer hot path.
//   - All replacement buffers must be pre-allocated in VRAM at init time.
//   - Zero heap allocations in the draw-call path.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>
#include <unordered_map>

namespace gimi {

// ─── MeshOverrideResult ──────────────────────────────────────────────────────
// Returned by MeshSwapper::try_swap when a match is found.
struct MeshOverrideResult {
    bool        should_override   = false;
    VkBuffer    override_vb       = VK_NULL_HANDLE;  // replacement vertex buffer
    VkBuffer    override_ib       = VK_NULL_HANDLE;  // replacement index buffer (or VK_NULL_HANDLE)
    uint32_t    index_count       = 0;                // new indexCount for vkCmdDrawIndexed
    uint32_t    vertex_offset     = 0;                // new vertexOffset
    uint32_t    first_index       = 0;                // new firstIndex
};

// ─── MeshSwapper ──────────────────────────────────────────────────────────────
class MeshSwapper {
public:
    static MeshSwapper& instance() noexcept;

    // Check if the currently-bound vertex buffer hash matches a mod override.
    // If yes, populates result with the replacement buffer handles and counts.
    // Must be called from the vkCmdDrawIndexed intercept.
    MeshOverrideResult try_swap(uint32_t active_vb_hash) noexcept;

    // Register a mod replacement buffer in VRAM (called at mod load time).
    // hash: the 3dmigoto hash identifying the original geometry.
    // vb/ib: pre-allocated Vulkan buffers containing the modded geometry.
    void register_override(uint32_t hash,
                           VkBuffer vb, VkBuffer ib,
                           uint32_t index_count,
                           uint32_t vertex_offset,
                           uint32_t first_index) noexcept;

private:
    struct OverrideEntry {
        VkBuffer vb           = VK_NULL_HANDLE;
        VkBuffer ib           = VK_NULL_HANDLE;
        uint32_t index_count  = 0;
        uint32_t vertex_offset = 0;
        uint32_t first_index  = 0;
    };

    // Indexed by hash for O(1) lookup during draw calls.
    // Populated at mod load time, read-only during render.
    std::unordered_map<uint32_t, OverrideEntry> m_overrides;
};

} // namespace gimi
