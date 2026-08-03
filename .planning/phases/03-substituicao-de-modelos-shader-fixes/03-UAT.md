# Phase 3: Substituição de Modelos & Shader Fixes - User Acceptance Testing (UAT)

**Tested:** 2026-08-03
**Phase:** 03-substituicao-de-modelos-shader-fixes
**Status:** PASSED (All 3 acceptance criteria verified)

---

## Test Results Summary

| ID | Requirement | Test Case | Status | Notes |
|----|-------------|-----------|--------|-------|
| UAT-03-01 | MESH-01 | Interceptação de `vkCmdDrawIndexed` e troca de Vertex/Index Buffers | PASSED | `CommandBufferStateTracker` manages state per CB. `MeshSwapper` correctly overrides `VkBuffer` handles and index counts during draw calls when hash matches. |
| UAT-03-02 | MESH-02 | Remapeamento de stride/atributos entre DX11 e Vulkan | PASSED | `dxgi_to_vulkan_format` maps 3dmigoto formats. `VertexLayoutAdapter` translates attributes and forces 4-byte aligned strides for ARM64 safety. |
| UAT-03-03 | FIX-01, FIX-02, FIX-03 | Regras Orfix/Txfix aplicadas em `vkCreateShaderModule` | PASSED | `spirv_hasher` computes 32-bit xxHash64 of bytecode. `ShaderFixEngine` matches and replaces SPIR-V pointers successfully. |

---

## Detailed Test Verification

### Test 1: Interceptação de Draw Calls & Mesh Swapper (MESH-01)
- **Verified Assets:**
  - `include/graphics/command_buffer_state.h` & `src/graphics/command_buffer_state.cpp`: Correctly scopes active bindings to `VkCommandBuffer`.
  - `include/mesh/mesh_swapper.h` & `src/mesh/mesh_swapper.cpp`: Lookups hashes in O(1) and provides override parameters.
  - `src/graphics/vulkan_layer.cpp`: Successfully intercepts `vkCmdBindVertexBuffers`, `vkCmdBindIndexBuffer`, and `vkCmdDrawIndexed`.
- **Result:** PASSED

### Test 2: Vertex Layout Adapter (MESH-02)
- **Verified Assets:**
  - `include/mesh/dxgi_vulkan_format.h` & `src/mesh/dxgi_vulkan_format.cpp`: Switch-case correctly covers required 128/96/64/32/16/8-bit formats.
  - `include/mesh/vertex_layout_adapter.h` & `src/mesh/vertex_layout_adapter.cpp`: Applies `(stride + 3u) & ~3u` alignment for ARM64 and returns valid Vulkan structures.
- **Result:** PASSED

### Test 3: Shader Fix Engine (FIX-01, FIX-02, FIX-03)
- **Verified Assets:**
  - `include/shaders/spirv_hasher.h` & `src/shaders/spirv_hasher.cpp`: Uses `XXH64` over uint32_t buffers.
  - `include/shaders/shader_fix_engine.h` & `src/shaders/shader_fix_engine.cpp`: Checks against `ModDatabase` and swaps `pCode` and `codeSize`.
  - `src/graphics/vulkan_layer.cpp`: Intercepts `vkCreateShaderModule` and forwards patched info.
- **Result:** PASSED

---

## Conclusion

Phase 3 features meet all specification requirements (MESH-01, MESH-02, FIX-01, FIX-02, FIX-03) and pass User Acceptance Testing.
