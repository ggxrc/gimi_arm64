# Phase 7 Context: Android Launcher GUI (Jetpack Compose & NavGraph)

## Phase Goal
Desenvolver o aplicativo Android nativo (GUI) do Launcher em **Kotlin** utilizando **Jetpack Compose (Material 3)**, estruturado com **Navigation Compose (NavGraph)** com 3 telas principais (Dashboard, Mod Manager e Settings) e integração JNI via `GimiNativeBridge` para interagir com o motor nativo `libgimi_arm64.so`.

## Requirements Addressed
- **COMPOSE-01**: Desenvolver a interface visual em Jetpack Compose Material 3 com suporte a temas modernos, cards de mods, filtros e alternância de estado (ativado/desativado).
- **NAV-01**: Estruturar o fluxo de navegação do app usando Navigation Compose (`NavHost`) com 3 destinos: Dashboard, Mod Manager e Settings.
- **JNI-01**: Implementar a classe `GimiNativeBridge` (`com.gimi.launcher.native.GimiNativeBridge`) para ponte JNI direta entre o aplicativo Kotlin e a biblioteca C++ `libgimi_arm64.so`.

## Key Architectural Decisions

1. **Arquitetura Kotlin & Jetpack Compose (MVVM)**:
   - **Padrão**: MVVM (Model-View-ViewModel) com Jetpack Compose Material 3.
   - **Gerenciamento de Estado**: `StateFlow` e ViewModels (`DashboardViewModel`, `ModManagerViewModel`, `SettingsViewModel`).

2. **Navegação (NavGraph - 3 Telas)**:
   - **Dashboard Screen**: Status da Vulkan Layer, status do serviço Shizuku, botão de injeção e seletor da variante do jogo (PlayStore, GalaxyStore, ChinaServer, Bilibili).
   - **Mod Manager Screen**: Lista visual de mods encontrados na pasta `/sdcard/GIMI/Mods/`, cartões informativos, campo de busca e toggles de habilitação/desabilitação (renomeação de sufixo `.disabled`).
   - **Settings Screen**: Seleção do diretório customizado de mods, alternância de tema e registros de logs do sistema.

3. **JNI Native Bridge (`GimiNativeBridge`)**:
   - Pacote: `com.gimi.launcher.native.GimiNativeBridge`
   - Métodos nativos expostos:
     - `nativeScanMods(String path): Array<ModInfo>`
     - `nativeToggleMod(String modPath, boolean enable): boolean`
     - `nativeInjectLayer(String packageName): int`
     - `nativeGetLayerStatus(): int`

4. **Entregável & Condição de Sucesso**:
   - APK do Launcher (`GIMI-Launcher.apk`) compilável, instalável e funcional em dispositivos Android ARM64, com a interface em Compose operante aguardando a interação do usuário.

## Execution Constraints
- Execução no Android ARM64.
- Uso de Material Design 3 e Jetpack Compose em Kotlin.
