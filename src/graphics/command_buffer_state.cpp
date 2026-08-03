// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Command Buffer State Tracker Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "graphics/command_buffer_state.h"
#include "utils/logger.h"

namespace gimi {

CommandBufferStateTracker& CommandBufferStateTracker::instance() noexcept {
    static CommandBufferStateTracker s_tracker;
    return s_tracker;
}

void CommandBufferStateTracker::bind_vertex_buffers(
        VkCommandBuffer cb,
        uint32_t first_binding,
        uint32_t binding_count,
        const VkBuffer* buffers,
        const VkDeviceSize* offsets) noexcept {

    std::lock_guard lock(m_mtx);
    auto& state = m_states[cb];

    // Ensure the vector is large enough for all binding slots
    uint32_t required = first_binding + binding_count;
    if (state.vertex_buffers.size() < required) {
        state.vertex_buffers.resize(required);
    }

    for (uint32_t i = 0; i < binding_count; ++i) {
        auto& slot = state.vertex_buffers[first_binding + i];
        slot.buffer = buffers[i];
        slot.offset = offsets ? offsets[i] : 0;
        slot.hash   = 0;  // Reset — will be computed lazily on draw call
    }
}

void CommandBufferStateTracker::bind_index_buffer(
        VkCommandBuffer cb,
        VkBuffer buffer,
        VkDeviceSize offset,
        VkIndexType index_type) noexcept {

    std::lock_guard lock(m_mtx);
    auto& state = m_states[cb];
    state.index_buffer.buffer = buffer;
    state.index_buffer.offset = offset;
    state.index_buffer.hash   = 0;
    state.index_type = index_type;
}

CommandBufferState* CommandBufferStateTracker::get(VkCommandBuffer cb) noexcept {
    std::lock_guard lock(m_mtx);
    auto it = m_states.find(cb);
    return it != m_states.end() ? &it->second : nullptr;
}

void CommandBufferStateTracker::remove(VkCommandBuffer cb) noexcept {
    std::lock_guard lock(m_mtx);
    m_states.erase(cb);
    LOGD("CommandBufferStateTracker: removed state for CB %p", (void*)cb);
}

} // namespace gimi
