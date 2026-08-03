# Phase 6 Context: Compilação de APK e Testes em Ambiente Linux / Termux (ARM64)

## Phase Goal
Garantir a compatibilidade total do projeto para compilar e empacotar o APK do Launcher (`GIMI-Launcher.apk`) diretamente no ambiente Linux do Termux (Android ARM64), utilizando um script de build leve sem dependência do Gradle Desktop ou Android Studio.

## Requirements Addressed
- **BUILD-01**: Criar pipeline de build leve via script Termux (`build_termux.sh`) que compila C++20 nativo (`libgimi_arm64.so`), compila recursos (`aapt2`), compila código Java/Kotlin (`d8` + `openjdk-21`), e assina o APK (`apksigner`).
- **TEST-01**: Executar suíte de testes de integração e validação do APK gerado no ambiente Termux (verificação de assinatura, layout interno de arquivos `.so` e `.dex`, e execução do serviço de gerenciamento de mods).

## Key Architectural Decisions

1. **Pipeline de Build Leve no Termux (`build_termux.sh`)**:
   - Invocação direta de `cmake` e `clang` nativo no Termux para compilar C++20 para `arm64-v8a` (`libgimi_arm64.so`).
   - Processamento de recursos visuais com `aapt2 compile` e `aapt2 link`.
   - Compilação de código Java/Kotlin com `javac` / `ecj` (`openjdk-21`) e conversão para bytecode Dalvik com `d8`.
   - Empacotamento de `classes.dex`, `lib/arm64-v8a/libgimi_arm64.so` e `resources.ap_` no APK.
   - Alinhamento (`zipalign`) e assinatura (`apksigner`) gerando o `GIMI-Launcher.apk`.

2. **Gerenciamento de Pré-requisitos & Ferramentas**:
   - `build_termux.sh` auto-detecta ferramentas instaladas no Termux (`pkg install openjdk-21 cmake clang aapt zipalign apksigner ecj`).
   - Download/cache automático de `android.jar` minimalista (API 34/33) caso não esteja presente no ambiente local.

3. **Keystore Determinístico de Assinatura**:
   - Utilizar um `debug.keystore` empacotado na estrutura do repositório (`android/debug.keystore`) para garantir builds assinados de forma determinística sem exigir input do usuário.

4. **Suíte de Testes de Integração em Ambiente Linux/Termux**:
   - Teste de sanidade que inspeciona o APK final com `unzip -l` ou `aapt dump` para confirmar presença de `lib/arm64-v8a/libgimi_arm64.so` e `classes.dex`.
   - Validação da execução dos binários nativos e scripts de injeção em ambiente Linux local.

## Execution Constraints
- Execução nativa no Android ARM64 via Termux / Linux environment.
- Suporte estrito a Java 21 (`openjdk-21`).
