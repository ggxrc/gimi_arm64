# Phase 3: Substituição de Modelos & Shader Fixes - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-08-03
**Phase:** 03-substituicao-de-modelos-shader-fixes
**Areas discussed:** Interceptação de Draw Calls, Mapeamento de Layout de Vértices (DX11 vs Vulkan), Mecanismo de Patching de Shader (Orfix/Txfix)

---

## Interceptação de Render & Mesh Swapping

| Option | Description | Selected |
|--------|-------------|----------|
| Interceptação no nível de API Android (Java) | Ineficiente, latência alta, incompatível com Vulkan nativo | |
| Hooks dinâmicos na `libvulkan.so` via Dobby | Funciona mas viola a especificação Khronos de Vulkan Layers | |
| `DeviceDispatchTable` oficial Khronos | Interceptação limpa e performática na tabela de dispatch do Vulkan Layer | ✓ |

**User's choice:** `DeviceDispatchTable` oficial do Vulkan Layer.
**Notes:** Garante compatibilidade limpa e zero framedrops durante o render loop.

---

## Adaptador de Layout de Vértices (DX11 → Vulkan)

| Option | Description | Selected |
|--------|-------------|----------|
| Conversão estática dos modelos no PC antes do envio | Requer re-converter todos os mods manualmente | |
| Tradução dinâmica de layout (`VertexLayoutAdapter`) | Converte strides e `DXGI_FORMAT` para `VkFormat` em tempo de execução | ✓ |

**Notes:** A tradução dinâmica permite usar mods do 3dmigoto de PC diretamente no Android ARM64 sem necessidade de regravação do asset.

---

## Shader Fixes (Orfix & Txfix)

| Option | Description | Selected |
|--------|-------------|----------|
| Recompilação HLSL → SPIR-V em tempo real no celular | Dependência pesada de compiladores (glslang/shaderc) no Android | |
| Interceptação `vkCreateShaderModule` com substituição SPIR-V pré-compilado | Leve, instantâneo, zero alocação pesada | ✓ |

**Notes:** Shaders corrigidos são pré-compilados e substituídos no momento em que a GPU cria os módulos de shader via `vkCreateShaderModule`.
