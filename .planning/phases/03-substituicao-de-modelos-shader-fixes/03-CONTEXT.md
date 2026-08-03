# Phase 3: Substituição de Modelos & Shader Fixes - Context

**Gathered:** 2026-08-03
**Status:** Ready for planning

<domain>
## Phase Boundary

A Fase 3 implementa o mecanismo central de **substituição de geometria 3D** (Vertex/Index Buffers) e **aplicação de correções de iluminação/shader (Orfix.ini e Txfix.ini)** durante as chamadas de renderização Vulkan no Android ARM64.

Esta fase utiliza a infraestrutura de interceptação da Fase 1 e o banco de dados de regras por hash da Fase 2 para realizar a injeção não-destrutiva dos modelos modded na GPU.

</domain>

<decisions>
## Implementation Decisions

### Interceptação de Render & Mesh Swapping
- **D-01:** Interceptar `vkCmdBindVertexBuffers`, `vkCmdBindIndexBuffer` e `vkCmdDrawIndexed` na `DeviceDispatchTable` do Vulkan Layer. — **Reversibility:** costly — afeta o fluxo de render de todas as draw calls.
- **D-02:** Manter um estado por command buffer (`CommandBufferState`) rastreando os buffers de vértices/índices ativos e seus hashes correspondentes para lookup O(1) no momento do draw call.
- **D-03:** Se o hash do buffer ativo corresponder a uma regra na `ModDatabase`, o `MeshSwapper` substitui o handle do `VkBuffer` e reajusta `indexCount`, `firstIndex` e `vertexOffset`.

### Adaptador de Layout de Vértices
- **D-04:** `VertexLayoutAdapter` realiza a tradução de tipos entre o formato de input do 3dmigoto (origem DX11) e as estruturas `VkVertexInputAttributeDescription` do Vulkan.
- **D-05:** Tabela de mapeamento determinística entre `DXGI_FORMAT` e `VkFormat` (ex: `DXGI_FORMAT_R32G32B32_FLOAT` → `VK_FORMAT_R32G32B32_SFLOAT`).

### Execução de Shader Fixes (Orfix & Txfix)
- **D-06:** Interceptar `vkCreateShaderModule` para calcular o hash do bytecode SPIR-V recebido e consultar a `ModDatabase`.
- **D-07:** `ShaderFixEngine` aplica correções de iluminação (Orfix) e transparência (Txfix) substituindo ou modicando o SPIR-V antes da criação do módulo Vulkan.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project & System Architecture
- `.planning/PROJECT.md` — Visão geral e premissas anti-cheat
- `.planning/REQUIREMENTS.md` — Requisitos MESH-01, MESH-02, FIX-01, FIX-02, FIX-03
- `.planning/research/ARCHITECTURE.md` — Diagrama de componentes (Layer 3: Resource Replacement Engine)
- `include/graphics/vulkan_layer.h` — DeviceDispatchTable onde os hooks de draw call são adicionados
- `include/config/mod_types.h` — Estrutura `ModDatabase` com regras de substituição por hash
- `include/hash/resource_hash_engine.h` — `ResourceHashEngine` para identificação de buffers

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `ModDatabase` (Phase 2): provê `texture_overrides`, `shader_overrides` e `resources` indexados por hash.
- `ResourceHashEngine` & `HashRegistry` (Phase 2): provê hashes estáveis de `VkBuffer` / `VkImage`.
- `vulkan_layer.cpp` (Phase 1): expõe os interceptores Vulkan onde `vkCmdDrawIndexed` e `vkCreateShaderModule` serão ligados.

</code_context>

<specifics>
## Specific Implementation Notes

- A troca de buffers deve ser **estritamente em memória RAM/VRAM** — sem ler arquivos do disco durante o `vkCmdDrawIndexed`. Os buffers customizados são carregados e alocados no VRAM na inicialização ou em background.
- Para shaders, a verificação de hash do SPIR-V ocorre apenas na compilação do pipeline (`vkCreateShaderModule`), não introduzindo overhead durante a renderização de cada frame.

</specifics>

<deferred>
## Deferred Ideas

- Suporte a animações esqueléticas customizadas (rigging) — Fase posterior
- Shader reflection com SPIRV-Cross em tempo real no dispositivo — Fase posterior / v2

</deferred>

---

*Phase: 3-Substituição de Modelos & Shader Fixes*
*Context gathered: 2026-08-03*
