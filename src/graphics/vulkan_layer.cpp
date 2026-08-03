// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Vulkan Layer Implementation
//
// Implements the Khronos Layer Intercept pattern:
//   1. Intercept vkGetInstanceProcAddr / vkGetDeviceProcAddr.
//   2. For functions we override, return our wrapper.
//   3. For all others, forward transparently to the next layer / driver.
//
// Non-destructive guarantee: only function-pointer tables in RAM are modified;
// no ELF patching, no APK modification, no on-disk file writes.
// ─────────────────────────────────────────────────────────────────────────────

#include "graphics/vulkan_layer.h"
#include "hash/hash_registry.h"   // Phase 2: eviction hooks
#include "utils/logger.h"

#include <vulkan/vk_layer.h>   // VkNegotiateLayerInterface, CURRENT_LOADER_LAYER_INTERFACE_VERSION
#include <cstring>

namespace gimi {

// ─── VulkanLayerRegistry ──────────────────────────────────────────────────────
VulkanLayerRegistry& VulkanLayerRegistry::instance() noexcept {
    static VulkanLayerRegistry s_registry;
    return s_registry;
}

void VulkanLayerRegistry::register_instance(VkInstance inst, InstanceDispatchTable table) noexcept {
    std::lock_guard lock(m_mtx);
    m_instances[inst] = std::move(table);
}

void VulkanLayerRegistry::register_device(VkDevice dev, DeviceDispatchTable table) noexcept {
    std::lock_guard lock(m_mtx);
    m_devices[dev] = std::move(table);
}

const InstanceDispatchTable* VulkanLayerRegistry::get_instance(VkInstance inst) const noexcept {
    std::lock_guard lock(m_mtx);
    auto it = m_instances.find(inst);
    return it != m_instances.end() ? &it->second : nullptr;
}

const DeviceDispatchTable* VulkanLayerRegistry::get_device(VkDevice dev) const noexcept {
    std::lock_guard lock(m_mtx);
    auto it = m_devices.find(dev);
    return it != m_devices.end() ? &it->second : nullptr;
}

void VulkanLayerRegistry::remove_instance(VkInstance inst) noexcept {
    std::lock_guard lock(m_mtx);
    m_instances.erase(inst);
}

void VulkanLayerRegistry::remove_device(VkDevice dev) noexcept {
    std::lock_guard lock(m_mtx);
    m_devices.erase(dev);
}

// ─── Internal Helpers ─────────────────────────────────────────────────────────

// Retrieve the downstream GetInstanceProcAddr from the loader's chain.
// The loader passes it via the layer-create-info chain (pCreateInfo->pNext).
static PFN_vkGetInstanceProcAddr get_next_instance_proc_addr(
        [[maybe_unused]] VkInstance instance,
        [[maybe_unused]] const VkInstanceCreateInfo* pCreateInfo) noexcept {
    // Walk the pNext chain to locate VkLayerInstanceCreateInfo with
    // function == VK_LAYER_LINK_INFO to extract the next GetInstanceProcAddr.
    if (pCreateInfo) {
        auto* chain = reinterpret_cast<const VkLayerInstanceCreateInfo*>(pCreateInfo->pNext);
        while (chain) {
            if (chain->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO &&
                chain->function == VK_LAYER_LINK_INFO) {
                return chain->u.pLayerInfo->pfnNextGetInstanceProcAddr;
            }
            chain = reinterpret_cast<const VkLayerInstanceCreateInfo*>(chain->pNext);
        }
    }
    return nullptr;
}

// ─── vkCreateInstance Intercept ───────────────────────────────────────────────
static VKAPI_ATTR VkResult VKAPI_CALL
gimi_vkCreateInstance(
        const VkInstanceCreateInfo*  pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkInstance*                  pInstance) noexcept {

    PFN_vkGetInstanceProcAddr nextGIPA = get_next_instance_proc_addr(nullptr, pCreateInfo);
    if (!nextGIPA) {
        LOGE("vkCreateInstance: cannot find next GetInstanceProcAddr in chain");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the loader chain before calling downstream CreateInstance.
    auto* chain = reinterpret_cast<VkLayerInstanceCreateInfo*>(
            const_cast<void*>(reinterpret_cast<const void*>(pCreateInfo->pNext)));
    while (chain && !(chain->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO &&
                      chain->function == VK_LAYER_LINK_INFO)) {
        chain = reinterpret_cast<VkLayerInstanceCreateInfo*>(chain->pNext);
    }
    if (chain) chain->u.pLayerInfo = chain->u.pLayerInfo->pNext;

    auto fpCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
            nextGIPA(VK_NULL_HANDLE, "vkCreateInstance"));
    if (!fpCreateInstance) return VK_ERROR_INITIALIZATION_FAILED;

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) return result;

    // Build and register the instance dispatch table.
    InstanceDispatchTable table{};
    table.GetInstanceProcAddr = nextGIPA;
    table.DestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(
            nextGIPA(*pInstance, "vkDestroyInstance"));
    table.CreateDevice = reinterpret_cast<PFN_vkCreateDevice>(
            nextGIPA(*pInstance, "vkCreateDevice"));

    VulkanLayerRegistry::instance().register_instance(*pInstance, table);
    LOGI("vkCreateInstance: registered dispatch table for instance %p", (void*)*pInstance);
    return VK_SUCCESS;
}

// ─── vkDestroyInstance Intercept ──────────────────────────────────────────────
static VKAPI_ATTR void VKAPI_CALL
gimi_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) noexcept {
    const InstanceDispatchTable* table = VulkanLayerRegistry::instance().get_instance(instance);
    if (table && table->DestroyInstance) {
        table->DestroyInstance(instance, pAllocator);
    }
    VulkanLayerRegistry::instance().remove_instance(instance);
    LOGD("vkDestroyInstance: removed dispatch table for instance %p", (void*)instance);
}

// ─── Phase 2: vkDestroyBuffer / vkDestroyImage Intercepts ────────────────────
// Evict hashes from HashRegistry before forwarding to the driver.
static VKAPI_ATTR void VKAPI_CALL
gimi_vkDestroyBuffer(VkDevice device, VkBuffer buffer,
                     const VkAllocationCallbacks* pAllocator) noexcept {
    HashRegistry::instance().evict_buffer(buffer);
    const DeviceDispatchTable* table = VulkanLayerRegistry::instance().get_device(device);
    if (table && table->DestroyBuffer) {
        table->DestroyBuffer(device, buffer, pAllocator);
    }
}

static VKAPI_ATTR void VKAPI_CALL
gimi_vkDestroyImage(VkDevice device, VkImage image,
                    const VkAllocationCallbacks* pAllocator) noexcept {
    HashRegistry::instance().evict_image(image);
    const DeviceDispatchTable* table = VulkanLayerRegistry::instance().get_device(device);
    if (table && table->DestroyImage) {
        table->DestroyImage(device, image, pAllocator);
    }
}

} // namespace gimi

// ─── Exported Vulkan Layer Entrypoints ────────────────────────────────────────
extern "C" {

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
    // ── Intercept our overridden functions ──────────────────────────────────
    if (strcmp(pName, "vkGetInstanceProcAddr")  == 0) return (PFN_vkVoidFunction)vkGetInstanceProcAddr;
    if (strcmp(pName, "vkGetDeviceProcAddr")    == 0) return (PFN_vkVoidFunction)vkGetDeviceProcAddr;
    if (strcmp(pName, "vkCreateInstance")       == 0) return (PFN_vkVoidFunction)gimi::gimi_vkCreateInstance;
    if (strcmp(pName, "vkDestroyInstance")      == 0) return (PFN_vkVoidFunction)gimi::gimi_vkDestroyInstance;

    // ── Forward everything else to the downstream driver ────────────────────
    const gimi::InstanceDispatchTable* table =
            gimi::VulkanLayerRegistry::instance().get_instance(instance);
    if (table && table->GetInstanceProcAddr) {
        return table->GetInstanceProcAddr(instance, pName);
    }
    return nullptr;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice device, const char* pName) {
    // ── Phase 2: hash eviction intercepts ──────────────────────────────────
    if (strcmp(pName, "vkDestroyBuffer") == 0) return (PFN_vkVoidFunction)gimi::gimi_vkDestroyBuffer;
    if (strcmp(pName, "vkDestroyImage")  == 0) return (PFN_vkVoidFunction)gimi::gimi_vkDestroyImage;

    // ── Forward everything else to the downstream driver ────────────────────
    // Phase 3 will add draw-call intercepts here.
    const gimi::DeviceDispatchTable* table =
            gimi::VulkanLayerRegistry::instance().get_device(device);
    if (table && table->GetDeviceProcAddr) {
        return table->GetDeviceProcAddr(device, pName);
    }
    return nullptr;
}

VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
    // Advertise the maximum interface version we support.
    if (pVersionStruct->loaderLayerInterfaceVersion > CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        pVersionStruct->loaderLayerInterfaceVersion = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
    }
    pVersionStruct->pfnGetInstanceProcAddr       = vkGetInstanceProcAddr;
    pVersionStruct->pfnGetDeviceProcAddr         = vkGetDeviceProcAddr;
    pVersionStruct->pfnGetPhysicalDeviceProcAddr = nullptr;

    LOGI("vkNegotiateLoaderLayerInterfaceVersion: negotiated version %u",
         pVersionStruct->loaderLayerInterfaceVersion);
    return VK_SUCCESS;
}

} // extern "C"
