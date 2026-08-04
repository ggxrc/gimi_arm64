# Phase 9: Pipeline de Compilação Padrão Gradle & Reestruturação da Codebase

## Domain Boundary

Refatorar a estrutura do repositório, a pipeline de compilação e a camada de interface do usuário para alinhar o projeto ao padrão oficial de desenvolvimento Android (Android Studio / Gradle), unificando a UI em Kotlin + Jetpack Compose e eliminando código legado.

## Locked Decisions

### 1. Reorganização de Módulos e Estrutura de Pastas
- **Módulo Android Principal:** Mover `android/app/` para `app/` na raiz do repositório, estabelecendo a estrutura padrão do Gradle (`:app`).
- **Código Nativo C++:** Manter o código C++ nativo nas pastas `src/` e `include/` na raiz do projeto, com o `CMakeLists.txt` na raiz referenciado pelo `app/build.gradle`.

### 2. Migração para Kotlin + Jetpack Compose
- **Interface Nível 100% Kotlin:** Substituir o `MainActivity.java` imperativo por `MainActivity.kt` em Jetpack Compose Material 3.
- **Telas Compose:** Reutilizar e integrar `DashboardScreen.kt`, `ModManagerScreen.kt`, `SettingsScreen.kt` e `NavGraph.kt`.
- **Pipeline de Compilação Kotlin:** Incluir suporte ao compilador Kotlin (`kotlinc` / `kotlin-android` plugin) na pipeline de build.

### 3. Limpeza da Codebase e Remoção de Código Legado
- **Remoção de Arquivos Java:** Deletar `MainActivity.java`, `GimiNativeBridgeJava.java` e `ModInfoJava.java` após a transição para Kotlin (`GimiNativeBridge.kt` e `ModInfo.kt`).
- **Limpeza do Nativo C++:** Simplificar `shizuku_layer_injector.cpp` removendo chamadas `system()` obsoletas e mantendo a interface JNI limpa em `src/launcher/gimi_native_bridge.cpp`.
- **Recursos Android:** Extrair textos e cores hardcoded para `res/values/strings.xml`, `res/values/colors.xml` e temas Compose.

### 4. Pipeline Híbrida Gradle + Termux
- **Compatibilidade Gradle/IDE:** Atualizar `build.gradle` na raiz e em `app/build.gradle` com suporte nativo ao Android Studio.
- **Suporte Termux ARM64:** Manter o fallback/wrapper resiliente para que `gradle assembleDebug` funcione de forma transparente tanto em máquinas x86_64 quanto diretamente no Termux ARM64.

## Canonical References

- [ROADMAP.md](file:///sdcard/github/gimi_arm64/.planning/ROADMAP.md#L18) — Objetivo da Fase 9
- [CMakeLists.txt](file:///sdcard/github/gimi_arm64/CMakeLists.txt) — Configuração de build C++ nativa
- [app/build.gradle](file:///sdcard/github/gimi_arm64/android/app/build.gradle) — Configuração de build do módulo Android
- [build_termux.sh](file:///sdcard/github/gimi_arm64/build_termux.sh) — Script de compilação nativo Termux
