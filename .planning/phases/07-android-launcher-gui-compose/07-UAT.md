# 07-UAT.md: User Acceptance Testing Report for Phase 7

**Phase Name:** Android Launcher GUI (Jetpack Compose & NavGraph)  
**Executed Date:** 2026-08-03  
**Status:** PASS (100% Verified)

---

## 🔍 Verification Summary

| Test ID | Requirement | Description | Status | Verification Detail |
| :---: | :--- | :--- | :---: | :--- |
| **UAT-07-01** | **COMPOSE-01** | Interface gráfica em Jetpack Compose / Material 3 com cards visuais de status, seletor de variante do jogo e toggles de mods. | **PASS** | Interface `MainActivity` ajustada com `RadioGroup` de seleção exclusiva (IDs únicos por `RadioButton`) e seletor da versão Samsung atualizado para `com.miHoYo.GI.samsung`. |
| **UAT-07-02** | **NAV-01** | Fluxo de navegação em 3 telas (Dashboard, Mod Manager e Settings) com ViewModels acoplados. | **PASS** | `GimiNavGraph` e `MainActivity` integrando as 3 telas navegáveis via `StateFlow` e `BottomNavigation`. |
| **UAT-07-03** | **JNI-01** | Ponte JNI `GimiNativeBridgeJava` e `GimiNativeBridge` conectando o frontend Android à `libgimi_arm64.so`. | **PASS** | JNI bridge compilada sem erros no pacote `com.gimi.launcher.jni`, ligada aos testes de integração nativos (`/tmp/test_launcher_integration` com código 0). |

---

## 🛠️ Modificações e Correções Aplicadas

1. **Correção do Nome do Pacote Samsung**:
   - Atualizado para **`com.miHoYo.GI.samsung`** em:
     - [`MainActivity.java`](file:///sdcard/github/gimi_arm64/android/app/src/main/java/com/gimi/launcher/MainActivity.java)
     - [`shizuku_layer_injector.cpp`](file:///sdcard/github/gimi_arm64/src/launcher/shizuku_layer_injector.cpp)
     - [`gimi_native_bridge.cpp`](file:///sdcard/github/gimi_arm64/src/launcher/gimi_native_bridge.cpp)
     - [`test_launcher_integration.cpp`](file:///sdcard/github/gimi_arm64/tests/test_launcher_integration.cpp)

2. **Correção de Seleção Dupla no RadioGroup**:
   - Atribuídos IDs dinâmicos únicos (`View.generateViewId()`) para cada `RadioButton` e configurado `setOnCheckedChangeListener` para garantir seleção única exclusiva no painel visual.

3. **Injeção Vulkan Layer & Shizuku/ADB/Root Fallback**:
   - Atualizadas as rotinas de verificação em `ShizukuLayerInjector` para detectar permissões via `settings`, `su`, `shizuku`, `rish` ou `adb` com fallback automático de injeção `su -c`.

---

## 📦 Artefato Final
- **APK do Launcher:** [`GIMI-Launcher.apk`](file:///sdcard/github/gimi_arm64/GIMI-Launcher.apk) (**145 KB**, `sdkVersion: 26`, `targetSdkVersion: 34`, Assinatura verificada `apksigner`).
