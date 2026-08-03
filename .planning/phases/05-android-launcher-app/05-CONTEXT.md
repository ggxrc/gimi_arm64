# Phase 5 Context: Android Launcher App (GUI) & Manager de Injeção de Mods

## Phase Goal
Desenvolver um aplicativo Android nativo (GUI) amigável que permite ao usuário gerenciar a biblioteca de mods do 3dmigoto (na pasta `/sdcard/GIMI/Mods/`), ativar/desativar mods individualmente via interface gráfica, e injetar a Vulkan Layer (`libgimi_arm64.so`) no Genshin Impact (suportando Play Store, Galaxy Store e servidores globais/chineses) de forma transparente e sem necessidade de Root.

## Requirements Addressed
- **LAUNCHER-01**: Criar aplicativo Android GUI para gerenciamento de mods e ativação da Vulkan Layer.
- **LAUNCHER-02**: Suportar injeção sem root via Shizuku / Android GPU Debug Layer settings.
- **LAUNCHER-03**: Auto-detecção e suporte a múltiplos pacotes do jogo (`com.miHoYo.GenshinImpact`, `.samsung`, `com.yuanshen.site`).

## Key Architectural Decisions

1. **Tecnologia do App Launcher**:
   - Desenvolver o Launcher Android nativo em Kotlin/Android SDK leve ou Flutter/Android NDK Wrapper.
   - O aplicativo conterá a biblioteca `libgimi_arm64.so` embutida em sua pasta `lib/arm64-v8a/` para distribuição unificada.

2. **Mecanismo de Injeção Sem Root**:
   - Suporte a **Shizuku API**: Comunicação direta com a API do Shizuku para chamar comandos de permissão de sistema `settings put global enable_gpu_debug_layers 1` e atribuir `gpu_debug_app` ao jogo selecionado.
   - Suporte a **ADB Pair (Wireless Debugging)** integrado no app para ativar o modo de injeção mesmo sem Shizuku.

3. **Gerenciador de Mods**:
   - Varredura de pastas no diretório `/sdcard/GIMI/Mods/`.
   - Leitura automática dos arquivos `.ini` do mod para extrair nomes, imagens de preview (se disponíveis) e toggles ativados.
   - Geração dinâmica do arquivo `gimi_active_mods.ini` contendo apenas as regras dos mods habilitados.

4. **Detecção Dinâmica do Pacote do Jogo**:
   - Seletor de versão do jogo na tela inicial do app (Play Store, Galaxy Store, China, Bilibili).
   - Verificação automática de apps instalados no dispositivo correspondentes a `com.miHoYo.*` e `com.yuanshen.*`.

## Execution Constraints
- O app deve ser não-destrutivo: não modifica o APK do jogo nem altera arquivos originais da miHoYo.
- Compatível com Android 10+ (API Level 29+).
