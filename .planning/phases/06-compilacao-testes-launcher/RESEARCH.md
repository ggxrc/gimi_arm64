# Phase 6 Research: Compilação & Testes de Integração do Launcher App

## Domain Knowledge
- **Android Gradle + CMake Native Integration**:
  - Em projetos Android NDK, o Gradle utiliza a diretiva `externalNativeBuild` apontando para o `CMakeLists.txt` do repositório.
  - Exemplo em `android/build.gradle`:
    ```groovy
    android {
        externalNativeBuild {
            cmake {
                path "../CMakeLists.txt"
                version "3.22.1+"
            }
        }
        defaultConfig {
            ndk {
                abiFilters 'arm64-v8a'
            }
        }
    }
    ```
- **Integration Test Strategy**:
  - Testes C++ executáveis standalone verificando:
    1. Funcionamento do `ModManagerService` no isolamento (criação de pasta virtual `/tmp/GIMI/Mods`, varredura, toggle `.disabled`).
    2. Validação da interface do `ShizukuLayerInjector` (formatação correta dos pacotes do Genshin e comandos `settings put global`).

## Actionable Strategy
- **BUILD-01**: Configurar a estrutura Gradle (`android/build.gradle`, `android/settings.gradle`) vinculando com CMake.
- **TEST-01**: Criar a suíte de testes de integração (`tests/test_launcher_integration.cpp`).
