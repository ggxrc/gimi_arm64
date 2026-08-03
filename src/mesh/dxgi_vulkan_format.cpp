// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — DXGI to Vulkan Format Conversion Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "mesh/dxgi_vulkan_format.h"

namespace gimi {

VkFormat dxgi_to_vulkan_format(uint32_t dxgi_format) noexcept {
    switch (static_cast<DxgiFormat>(dxgi_format)) {
        // ── 128-bit (4×32) ────────────────────────────────────────────────────
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case DXGI_FORMAT_R32G32B32A32_UINT:  return VK_FORMAT_R32G32B32A32_UINT;
        case DXGI_FORMAT_R32G32B32A32_SINT:  return VK_FORMAT_R32G32B32A32_SINT;

        // ── 96-bit (3×32) ─────────────────────────────────────────────────────
        case DXGI_FORMAT_R32G32B32_FLOAT:    return VK_FORMAT_R32G32B32_SFLOAT;
        case DXGI_FORMAT_R32G32B32_UINT:     return VK_FORMAT_R32G32B32_UINT;
        case DXGI_FORMAT_R32G32B32_SINT:     return VK_FORMAT_R32G32B32_SINT;

        // ── 64-bit (4×16) ─────────────────────────────────────────────────────
        case DXGI_FORMAT_R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case DXGI_FORMAT_R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
        case DXGI_FORMAT_R16G16B16A16_UINT:  return VK_FORMAT_R16G16B16A16_UINT;
        case DXGI_FORMAT_R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
        case DXGI_FORMAT_R16G16B16A16_SINT:  return VK_FORMAT_R16G16B16A16_SINT;

        // ── 64-bit (2×32) ─────────────────────────────────────────────────────
        case DXGI_FORMAT_R32G32_FLOAT:       return VK_FORMAT_R32G32_SFLOAT;
        case DXGI_FORMAT_R32G32_UINT:        return VK_FORMAT_R32G32_UINT;
        case DXGI_FORMAT_R32G32_SINT:        return VK_FORMAT_R32G32_SINT;

        // ── 32-bit (4×8) ──────────────────────────────────────────────────────
        case DXGI_FORMAT_R8G8B8A8_UNORM:     return VK_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_R8G8B8A8_UINT:      return VK_FORMAT_R8G8B8A8_UINT;
        case DXGI_FORMAT_R8G8B8A8_SNORM:     return VK_FORMAT_R8G8B8A8_SNORM;
        case DXGI_FORMAT_R8G8B8A8_SINT:      return VK_FORMAT_R8G8B8A8_SINT;

        // ── 32-bit (2×16) ─────────────────────────────────────────────────────
        case DXGI_FORMAT_R16G16_FLOAT:       return VK_FORMAT_R16G16_SFLOAT;
        case DXGI_FORMAT_R16G16_UNORM:       return VK_FORMAT_R16G16_UNORM;
        case DXGI_FORMAT_R16G16_UINT:        return VK_FORMAT_R16G16_UINT;
        case DXGI_FORMAT_R16G16_SNORM:       return VK_FORMAT_R16G16_SNORM;
        case DXGI_FORMAT_R16G16_SINT:        return VK_FORMAT_R16G16_SINT;

        // ── 32-bit (1×32) ─────────────────────────────────────────────────────
        case DXGI_FORMAT_R32_FLOAT:          return VK_FORMAT_R32_SFLOAT;
        case DXGI_FORMAT_R32_UINT:           return VK_FORMAT_R32_UINT;
        case DXGI_FORMAT_R32_SINT:           return VK_FORMAT_R32_SINT;

        // ── 16-bit (2×8) ──────────────────────────────────────────────────────
        case DXGI_FORMAT_R8G8_UNORM:         return VK_FORMAT_R8G8_UNORM;
        case DXGI_FORMAT_R8G8_UINT:          return VK_FORMAT_R8G8_UINT;

        // ── 16-bit (1×16) ─────────────────────────────────────────────────────
        case DXGI_FORMAT_R16_FLOAT:          return VK_FORMAT_R16_SFLOAT;
        case DXGI_FORMAT_R16_UNORM:          return VK_FORMAT_R16_UNORM;
        case DXGI_FORMAT_R16_UINT:           return VK_FORMAT_R16_UINT;

        // ── 8-bit (1×8) ───────────────────────────────────────────────────────
        case DXGI_FORMAT_R8_UNORM:           return VK_FORMAT_R8_UNORM;
        case DXGI_FORMAT_R8_UINT:            return VK_FORMAT_R8_UINT;

        default:                             return VK_FORMAT_UNDEFINED;
    }
}

uint32_t dxgi_format_byte_size(uint32_t dxgi_format) noexcept {
    switch (static_cast<DxgiFormat>(dxgi_format)) {
        // 128-bit = 16 bytes
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:  return 16;

        // 96-bit = 12 bytes
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:     return 12;

        // 64-bit = 8 bytes
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:        return 8;

        // 32-bit = 4 bytes
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:           return 4;

        // 16-bit = 2 bytes
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:           return 2;

        // 8-bit = 1 byte
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:            return 1;

        default:                             return 0;
    }
}

} // namespace gimi
