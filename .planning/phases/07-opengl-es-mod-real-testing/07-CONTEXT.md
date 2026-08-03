# Phase 7 Context: Suporte a OpenGL ES & Testes E2E com Mod Existente do 3dmigoto

## Phase Goal
Implementar o suporte à API de renderização **OpenGL ES (GLES 3.x)** através de hooks EGL/GLES (usando Dobby/Substrate) para complementar a camada Vulkan, e realizar a validação ponta a ponta (E2E) com um pacote de mod real do 3dmigoto (arquivos `.ini`, `.vb`, `.ib` e texturas).

## Requirements Addressed
- **GLES-01**: Implementar suporte a interceptação de chamadas OpenGL ES (`glDrawElements`, `glBindBuffer`, `glTexImage2D`).
- **MOD-TEST-01**: Executar testes E2E com um mod existente do 3dmigoto para validar parsing, hashing e substituição real.

## Key Architectural Decisions

1. **Camada de Interceptação OpenGL ES (GLES/EGL)**:
   - Implementar `src/graphics/gles_hook.cpp` para interceptar `eglGetProcAddress` e os símbolos `glDrawElements`, `glBindBuffer`, `glBufferData`.
   - Mapear a mesma lógica de hashing O(1) e `MeshSwapper` / `TextureSwapper` para que mods do 3dmigoto funcionem tanto em Vulkan quanto em OpenGL ES.

2. **Suíte de Testes com Mod Real do 3dmigoto**:
   - Montar um exemplo de mod real completo em `.planning/test_assets/SampleMod/`:
     - `Sample.ini` com seções `[TextureOverride...]` e `[ShaderOverride...]`.
     - Buffers binários de vértice (`.vb`) e índice (`.ib`).
     - Texturas em ASTC/PNG.
   - Executar o pipeline completo de verificação (Parsing → Hasher → Swapper → Pipeline de Injeção).

## Execution Constraints
- Garantir paridade entre os sistemas de substituição Vulkan e OpenGL ES.
