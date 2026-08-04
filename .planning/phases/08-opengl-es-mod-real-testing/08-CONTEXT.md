# Phase 8 Context: Suporte a OpenGL ES & Testes E2E com Mod Real do 3dmigoto

## Phase Goal
Implementar a camada de interceptação **OpenGL ES 3.x (GLES)** via EGL proxying e Dobby inline hooks (`libGLESv3.so`), e realizar a suíte de testes e validação ponta a ponta (E2E) utilizando o mod real do 3dmigoto localizado em `/sdcard/GIMI/Mods/Paimon Redesign by Rain_9`.

## Requirements Addressed
- **GLES-01**: Implementar suporte a interceptação de chamadas OpenGL ES (`glDrawElements`, `glBindBuffer`, `glBufferData`, `glTexImage2D`).
- **MOD-TEST-01**: Executar testes E2E com mod real do 3dmigoto (`Paimon.ini`, `.buf`, `.ib`, `.dds`) para validar a esteira completa de parsing, hashing e substituição.

## Key Architectural Decisions

1. **Estratégia de Interceptação OpenGL ES (GLES 3.x)**:
   - **EGL Proxying + Dobby Inline Hooks**: Interceptar chamadas `eglGetProcAddress` e utilizar o Dobby inline hooker para plugar diretamente nos símbolos do `libGLESv3.so`:
     - `glDrawElements` / `glDrawElementsInstanced` (Interceptação de Draw calls e troca de VBO/IBO).
     - `glBindBuffer` / `glBufferData` / `glBufferSubData` (Cálculo de hashes O(1) de buffers em memória).
     - `glTexImage2D` / `glCompressedTexImage2D` (Upload e override de texturas móveis).
   - Manter paridade total de arquitetura com o backend Vulkan já implementado.

2. **Validação E2E com Mod Real**:
   - Utilizar diretamente a pasta `/sdcard/GIMI/Mods/Paimon Redesign by Rain_9`:
     - Parser `.ini` (`Paimon.ini` com seções `[TextureOverride...]` e `[ShaderOverride...]`).
     - Carregamento de buffers binários (`PaimonPosition.buf`, `PaimonTexcoord.buf`, `PaimonHead.ib`, `PaimonBody.ib`).
     - Processamento de texturas DDS/ASTC (`PaimonHeadDiffuse.dds`, `PaimonHeadLightMap.dds`).
   - Implementar binário de teste `tests/test_e2e_real_mod.cpp` validando a pipeline completa.

## Execution Constraints
- Manter compatibilidade com ARM64 Android NDK C++20.
- Executar testes de integração sem provocar leaks de memória ou travamentos.
