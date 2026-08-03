// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Vertex Layout Adapter Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "mesh/vertex_layout_adapter.h"
#include "mesh/dxgi_vulkan_format.h"
#include "utils/logger.h"

#include <algorithm>
#include <unordered_map>
#include <cstring>

namespace gimi {

// ─── VertexLayoutAdapter::adapt ───────────────────────────────────────────────
bool VertexLayoutAdapter::adapt(
        const std::vector<DX11InputElement>& elements,
        AdaptedVertexLayout& out_layout) noexcept {

    out_layout.bindings.clear();
    out_layout.attributes.clear();

    // Group elements by input_slot (binding) and compute per-slot stride.
    // key: input_slot → accumulated stride
    std::unordered_map<uint32_t, uint32_t> slot_strides;

    uint32_t location = 0;
    for (const auto& elem : elements) {
        VkFormat vk_fmt = dxgi_to_vulkan_format(elem.dxgi_format);
        if (vk_fmt == VK_FORMAT_UNDEFINED) {
            LOGW("VertexLayoutAdapter: unsupported DXGI_FORMAT %u for semantic '%s'",
                 elem.dxgi_format, elem.semantic_name.c_str());
            return false;
        }

        uint32_t elem_size = dxgi_format_byte_size(elem.dxgi_format);

        VertexAttribute attr;
        attr.location = location++;
        attr.binding  = elem.input_slot;
        attr.offset   = elem.byte_offset;
        attr.format   = vk_fmt;
        out_layout.attributes.push_back(attr);

        // Track maximum extent for stride calculation
        uint32_t end_offset = elem.byte_offset + elem_size;
        auto it = slot_strides.find(elem.input_slot);
        if (it == slot_strides.end() || end_offset > it->second) {
            slot_strides[elem.input_slot] = end_offset;
        }
    }

    // Build binding descriptors
    for (auto& [slot, stride] : slot_strides) {
        // ARM64 alignment: round up stride to 4-byte boundary.
        // Mali and Adreno GPUs may fault on unaligned vertex fetches.
        stride = (stride + 3u) & ~3u;

        VertexBinding binding;
        binding.binding    = slot;
        binding.stride     = stride;
        binding.input_rate = VK_VERTEX_INPUT_RATE_VERTEX;
        out_layout.bindings.push_back(binding);
    }

    // Sort bindings by slot for deterministic ordering
    std::sort(out_layout.bindings.begin(), out_layout.bindings.end(),
              [](const VertexBinding& a, const VertexBinding& b) {
                  return a.binding < b.binding;
              });

    LOGD("VertexLayoutAdapter: adapted %zu elements → %zu bindings, %zu attributes",
         elements.size(), out_layout.bindings.size(), out_layout.attributes.size());
    return true;
}

// ─── AdaptedVertexLayout::fill_pipeline_state ─────────────────────────────────
void AdaptedVertexLayout::fill_pipeline_state(
        VkPipelineVertexInputStateCreateInfo& out_state) const noexcept {
    // We build temporary arrays on the stack; caller must ensure the
    // AdaptedVertexLayout outlives the returned pointers.
    // In practice this is used during pipeline creation which is a one-shot.
    static thread_local std::vector<VkVertexInputBindingDescription>   vk_bindings;
    static thread_local std::vector<VkVertexInputAttributeDescription> vk_attrs;

    vk_bindings.clear();
    vk_attrs.clear();

    for (const auto& b : bindings) {
        VkVertexInputBindingDescription desc{};
        desc.binding   = b.binding;
        desc.stride    = b.stride;
        desc.inputRate = b.input_rate;
        vk_bindings.push_back(desc);
    }

    for (const auto& a : attributes) {
        VkVertexInputAttributeDescription desc{};
        desc.location = a.location;
        desc.binding  = a.binding;
        desc.format   = a.format;
        desc.offset   = a.offset;
        vk_attrs.push_back(desc);
    }

    out_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    out_state.pNext = nullptr;
    out_state.flags = 0;
    out_state.vertexBindingDescriptionCount   = static_cast<uint32_t>(vk_bindings.size());
    out_state.pVertexBindingDescriptions      = vk_bindings.data();
    out_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(vk_attrs.size());
    out_state.pVertexAttributeDescriptions    = vk_attrs.data();
}

} // namespace gimi
