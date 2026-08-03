#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Vertex Layout Adapter (DX11 → Vulkan)
//
// Translates DirectX 11 vertex input element descriptions (as used by
// 3dmigoto mods) into Vulkan's VkVertexInputBindingDescription and
// VkVertexInputAttributeDescription structures.
//
// ARM64 alignment: strides are aligned to 4-byte boundaries to prevent
// unaligned access faults on Mali and Adreno GPUs.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>
#include <vector>

namespace gimi {

// ─── DX11 Input Element (3dmigoto format) ─────────────────────────────────────
// Mirrors the fields exported by 3dmigoto in mod .txt descriptor files.
struct DX11InputElement {
    std::string semantic_name;  // e.g. "POSITION", "NORMAL", "TEXCOORD"
    uint32_t    semantic_index = 0;
    uint32_t    dxgi_format    = 0;  // DxgiFormat enum value
    uint32_t    input_slot     = 0;  // binding slot (0 = primary VB)
    uint32_t    byte_offset    = 0;  // offset within stride
};

// ─── Vulkan Vertex Attribute ──────────────────────────────────────────────────
struct VertexAttribute {
    uint32_t location = 0;
    uint32_t binding  = 0;
    uint32_t offset   = 0;
    VkFormat format   = VK_FORMAT_UNDEFINED;
};

// ─── Vulkan Vertex Binding ────────────────────────────────────────────────────
struct VertexBinding {
    uint32_t binding  = 0;
    uint32_t stride   = 0;
    VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX;
};

// ─── Adapted Layout ───────────────────────────────────────────────────────────
struct AdaptedVertexLayout {
    std::vector<VertexBinding>   bindings;
    std::vector<VertexAttribute> attributes;

    // Generate Vulkan pipeline create info arrays (non-owning pointers).
    // Caller must ensure the AdaptedVertexLayout outlives the pointers.
    void fill_pipeline_state(
        VkPipelineVertexInputStateCreateInfo& out_state) const noexcept;
};

// ─── VertexLayoutAdapter ──────────────────────────────────────────────────────
class VertexLayoutAdapter {
public:
    // Convert a list of DX11 input elements into Vulkan vertex bindings and
    // attributes. Automatically computes strides per binding slot.
    // Returns false if any element has an unsupported DXGI format.
    bool adapt(const std::vector<DX11InputElement>& elements,
               AdaptedVertexLayout& out_layout) noexcept;
};

} // namespace gimi
