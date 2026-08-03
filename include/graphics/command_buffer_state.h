#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Command Buffer State Tracker
//
// Tracks which VkBuffers are currently bound as Vertex/Index buffers on each
// VkCommandBuffer. This per-command-buffer state is queried at draw-call time
// to determine whether the active geometry matches a mod hash.
//
// Thread safety: VkCommandBuffer is externally synchronized per the Vulkan
// spec (only one thread records to a given command buffer at a time), so we
// do NOT need internal locking on per-CB state. The global map uses a mutex
// for insert/remove safety across different command buffers.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace gimi {

// ─── BoundBuffer ──────────────────────────────────────────────────────────────
struct BoundBuffer {
    VkBuffer     buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    uint32_t     hash   = 0;  // Filled lazily on first draw call
};

// ─── Per-CommandBuffer State ──────────────────────────────────────────────────
struct CommandBufferState {
    // Currently bound vertex buffers (one per binding slot)
    std::vector<BoundBuffer> vertex_buffers;

    // Currently bound index buffer
    BoundBuffer index_buffer;
    VkIndexType index_type = VK_INDEX_TYPE_UINT16;

    void reset() noexcept {
        vertex_buffers.clear();
        index_buffer = {};
        index_type = VK_INDEX_TYPE_UINT16;
    }
};

// ─── CommandBufferStateTracker ────────────────────────────────────────────────
// Global tracker that maps VkCommandBuffer → CommandBufferState.
class CommandBufferStateTracker {
public:
    static CommandBufferStateTracker& instance() noexcept;

    // Called from gimi_vkCmdBindVertexBuffers
    void bind_vertex_buffers(VkCommandBuffer cb,
                             uint32_t first_binding,
                             uint32_t binding_count,
                             const VkBuffer* buffers,
                             const VkDeviceSize* offsets) noexcept;

    // Called from gimi_vkCmdBindIndexBuffer
    void bind_index_buffer(VkCommandBuffer cb,
                           VkBuffer buffer,
                           VkDeviceSize offset,
                           VkIndexType index_type) noexcept;

    // Get state for a command buffer (returns nullptr if never recorded)
    CommandBufferState* get(VkCommandBuffer cb) noexcept;

    // Remove state (called when command buffer is freed/reset)
    void remove(VkCommandBuffer cb) noexcept;

private:
    std::mutex m_mtx;
    std::unordered_map<VkCommandBuffer, CommandBufferState> m_states;
};

} // namespace gimi
