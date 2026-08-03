# Phase 1: Engine de Interceptação Vulkan ARM64 & Infraestrutura Nativa - Research

**Researched:** 2026-08-03
**Phase:** 01-engine-de-intercepta-o-vulkan-arm64-infraestrutura-nativa

## Executive Summary

A Fase 1 estabelece a base nativa C++20 e a camada de interceptação não-destrutiva de renderização gráfica para Android ARM64 (`arm64-v8a`). A solução utiliza a especificação oficial de Vulkan Layers combinada com o Dobby Hooking Library para hooking dinâmico de EGL (`eglGetProcAddress`), permitindo suporte transparente a jogos que utilizam Vulkan ou OpenGL ES.

---

## Key Technical Decisions & Patterns

### 1. Dual-API Interception Architecture (Vulkan + OpenGL ES)
- **Vulkan Entrypoint Hooking**: Implementa os entrypoints padrão da especificação Vulkan Layer API (`vkGetInstanceProcAddr`, `vkGetDeviceProcAddr`, `vkNegotiateLoaderLayerInterfaceVersion`).
- **EGL / OpenGL ES Hooking**: Utiliza Dobby para interceptar `eglGetProcAddress` na `libEGL.so` quando o jogo opera em modo OpenGL ES.
- **Dynamic Dispatcher**: Detecta em tempo de execução qual API gráfica foi inicializada pelo jogo e redireciona os hooks para a dispatch table correspondente.

### 2. Native Toolchain & Build System
- **Android NDK**: Version r26b+ com padrão C++20 (`-std=c++20`).
- **Target Architecture**: Exclusivamente `arm64-v8a`.
- **CMake Target**: `libgimi_arm64.so` compilado como biblioteca compartilhada (`SHARED`).
- **Logging**: Integrado ao `<android/log.h>` com tag `gimi_arm64` e macros `LOGD`, `LOGI`, `LOGW`, `LOGE`.

### 3. Non-Destructive In-Memory Hooking
- Não realiza modificações no APK, nem altera arquivos ELF em disco.
- Todo o remapeamento de ponteiros de funções é efetuado estritamente na memória RAM/VRAM durante a inicialização do processo.

---

## Implementation Slices & Plans

1. **Plan 01-01: Setup NDK, CMake e Estrutura C++20 Nativa**
   - Configurar `CMakeLists.txt` (NDK r26b+, C++20, `arm64-v8a`, flags `-O3 -fvisibility=hidden`).
   - Criar sistema de logging nativo (`log.h` / `log.cpp` encapsulando `android/log.h`).
   - Estrutura base de inicialização e ciclo de vida da biblioteca (`JNI_OnLoad` ou `__attribute__((constructor))`).

2. **Plan 01-02: Engine de Dispatch & Layer Hooking Vulkan/EGL**
   - Implementar tabela de dispatch Vulkan Layer (`vulkan_layer.h` / `vulkan_layer.cpp`).
   - Implementar hooker de EGL via Dobby (`egl_hook.h` / `egl_hook.cpp`).
   - Implementar o dispatcher de runtime (`graphics_dispatcher.h` / `graphics_dispatcher.cpp`) que detecta a API ativa e encaminha as chamadas sem causar overhead ou crashes.

---

## Verification Criteria & Risks

- **Verificação de Compilação**: `libgimi_arm64.so` deve compilar sem warnings/erros direcionado a `arm64-v8a`.
- **Verificação de Simbolos**: `readelf -d` / `nm -D` deve expor os símbolos exportados de Vulkan Layer.
- **Mitigação de Riscos**: Evitar chamadas blocantes ou alocações excessivas dentro das funções interceptadas para não introduzir framedrops.
