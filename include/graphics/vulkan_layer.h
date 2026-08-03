#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Vulkan Layer Entrypoints & Dispatch Table
//
// Implements the Khronos Vulkan Layer specification:
//   https://vulkan.lunarg.com/doc/view/latest/linux/layer_configuration.html
//
// The layer intercepts vkGetInstanceProcAddr / vkGetDeviceProcAddr to
// install function-pointer overrides without touching any on-disk files.
//
// Exported symbols (must be visible to the Vulkan loader):
//   vkGetInstanceProcAddr
//   vkGetDeviceProcAddr
//   vkNegotiateLoaderLayerInterfaceVersion
// ─────────────────────────────────────────────────────────────────────────────

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <string>
#include <mutex>

namespace gimi {

// ─── Instance Dispatch Table ──────────────────────────────────────────────────
// Stores the original function pointers obtained from the downstream Vulkan
// driver so that non-overridden calls can be forwarded transparently.
struct InstanceDispatchTable {
    PFN_vkGetInstanceProcAddr   GetInstanceProcAddr  = nullptr;
    PFN_vkDestroyInstance       DestroyInstance      = nullptr;
    PFN_vkCreateDevice          CreateDevice         = nullptr;
    // Extended in later phases as more entrypoints are intercepted.
};

// ─── Device Dispatch Table ────────────────────────────────────────────────────
struct DeviceDispatchTable {
    PFN_vkGetDeviceProcAddr          GetDeviceProcAddr         = nullptr;
    PFN_vkDestroyDevice              DestroyDevice             = nullptr;
    PFN_vkCmdDrawIndexed             CmdDrawIndexed            = nullptr;
    PFN_vkCmdBindVertexBuffers       CmdBindVertexBuffers      = nullptr;
    PFN_vkCmdBindDescriptorSets      CmdBindDescriptorSets     = nullptr;
    // Phase 2: hash eviction hooks
    PFN_vkDestroyBuffer              DestroyBuffer             = nullptr;
    PFN_vkDestroyImage               DestroyImage              = nullptr;
    // Phase 3: draw-call interception & shader fixes
    PFN_vkCmdBindIndexBuffer         CmdBindIndexBuffer        = nullptr;
    PFN_vkCreateShaderModule         CreateShaderModule        = nullptr;
    // Extended in Phase 4 (texture override).
};

// ─── Dispatch Table Registry ──────────────────────────────────────────────────
// Keyed by the dispatchable-handle's implicit dispatch key (first pointer-sized
// word of every dispatchable Vulkan handle) for O(1) per-call lookup.
class VulkanLayerRegistry {
public:
    static VulkanLayerRegistry& instance() noexcept;

    void register_instance(VkInstance inst, InstanceDispatchTable table) noexcept;
    void register_device(VkDevice dev, DeviceDispatchTable table) noexcept;

    const InstanceDispatchTable* get_instance(VkInstance inst) const noexcept;
    const DeviceDispatchTable*   get_device(VkDevice dev)      const noexcept;

    void remove_instance(VkInstance inst) noexcept;
    void remove_device(VkDevice dev) noexcept;

private:
    mutable std::mutex m_mtx;
    std::unordered_map<VkInstance, InstanceDispatchTable> m_instances;
    std::unordered_map<VkDevice,   DeviceDispatchTable>   m_devices;
};

} // namespace gimi

// ─── Exported Vulkan Layer Entrypoints ────────────────────────────────────────
// Must use C linkage and be visible to the dynamic linker.
extern "C" {

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance instance, const char* pName);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice device, const char* pName);

VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct);

} // extern "C"
