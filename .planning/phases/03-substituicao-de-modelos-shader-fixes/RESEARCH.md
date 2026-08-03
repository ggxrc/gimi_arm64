# Phase 3: Substituição de Modelos & Shader Fixes - Research

**Researched:** 2026-08-03
**Phase:** 03-substituicao-de-modelos-shader-fixes
**Depends on:** Phase 1 (Vulkan Hooking) & Phase 2 (3dmigoto Parser & Hash Engine)

## Executive Summary

A Fase 3 implementa a funcionalidade principal do gimi_arm64: a **substituição em tempo de execução de modelos 3D** (Vertex Buffers / Index Buffers) e a **aplicação de correções de shader** (Orfix.ini e Txfix.ini). Ela conecta a camada de interceptação Vulkan (Fase 1) e o banco de dados de regras por hash (Fase 2) ao mecanismo de troca de buffers e patching SPIR-V.

---

## Technical Architecture & Core Mechanisms

### 1. Interceptação de Draw Calls & Mesh Swapping (MESH-01)
- **Funções Interceptadas no Vulkan**:
  - `vkCmdBindVertexBuffers`: Captura os handles dos `VkBuffer` atrelados a cada binding slot.
  - `vkCmdBindIndexBuffer`: Captura o `VkBuffer` do buffer de índices.
  - `vkCmdDrawIndexed`: Momento do render. O hash do buffer de vértices corrente é consultado na `ModDatabase`.
- **Mecanismo de Substituição**:
  Se o hash do buffer ativo bate com uma regra da `ModDatabase`:
  1. Redireciona a chamada `vkCmdBindVertexBuffers` downstream substituindo o handle do `VkBuffer` original pelo handle do `VkBuffer` do modelo modificado.
  2. Ajusta `indexCount`, `firstIndex` e `vertexOffset` em `vkCmdDrawIndexed` para corresponder ao número de vértices/índices da nova geometria.

### 2. Adaptador de Layout de Vértices (DX11 → Vulkan ARM64) (MESH-02)
- **Desafio**: Os mods do 3dmigoto para PC foram concebidos com layouts de entrada DirectX 11 (`D3D11_INPUT_ELEMENT_DESC`). No Android Vulkan, a estrutura equivalente é `VkPipelineVertexInputStateCreateInfo`.
- **Solução (`VertexLayoutAdapter`)**:
  - Remapear `DXGI_FORMAT` para `VkFormat`:
    - `DXGI_FORMAT_R32G32B32_FLOAT` → `VK_FORMAT_R32G32B32_SFLOAT`
    - `DXGI_FORMAT_R16G16_FLOAT` → `VK_FORMAT_R16G16_SFLOAT`
    - `DXGI_FORMAT_R8G8B8A8_UNORM` → `VK_FORMAT_R8G8B8A8_UNORM`
  - Recalcular strides e offsets de bindings quando a geometria customizada possui atributos com alinhamento diferente do modelo original do jogo.

### 3. Shader Fixes Engine: Orfix & Txfix (FIX-01, FIX-02, FIX-03)
- **Propósito do Orfix.ini**: Corrige artefatos de iluminação (decalques de sombra, normais da pele, iluminação de contorno) causados por alterações na malha 3D.
- **Propósito do Txfix.ini**: Corrige coordenadas UV, escala de textura, clipping de transparência (alpha test) e blend modes.
- **Mecanismo de Patching SPIR-V**:
  - Interceptar `vkCreateShaderModule`: calcula o hash do bytecode SPIR-V recebido.
  - Se o hash do shader corresponder a uma regra `[ShaderOverride.*]` no Orfix/Txfix:
    - Substitui o bytecode pelo SPIR-V corrigido pré-compilado, ou modifica constantes de inicialização em tempo de execução via `VkSpecializationInfo` / Push Constants.

---

## Plans Overview

1. **Plan 03-01: Interceptador de Draw Calls & Mesh Swapper**
   - Interceptar `vkCmdBindVertexBuffers`, `vkCmdBindIndexBuffer` e `vkCmdDrawIndexed` em `vulkan_layer.cpp`.
   - Implementar `MeshSwapper` que redireciona buffers e substitui parâmetros de contagem de vértices/índices.

2. **Plan 03-02: Adaptador de Layout de Vértices DX11 → Vulkan**
   - Criar `VertexLayoutAdapter` que converte estruturas `DXGI_FORMAT` e strides de vértices para `VkVertexInputBindingDescription` / `VkVertexInputAttributeDescription`.

3. **Plan 03-03: Shader Fix Engine (Orfix & Txfix SPIR-V Patching)**
   - Interceptar `vkCreateShaderModule` no Vulkan Layer.
   - Implementar `ShaderFixEngine` que aplica as regras do `Orfix.ini` e `Txfix.ini` aos shaders do jogo.
