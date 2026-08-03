#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — DXGI to Vulkan Format Conversion Table
//
// Maps DirectX 11 DXGI_FORMAT enum values to their Vulkan VkFormat equivalents.
// Used by the VertexLayoutAdapter to translate 3dmigoto mod vertex attribute
// layouts (originally designed for DirectX 11) to Vulkan pipeline input state.
//
// Only the formats commonly found in 3dmigoto mods are mapped.
// Unknown formats return VK_FORMAT_UNDEFINED.
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <cstdint>

namespace gimi {

// DXGI_FORMAT values (from DirectX 11 / dxgiformat.h).
// We only define the subset actually used in 3dmigoto mod exports.
enum DxgiFormat : uint32_t {
    DXGI_FORMAT_UNKNOWN                     = 0,
    DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
    DXGI_FORMAT_R32G32B32A32_UINT           = 3,
    DXGI_FORMAT_R32G32B32A32_SINT           = 4,
    DXGI_FORMAT_R32G32B32_FLOAT             = 6,
    DXGI_FORMAT_R32G32B32_UINT              = 7,
    DXGI_FORMAT_R32G32B32_SINT              = 8,
    DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
    DXGI_FORMAT_R16G16B16A16_UINT           = 12,
    DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
    DXGI_FORMAT_R16G16B16A16_SINT           = 14,
    DXGI_FORMAT_R32G32_FLOAT                = 16,
    DXGI_FORMAT_R32G32_UINT                 = 17,
    DXGI_FORMAT_R32G32_SINT                 = 18,
    DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
    DXGI_FORMAT_R8G8B8A8_UINT               = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
    DXGI_FORMAT_R8G8B8A8_SINT               = 32,
    DXGI_FORMAT_R16G16_FLOAT                = 34,
    DXGI_FORMAT_R16G16_UNORM                = 35,
    DXGI_FORMAT_R16G16_UINT                 = 36,
    DXGI_FORMAT_R16G16_SNORM                = 37,
    DXGI_FORMAT_R16G16_SINT                 = 38,
    DXGI_FORMAT_R32_FLOAT                   = 41,
    DXGI_FORMAT_R32_UINT                    = 42,
    DXGI_FORMAT_R32_SINT                    = 43,
    DXGI_FORMAT_R8G8_UNORM                  = 49,
    DXGI_FORMAT_R8G8_UINT                   = 50,
    DXGI_FORMAT_R16_FLOAT                   = 54,
    DXGI_FORMAT_R16_UNORM                   = 56,
    DXGI_FORMAT_R16_UINT                    = 57,
    DXGI_FORMAT_R8_UNORM                    = 61,
    DXGI_FORMAT_R8_UINT                     = 62,
};

// Convert a DXGI_FORMAT value to the equivalent VkFormat.
// Returns VK_FORMAT_UNDEFINED for unrecognised formats.
VkFormat dxgi_to_vulkan_format(uint32_t dxgi_format) noexcept;

// Returns the byte size of a single element of the given DXGI_FORMAT.
// Returns 0 for unrecognised formats.
uint32_t dxgi_format_byte_size(uint32_t dxgi_format) noexcept;

} // namespace gimi
