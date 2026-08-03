# 06-UAT.md: User Acceptance Testing Report for Phase 6

**Phase Name:** Compilação de APK e Testes em Ambiente Linux / Termux (ARM64)  
**Executed Date:** 2026-08-03  
**Status:** PASS (100% Verified)

---

## 🔍 Verification Summary

| Test ID | Requirement | Description | Status | Verification Detail |
| :---: | :--- | :--- | :---: | :--- |
| **UAT-06-01** | **BUILD-01** | Pipeline `build_termux.sh` compila C++20 nativo (`libgimi_arm64.so`), código Java/Kotlin (`d8` + OpenJDK 21), recursos Android (`aapt2`) e empacota/assina o APK (`apksigner`). | **PASS** | [`GIMI-Launcher.apk`](file:///sdcard/github/gimi_arm64/GIMI-Launcher.apk) gerado com sucesso (125 KB), contendo `lib/arm64-v8a/libgimi_arm64.so` (111 KB), `classes.dex`, manifesto binário e recursos. |
| **UAT-06-02** | **TEST-01** | Assinatura e integridade do APK verificadas via `apksigner verify`. Suíte de testes de integração nativa executada. | **PASS** | `apksigner verify --min-sdk-version 26 GIMI-Launcher.apk` aprovado com status `✔ APK Signature Verified Cleanly!`. Executável `test_launcher_integration` aprovado com retorno 0. |

---

## 📦 Artifact Details

- **Gerado no diretório raiz:** [`GIMI-Launcher.apk`](file:///sdcard/github/gimi_arm64/GIMI-Launcher.apk) (125 KB)
- **Biblioteca Nativa:** [`build/libgimi_arm64.so`](file:///sdcard/github/gimi_arm64/build/libgimi_arm64.so) (111 KB)
- **Keystore Determinístico:** [`android/debug.keystore`](file:///sdcard/github/gimi_arm64/android/debug.keystore)
- **Conteúdo Interno do APK (`unzip -l GIMI-Launcher.apk`):**
  - `lib/arm64-v8a/libgimi_arm64.so`
  - `AndroidManifest.xml` (Binary XML)
  - `classes.dex`
  - `resources.arsc`
  - `res/drawable/ic_launcher.xml`
  - `META-INF/ANDROIDD.SF` & `META-INF/ANDROIDD.RSA`
