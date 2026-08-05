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
#include "graphics/command_buffer_state.h"  // Phase 3: per-CB state
#include "hash/hash_registry.h"            // Phase 2: eviction hooks
#include "hash/resource_hash_engine.h"     // Phase 2: buffer hashing
#include "mesh/mesh_swapper.h"             // Phase 3: mesh override
#include "shaders/shader_fix_engine.h"     // Phase 3: shader fixes
#include "textures/image_state_tracker.h"  // Phase 4: texture tracking
#include "textures/texture_swapper.h"      // Phase 4: texture override
#include "utils/logger.h"

#include <vulkan/vk_layer.h>   // VkNegotiateLayerInterface, CURRENT_LOADER_LAYER_INTERFACE_VERSION
#include <cstring>
#include <vector>

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

const DeviceDispatchTable* VulkanLayerRegistry::get_any_device() const noexcept {
    std::lock_guard lock(m_mtx);
    if (!m_devices.empty()) {
        return &m_devices.begin()->second;
    }
    return nullptr;
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
        chain = reinterpret_cast<VkLayerInstanceCreateInfo*>(const_cast<void*>(chain->pNext));
    }
    if (chain) chain->u.pLayerInfo = chain->u.pLayerInfo->pNext;

    auto fpCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
            nextGIPA(VK_NULL_HANDLE, "vkCreateInstance"));
    if (!fpCreateInstance) return VK_ERROR_INITIALIZATION_FAILED;

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) return result;

    // Build and register the instance dispatch table with ALL function pointers
    // to prevent nullptr dereference crashes.
    InstanceDispatchTable table{};
    table.GetInstanceProcAddr = nextGIPA;
    table.DestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(
            nextGIPA(*pInstance, "vkDestroyInstance"));
    table.CreateDevice = reinterpret_cast<PFN_vkCreateDevice>(
            nextGIPA(*pInstance, "vkCreateDevice"));
    // Initialize remaining instance functions to prevent crashes
    table.EnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
            nextGIPA(*pInstance, "vkEnumerateInstanceExtensionProperties"));
    table.EnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
            nextGIPA(*pInstance, "vkEnumerateInstanceLayerProperties"));
    table.EnumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
            nextGIPA(*pInstance, "vkEnumerateInstanceVersion"));

    VulkanLayerRegistry::instance().register_instance(*pInstance, table);
    LOGI("vkCreateInstance: registered dispatch table for instance %p", (void*)*pInstance);
    return VK_SUCCESS;
}

// ─── vkCreateDevice Intercept ───────────────────────────────────────────────
static VKAPI_ATTR VkResult VKAPI_CALL
gimi_vkCreateDevice(
        VkPhysicalDevice physicalDevice,
        const VkDeviceCreateInfo* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDevice* pDevice) noexcept {
    
    // Walk the pNext chain to get the layer's GetInstanceProcAddr
    PFN_vkGetInstanceProcAddr nextGIPA = nullptr;
    PFN_vkGetDeviceProcAddr nextGDPA = nullptr;
    
    if (pCreateInfo && pCreateInfo->pNext) {
        auto* chain = reinterpret_cast<const VkLayerDeviceCreateInfo*>(pCreateInfo->pNext);
        while (chain) {
            if (chain->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO &&
                chain->function == VK_LAYER_LINK_INFO) {
                nextGIPA = chain->u.pLayerInfo->pfnNextGetInstanceProcAddr;
                nextGDPA = chain->u.pLayerInfo->pfnNextGetDeviceProcAddr;
                // Advance the chain for the next layer
                const_cast<VkLayerDeviceCreateInfo*>(chain)->u.pLayerInfo = 
                    chain->u.pLayerInfo->pNext;
                break;
            }
            chain = reinterpret_cast<const VkLayerDeviceCreateInfo*>(chain->pNext);
        }
    }
    
    if (!nextGIPA) {
        LOGE("gimi_vkCreateDevice: cannot find next GetInstanceProcAddr in chain");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Get the real vkCreateDevice from the next layer/driver
    auto fpCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(
            nextGIPA(VK_NULL_HANDLE, "vkCreateDevice"));
    if (!fpCreateDevice) {
        LOGE("gimi_vkCreateDevice: cannot resolve vkCreateDevice");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkResult result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS) return result;

    // Build and register the device dispatch table with ALL function pointers
    // to prevent nullptr dereference crashes.
    DeviceDispatchTable table{};
    
    // Use nextGDPA if available, otherwise use nextGIPA
    auto get_dev_proc = nextGDPA ? nextGDPA : 
        reinterpret_cast<PFN_vkGetDeviceProcAddr>(nextGIPA(VK_NULL_HANDLE, "vkGetDeviceProcAddr"));
    
    table.GetDeviceProcAddr = get_dev_proc;
    table.DestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(
            get_dev_proc(*pDevice, "vkDestroyDevice"));
    table.DestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(
            get_dev_proc(*pDevice, "vkDestroyBuffer"));
    table.DestroyImage = reinterpret_cast<PFN_vkDestroyImage>(
            get_dev_proc(*pDevice, "vkDestroyImage"));
    table.CmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(
            get_dev_proc(*pDevice, "vkCmdBindVertexBuffers"));
    table.CmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(
            get_dev_proc(*pDevice, "vkCmdBindIndexBuffer"));
    table.CmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(
            get_dev_proc(*pDevice, "vkCmdDrawIndexed"));
    table.CreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(
            get_dev_proc(*pDevice, "vkCreateShaderModule"));
    table.CreateImageView = reinterpret_cast<PFN_vkCreateImageView>(
            get_dev_proc(*pDevice, "vkCreateImageView"));
    table.DestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(
            get_dev_proc(*pDevice, "vkDestroyImageView"));
    table.CmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(
            get_dev_proc(*pDevice, "vkCmdCopyBufferToImage"));
    table.UpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(
            get_dev_proc(*pDevice, "vkUpdateDescriptorSets"));
    table.AllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(
            get_dev_proc(*pDevice, "vkAllocateCommandBuffers"));
    table.FreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(
            get_dev_proc(*pDevice, "vkFreeCommandBuffers"));

    VulkanLayerRegistry::instance().register_device(*pDevice, table);
    LOGI("gimi_vkCreateDevice: registered dispatch table for device %p", (void*)*pDevice);
    return VK_SUCCESS;
}

// ─── vkDestroyDevice Intercept ───────────────────────────────────────────────
static VKAPI_ATTR void VKAPI_CALL
gimi_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) noexcept {
    const DeviceDispatchTable* table = VulkanLayerRegistry::instance().get_device(device);
    if (table && table->DestroyDevice) {
        table->DestroyDevice(device, pAllocator);
    }
    VulkanLayerRegistry::instance().remove_device(device);
    LOGD("gimi_vkDestroyDevice: removed dispatch table for device %p", (void*)device);
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
    ImageStateTracker::instance().remove_image(image); // Phase 4
    const DeviceDispatchTable* table = VulkanLayerRegistry::instance().get_device(device);
    if (table && table->DestroyImage) {
        table->DestroyImage(device, image, pAllocator);
    }
}

// ─── Phase 3: Draw-Call Interception ──────────────────────────────────────
// Intercept vertex/index buffer binds and draw calls for mesh swapping.

// Helper to get any available device for command buffer forwarding
static const DeviceDispatchTable* get_any_device_table() noexcept {
    return VulkanLayerRegistry::instance().get_any_device();
}

static VKAPI_ATTR void VKAPI_CALL
gimi_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer,
                            uint32_t firstBinding, uint32_t bindingCount,
                            const VkBuffer* pBuffers,
                            const VkDeviceSize* pOffsets) noexcept {
    // Track the bind in our state tracker
    CommandBufferStateTracker::instance().bind_vertex_buffers(
        commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);

    // Forward to the real driver
    const DeviceDispatchTable* table = get_any_device_table();
    if (table && table->CmdBindVertexBuffers) {
        table->CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    }
}

static VKAPI_ATTR void VKAPI_CALL
gimi_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer,
                           VkBuffer buffer, VkDeviceSize offset,
                           VkIndexType indexType) noexcept {
    CommandBufferStateTracker::instance().bind_index_buffer(
        commandBuffer, buffer, offset, indexType);

    // Forward to the real driver
    const DeviceDispatchTable* table = get_any_device_table();
    if (table && table->CmdBindIndexBuffer) {
        table->CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    }
}

static VKAPI_ATTR void VKAPI_CALL
gimi_vkCmdDrawIndexed(VkCommandBuffer commandBuffer,
                       uint32_t indexCount, uint32_t instanceCount,
                       uint32_t firstIndex, int32_t vertexOffset,
                       uint32_t firstInstance) noexcept {
    // Check if the currently bound vertex buffer should be overridden
    auto* state = CommandBufferStateTracker::instance().get(commandBuffer);
    if (state && !state->vertex_buffers.empty()) {
        auto& vb0 = state->vertex_buffers[0];

        // Lazy hash computation (only on first draw with this buffer)
        if (vb0.hash == 0 && vb0.buffer != VK_NULL_HANDLE) {
            auto cached = HashRegistry::instance().get_buffer(vb0.buffer);
            if (cached.has_value()) {
                vb0.hash = cached->hash32;
            }
        }

        // Attempt mesh swap
        if (vb0.hash != 0) {
            auto result = MeshSwapper::instance().try_swap(vb0.hash);
            if (result.should_override) {
                indexCount   = result.index_count;
                firstIndex   = result.first_index;
                vertexOffset = static_cast<int32_t>(result.vertex_offset);
                LOGD("gimi_vkCmdDrawIndexed: swapped mesh for hash 0x%08X", vb0.hash);
            }
        }
    }

    // Forward to the real driver
    const DeviceDispatchTable* table = get_any_device_table();
    if (table && table->CmdDrawIndexed) {
        table->CmdDrawIndexed(commandBuffer, indexCount, instanceCount, 
                              firstIndex, vertexOffset, firstInstance);
    }
}

// ─── Phase 3: Shader Module Creation Intercept ───────────────────────────
static VKAPI_ATTR VkResult VKAPI_CALL
gimi_vkCreateShaderModule(VkDevice device,
                           const VkShaderModuleCreateInfo* pCreateInfo,
                           const VkAllocationCallbacks* pAllocator,
                           VkShaderModule* pShaderModule) noexcept {
    // Attempt to apply Orfix/Txfix shader patches
    std::vector<uint32_t> patched_code;
    bool patched = ShaderFixEngine::instance().process_shader_module(
        *pCreateInfo, patched_code);

    const DeviceDispatchTable* table = VulkanLayerRegistry::instance().get_device(device);
    if (!table || !table->CreateShaderModule) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (patched && !patched_code.empty()) {
        // Create a modified create info pointing to the patched bytecode
        VkShaderModuleCreateInfo patched_info = *pCreateInfo;
        patched_info.pCode    = patched_code.data();
        patched_info.codeSize = patched_code.size() * sizeof(uint32_t);
        return table->CreateShaderModule(device, &patched_info, pAllocator, pShaderModule);
    }

    // No patch — forward unchanged
    return table->CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}

// ─── Phase 4: Texture Pipeline Intercepts ────────────────────────────────

static VKAPI_ATTR VkResult VKAPI_CALL
gimi_vkCreateImageView(VkDevice device,
                       const VkImageViewCreateInfo* pCreateInfo,
                       const VkAllocationCallbacks* pAllocator,
                       VkImageView* pView) noexcept {
    const DeviceDispatchTable* table = VulkanLayerRegistry::instance().get_device(device);
    if (!table || !table->CreateImageView) return VK_ERROR_INITIALIZATION_FAILED;

    VkResult result = table->CreateImageView(device, pCreateInfo, pAllocator, pView);
    if (result == VK_SUCCESS && pCreateInfo) {
        ImageStateTracker::instance().register_image_view(*pView, pCreateInfo->image);
    }
    return result;
}

static VKAPI_ATTR void VKAPI_CALL
gimi_vkDestroyImageView(VkDevice device, VkImageView imageView,
                        const VkAllocationCallbacks* pAllocator) noexcept {
    ImageStateTracker::instance().remove_image_view(imageView);
    const DeviceDispatchTable* table = VulkanLayerRegistry::instance().get_device(device);
    if (table && table->DestroyImageView) {
        table->DestroyImageView(device, imageView, pAllocator);
    }
}

static VKAPI_ATTR void VKAPI_CALL
gimi_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer,
                            VkBuffer srcBuffer,
                            VkImage dstImage,
                            VkImageLayout dstImageLayout,
                            uint32_t regionCount,
                            const VkBufferImageCopy* pRegions) noexcept {
    // Phase 4: Hash texture content from srcBuffer.
    // In a full implementation, we'd queue an asynchronous compute shader or
    // map the memory if host-visible. For MVP structure, we assume we can read it,
    // but without full memory tracking we just simulate hashing for now or rely
    // on a previously computed buffer hash if it exists in HashRegistry.
    auto cached = HashRegistry::instance().get_buffer(srcBuffer);
    if (cached.has_value() && cached->hash32 != 0) {
        ImageStateTracker::instance().register_image_hash(dstImage, cached->hash32);
        LOGD("gimi_vkCmdCopyBufferToImage: Image %p linked to buffer hash 0x%08X", (void*)dstImage, cached->hash32);
    }

    // Forward to the real driver
    const DeviceDispatchTable* table = get_any_device_table();
    if (table && table->CmdCopyBufferToImage) {
        table->CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, 
                                    dstImageLayout, regionCount, pRegions);
    }
}

static VKAPI_ATTR void VKAPI_CALL
gimi_vkUpdateDescriptorSets(VkDevice device,
                            uint32_t descriptorWriteCount,
                            const VkWriteDescriptorSet* pDescriptorWrites,
                            uint32_t descriptorCopyCount,
                            const VkCopyDescriptorSet* pDescriptorCopies) noexcept {
    
    // We may need to modify the pDescriptorWrites array.
    std::vector<VkWriteDescriptorSet> patched_writes(descriptorWriteCount);
    // Keep patched image infos alive during the downstream call
    std::vector<std::vector<VkDescriptorImageInfo>> patched_image_infos(descriptorWriteCount);

    bool modified = false;

    for (uint32_t i = 0; i < descriptorWriteCount; ++i) {
        patched_writes[i] = pDescriptorWrites[i];
        const auto& write = pDescriptorWrites[i];

        if (write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
            write.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
            
            bool write_modified = false;
            std::vector<VkDescriptorImageInfo> new_image_infos(write.descriptorCount);

            for (uint32_t j = 0; j < write.descriptorCount; ++j) {
                new_image_infos[j] = write.pImageInfo[j];

                if (write.pImageInfo && write.pImageInfo[j].imageView != VK_NULL_HANDLE) {
                    auto hash = ImageStateTracker::instance().get_hash_for_view(write.pImageInfo[j].imageView);
                    if (hash.has_value()) {
                        auto replacement = TextureSwapper::instance().try_swap(hash.value());
                        if (replacement.has_value()) {
                            new_image_infos[j].imageView = replacement.value();
                            write_modified = true;
                            modified = true;
                            LOGD("gimi_vkUpdateDescriptorSets: Swapped texture hash 0x%08X", hash.value());
                        }
                    }
                }
            }

            if (write_modified) {
                patched_image_infos[i] = std::move(new_image_infos);
                patched_writes[i].pImageInfo = patched_image_infos[i].data();
            }
        }
    }

    const DeviceDispatchTable* table = VulkanLayerRegistry::instance().get_device(device);
    if (table && table->UpdateDescriptorSets) {
        table->UpdateDescriptorSets(device, descriptorWriteCount, 
                                    modified ? patched_writes.data() : pDescriptorWrites, 
                                    descriptorCopyCount, pDescriptorCopies);
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
    if (strcmp(pName, "vkCreateDevice")         == 0) return (PFN_vkVoidFunction)gimi::gimi_vkCreateDevice;

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
    // ── Phase 1: device lifecycle ───────────────────────────────────────────
    if (strcmp(pName, "vkDestroyDevice")     == 0) return (PFN_vkVoidFunction)gimi::gimi_vkDestroyDevice;
    // ── Phase 2: hash eviction intercepts ──────────────────────────────────
    if (strcmp(pName, "vkDestroyBuffer") == 0) return (PFN_vkVoidFunction)gimi::gimi_vkDestroyBuffer;
    if (strcmp(pName, "vkDestroyImage")  == 0) return (PFN_vkVoidFunction)gimi::gimi_vkDestroyImage;

    // ── Phase 3: draw-call interception & shader fixes ───────────────────
    if (strcmp(pName, "vkCmdBindVertexBuffers") == 0) return (PFN_vkVoidFunction)gimi::gimi_vkCmdBindVertexBuffers;
    if (strcmp(pName, "vkCmdBindIndexBuffer")  == 0) return (PFN_vkVoidFunction)gimi::gimi_vkCmdBindIndexBuffer;
    if (strcmp(pName, "vkCmdDrawIndexed")      == 0) return (PFN_vkVoidFunction)gimi::gimi_vkCmdDrawIndexed;
    if (strcmp(pName, "vkCreateShaderModule")  == 0) return (PFN_vkVoidFunction)gimi::gimi_vkCreateShaderModule;

    // ── Phase 4: texture pipeline ────────────────────────────────────────────
    if (strcmp(pName, "vkCreateImageView")       == 0) return (PFN_vkVoidFunction)gimi::gimi_vkCreateImageView;
    if (strcmp(pName, "vkDestroyImageView")      == 0) return (PFN_vkVoidFunction)gimi::gimi_vkDestroyImageView;
    if (strcmp(pName, "vkCmdCopyBufferToImage")  == 0) return (PFN_vkVoidFunction)gimi::gimi_vkCmdCopyBufferToImage;
    if (strcmp(pName, "vkUpdateDescriptorSets")  == 0) return (PFN_vkVoidFunction)gimi::gimi_vkUpdateDescriptorSets;

    // ── Forward everything else to the downstream driver ────────────────────
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
