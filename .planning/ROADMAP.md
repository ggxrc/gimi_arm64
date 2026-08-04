# Roadmap: gimi_arm64

## Overview

O gimi_arm64 traz a capacidade de model importing do 3dmigoto para dispositivos Android (ARM64) de forma não-destrutiva. O roteiro é dividido em 4 fases MVP focadas na infraestrutura Vulkan NDK, parser de configs `.ini`, substituição de modelos 3D/shaders e pipeline de texturas mobile.

## Phases

- [x] **Phase 1: Engine de Interceptação Vulkan ARM64 & Infraestrutura Nativa** - Setup NDK C++20 e camada Vulkan Layer não-destrutiva
- [x] **Phase 2: Parser de Configurações 3dmigoto & Gestão de Hashes** - Parser .ini (Orfix/Txfix) e cálculo de hashes em tempo real
- [x] **Phase 3: Substituição de Modelos & Shader Fixes** - Override de Vertex/Index Buffers e aplicação de regras do Orfix/Txfix
- [x] **Phase 4: Pipeline de Texturas & Compatibilidade de Formatos Móveis** - Substituição de texturas em memória para ASTC/ETC2
- [x] **Phase 5: Android Launcher App (GUI) & Manager de Injeção** - Interface Android nativa para gerenciar mods e ativar a Vulkan Layer sem root
- [x] **Phase 6: Compilação & Testes de Integração do Launcher App** - Build Termux/NDK do APK e testes de integração do Launcher
- [x] **Phase 7: Android Launcher GUI (Jetpack Compose & NavGraph)** - Interface Android nativa em Jetpack Compose Material 3 com NavGraph de 3 telas e JNI Native Bridge
- [x] **Phase 8: Suporte a OpenGL ES & Testes E2E com Mod Existente** - Hooking GLES 3.x e validação ponta a ponta com mod real do 3dmigoto
- [x] **Phase 8.6: Compatibilidade Android 16 (API 36) / Min SDK 29, Modern Gradle & Shizuku IPC** - Suporte ao Android 10-16, Gradle 8.2+ para Android Studio e pipeline leve Termux
- [x] **Phase 9: Pipeline de Compilação Padrão Gradle (`gradlew assembleDebug`)** - Configuração e validação completa do build via Gradle nativo

## Phase Details

### Phase 1: Engine de Interceptação Vulkan ARM64 & Infraestrutura Nativa
**Goal:** Estabelecer o ambiente de compilação NDK C++20 e a camada não-destrutiva de interceptação Vulkan no Android (ARM64).
**Mode:** mvp
**Depends on:** Nothing (primeira fase)
**Requirements:** [HOOK-01, HOOK-02, HOOK-03]
**Success Criteria**:
  1. Compilação NDK C++20 gera biblioteca nativa limpa para arquitetura `arm64-v8a`.
  2. Vulkan Layer intercepta entrypoints `vkGetInstanceProcAddr` e `vkGetDeviceProcAddr` sem provocar crashes.
  3. Interceptação ocorre estritamente em memória sem modificar APK ou arquivos no disco.
**Plans:** Complete

Plans:
- [x] 01-01: Setup do projeto NDK, CMake e estrutura base C++20
- [x] 01-02: Implementação da camada de interceptação Vulkan Layer / EGL Dispatch Table

### Phase 2: Parser de Configurações 3dmigoto & Gestão de Hashes
**Goal:** Implementar o parser de arquivos `.ini` do 3dmigoto (Orfix, Txfix e Overrides) e o sistema de hashing de recursos em tempo real.
**Mode:** mvp
**Depends on:** Phase 1
**Requirements:** [CFG-01, CFG-02, CFG-03]
**Success Criteria**:
  1. Parser C++ interpreta seções `[TextureOverride]`, `[ShaderOverride]`, `Orfix.ini` e `Txfix.ini`.
  2. Sistema calcula e armazena hashes de Vertex Buffers, Index Buffers e Texturas durante o render sem travar o jogo.
**Plans:** Complete

Plans:
- [x] 02-01: Parser de configurações `.ini` C++ e estruturas de dados de mod
- [x] 02-02: Engine de cálculo e cache de hashes de recursos gráficos em tempo real

### Phase 3: Substituição de Modelos & Shader Fixes
**Goal:** Permitir a substituição de modelos 3D originais por meshes customizados e aplicação de correções de shader.
**Mode:** mvp
**Depends on:** Phase 2
**Requirements:** [MESH-01, MESH-02, FIX-01, FIX-02, FIX-03]
**Success Criteria**:
  1. Interceptação de `vkCmdDrawIndexed` substitui Vertex Buffers originais por buffers de modelos modificados.
  2. Remapeamento de stride/atributos entre layouts de DX11 e Vulkan ocorre sem corrupção gráfica.
  3. Regras do `Orfix.ini` e `Txfix.ini` são aplicadas aos shaders/buffers corrigindo sombras e iluminação.
**Plans:** Complete

Plans:
- [x] 03-01: Interceptação de Draw Calls (`vkCmdDrawIndexed`) e troca de Vertex/Index Buffers
- [x] 03-02: Adaptador de layout de atributos de vértices (DX11 para Vulkan)
- [x] 03-03: Executor de correções de shader e iluminação (Orfix/Txfix)

### Phase 4: Pipeline de Texturas & Compatibilidade de Formatos Móveis
**Goal:** Interceptação e substituição de texturas modificadas em formatos móveis (ASTC/ETC2).
**Mode:** mvp
**Depends on:** Phase 3
**Requirements:** [TEX-01, TEX-02, TEX-03]
**Success Criteria**:
  1. Interceptação de uploads e descritores de textura no Vulkan.
  2. Carregamento e substituição transparente de texturas modded em formatos móveis (ASTC/ETC2/RGBA8) ao detectar correspondência de hashes.
**Plans:** Complete

Plans:
- [x] 04-01: Interceptação de Descriptor Sets e Uploads de Textura Vulkan
- [x] 04-02: Carregador e substituidor de texturas nos formatos ASTC/ETC2/RGBA8

### Phase 5: Android Launcher App (GUI) & Manager de Injeção
**Goal:** Criar o aplicativo Android nativo (GUI) para gerenciamento visual de mods e injeção transparente da Vulkan Layer sem root.
**Mode:** mvp
**Depends on:** Phase 4
**Requirements:** [LAUNCHER-01, LAUNCHER-02, LAUNCHER-03]
**Success Criteria**:
  1. App Android exibe interface amigável listando mods da pasta `/sdcard/GIMI/Mods/` com toggles de ativado/desativado.
  2. Suporte a injeção da `libgimi_arm64.so` sem root via Shizuku / Wireless Debugging.
  3. Auto-detecção de todas as distribuições do jogo (Play Store, Galaxy Store, China).
**Plans:** Complete

Plans:
- [x] 05-01: Interface Android (GUI) e Gerenciador de Arquivos de Mods
- [x] 05-02: Módulo de Injeção de Vulkan Layer sem Root (Shizuku API & ADB Manager)

### Phase 6: Compilação & Testes de Integração do Launcher App
**Goal:** Configurar o build do APK Android no Termux (AAPT2 + D8 + CMake) e testar a integração do Launcher com o motor nativo.
**Mode:** mvp
**Depends on:** Phase 5
**Requirements:** [BUILD-01, TEST-01]
**Success Criteria**:
  1. Script `build_termux.sh` compila C++20 nativo em `libgimi_arm64.so`, compila Java/Kotlin em `classes.dex`, alinha e assina o APK `GIMI-Launcher.apk`.
  2. Suíte de testes valida escaneamento de mods e chamadas do Shizuku com `apksigner verify` limpo.
**Plans:** Complete

Plans:
- [x] 06-01: Script de Build Leve no Termux (AAPT2 + D8 + CMake + OpenJDK-21 + Keystore)
- [x] 06-02: Testes de Integração e Validação do APK Termux

### Phase 7: Android Launcher GUI (Jetpack Compose & NavGraph)
**Goal:** Desenvolver a interface gráfica do Launcher Android em Jetpack Compose (Material 3) com NavGraph de 3 telas (Dashboard, Mod Manager, Settings) e JNI Native Bridge para interagir com a `libgimi_arm64.so`.
**Mode:** mvp
**Depends on:** Phase 6
**Requirements:** [COMPOSE-01, NAV-01, JNI-01]
**Success Criteria**:
  1. App compilado em APK instalável com 3 telas navegáveis via NavGraph (Dashboard, Mod Manager, Settings).
  2. Lista visual de mods com cards, busca, toggles ativado/desativado integrados via JNI Native Bridge (`GimiNativeBridge`).
  3. Painel de controle de status da Vulkan Layer e ativação via Shizuku aguardando interação do usuário.
**Plans:** Complete

Plans:
- [x] 07-01: JNI Native Bridge e Estrutura Navigation Compose (NavGraph)
- [x] 07-02: Interface Visual Compose (Dashboard, Mod Manager, Settings) & APK Integration

### Phase 8: Suporte a OpenGL ES & Testes E2E com Mod Existente
**Goal:** Implementar o suporte a OpenGL ES (GLES 3.x) e realizar o teste E2E completo com um mod real do 3dmigoto.
**Mode:** mvp
**Depends on:** Phase 7
**Requirements:** [GLES-01, MOD-TEST-01]
**Success Criteria**:
  1. Camada de interceptação OpenGL ES (GLES/EGL) intercepta draw calls e texturas.
  2. Mod real do 3dmigoto (arquivos `.ini`, `.vb`, `.ib` e texturas) é parsed, hash e substituído no pipeline.
**Plans:** Complete

Plans:
- [x] 08-01: Suporte a Interceptação de Renderização OpenGL ES (GLES 3.x)
- [x] 08-02: Suíte de Testes E2E com pacote de mod existente do 3dmigoto

## Progress

**Execution Order:**
Phases execute in numeric order: 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Engine de Interceptação Vulkan ARM64 | 2/2 | Complete | 2026-08-03 |
| 2. Parser 3dmigoto & Hashes | 2/2 | Complete | 2026-08-03 |
| 3. Substituição de Modelos & Shader Fixes | 3/3 | Complete | 2026-08-03 |
| 4. Pipeline de Texturas Mobile | 2/2 | Complete | 2026-08-03 |
| 5. Android Launcher App (GUI) | 2/2 | Complete | 2026-08-03 |
| 6. Compilação & Testes do Launcher | 2/2 | Complete | 2026-08-03 |
| 7. Android Launcher GUI (Jetpack Compose) | 2/2 | Complete | 2026-08-03 |
| 8. Suporte OpenGL ES & Teste com Mod Real | 2/2 | Complete | 2026-08-03 |
| 8.5. Correções de Bugs (Storage/Injeção) | 1/1 | Complete | 2026-08-03 |
