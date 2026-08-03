# Architecture Patterns: gimi_arm64

**Domain:** Vulkan Layer Graphics Interception & Resource Override Engine
**Researched:** 2026-08-03

## Recommended Architecture

O gimi_arm64 é estruturado como uma biblioteca nativa em C++ (Vulkan Layer / Graphics Hook) dividida em 4 subsistemas principais:

```
+-----------------------------------------------------------------------------------+
|                                  Genshin Impact                                   |
+-----------------------------------------------------------------------------------+
                                          |
                                 Vulkan / GLES Calls
                                          v
+-----------------------------------------------------------------------------------+
|                            1. Interception Layer                                  |
|   (vkCmdDrawIndexed, vkCmdBindVertexBuffers, vkCmdBindDescriptorSets, etc.)       |
+-----------------------------------------------------------------------------------+
                                          |
                                    Resource Hash
                                          v
+-----------------------------------------------------------------------------------+
|                             2. 3dmigoto Config Engine                             |
|       (Parses .ini, maintains Hashes -> Mod Overrides, Orfix/Txfix rules)         |
+-----------------------------------------------------------------------------------+
                                          |
                                  Matching Rule Found
                                          v
+-----------------------------------------------------------------------------------+
|                        3. Resource Replacement Engine                             |
|          (Injects custom Vertex/Index Buffers, ASTC Textures, Shaders)            |
+-----------------------------------------------------------------------------------+
                                          |
                                 Substituted Render Call
                                          v
+-----------------------------------------------------------------------------------+
|                                  GPU Hardware                                     |
+-----------------------------------------------------------------------------------+
```

### Component Boundaries

| Component | Responsibility | Communicates With |
|-----------|---------------|-------------------|
| **Interception Layer** | Intercepta entrypoints do Vulkan (`vkGetInstanceProcAddr`, `vkGetDeviceProcAddr`, `vkCmdDrawIndexed`) | GPU Driver & Config Engine |
| **Config Engine** | Carrega e interpreta arquivos `.ini` do 3dmigoto (Orfix.ini, Txfix.ini, Overrides) | Interception Layer & Resource Replacer |
| **Resource Replacer** | Mapeia buffers e texturas originais para os recursos modificados em memória GPU | Interception Layer & Memory Allocator |
| **Format Transpiler** | Converte layouts de vértices de DX11 para Vulkan e texturas DDS para ASTC | Resource Replacer |

## Patterns to Follow

### Pattern 1: Vulkan Layer Entrypoint Dispatching
**What:** Implementar a especificação oficial de Vulkan Layers (`vkGetInstanceProcAddr` / `vkGetDeviceProcAddr`) enviando chamadas não substituídas diretamente para a tabela de dispatch do driver original.
**Why:** Garante compatibilidade total e estabilidade sem quebrar o pipeline de renderização nativo.

### Pattern 2: Cache de Hashes de Recursos
**What:** Armazenar em cache o hash de Vertex Buffers e Descriptor Sets por frame.
**Why:** Evita recalcular o hash de buffers de vértices a cada chamada de desenho, mantendo alta taxa de quadros (FPS) no mobile.

## Anti-Patterns to Avoid

### Anti-Pattern 1: Leitura síncrona de arquivos do disco durante o draw call
**Why bad:** Provoca travamentos (stuttering) e quedas graves de FPS durante o jogo.
**Instead:** Pré-carregar todas as texturas e buffers modificados para a memória VRAM/RAM durante a inicialização ou em background.
