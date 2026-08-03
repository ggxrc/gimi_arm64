# Phase 6 Context: Compilação & Testes de Integração do Android Launcher App

## Phase Goal
Configurar a infraestrutura de build do APK do Android Launcher (`GIMI-Launcher.apk`), integrar a compilação NDK do C++ (`libgimi_arm64.so`) com o Gradle/Android Studio, e realizar os testes de integração do ecossistema do aplicativo no Android (escaneamento de pastas, ativação via Shizuku e disparo do processo).

## Requirements Addressed
- **BUILD-01**: Integrar compilação NDK CMake no Gradle para empacotamento automático da `libgimi_arm64.so` dentro do APK do Launcher.
- **TEST-01**: Executar suíte de testes de integração do Launcher (varredura de mods, alternância de toggles `.disabled`, comandos Shizuku).

## Key Architectural Decisions

1. **Build System Integration (Gradle + CMake)**:
   - Configurar o `build.gradle.kts` ou `build.gradle` na pasta `android/` vinculando ao `CMakeLists.txt` principal do repositório.
   - O Gradle compila o C++20 nativo para a ABI `arm64-v8a` e deposita a `libgimi_arm64.so` na estrutura do APK.

2. **Test Harness do App Android**:
   - Criar uma rotina/script de validação para simular o ambiente Android, testando o `ModManagerService` e a API do `ShizukuLayerInjector` em lote.
   - Garantir que a geração do APK ocorra de forma automatizada sem erros de build.

## Execution Constraints
- Garantir compatibilidade NDK r26b+ para arquitetura `arm64-v8a`.
