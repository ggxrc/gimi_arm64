# Phase 5 Research: Android Launcher App (GUI) & Manager de Injeção

## Domain Knowledge & Shizuku Layer Manager
- **Android Vulkan Layer Injection (Non-Root)**:
  - Android possui a API oficial `Global.Settings` para GPU debugging.
  - Com a permissão do Shizuku (que roda com privilégios de `shell`/ADB), o app pode executar:
    - `settings put global enable_gpu_debug_layers 1`
    - `settings put global gpu_debug_app <target_package>`
    - `settings put global gpu_debug_layer_app <launcher_package_name>`
    - `settings put global gpu_debug_layers libgimi_arm64.so`
  - Quando a biblioteca nativa `libgimi_arm64.so` está empacotada no APK do Launcher em `lib/arm64-v8a/libgimi_arm64.so`, o Android extrai a `.so` para `/data/data/<launcher_package>/lib/libgimi_arm64.so`.

- **Mod Directory Structure**:
  - O app escaneia `/sdcard/GIMI/Mods/` (ou `Android/data/` se necessário com Scoped Storage permissions).
  - Cada pasta de mod contém arquivos `.ini` (com seções `[TextureOverride...]` ou `[ShaderOverride...]`).
  - O app permite habilitar/desabilitar cada pasta adicionando/removendo um sufixo (ex: `GanyuMod/` vs `GanyuMod.disabled/`) ou gerando um `active_mods.ini` mestre.

## Actionable Strategy
- **LAUNCHER-01**: Desenvolver a interface Android (GUI em Kotlin/Android Studio ou Android Wrapper) para visualização de mods e controle.
- **LAUNCHER-02**: Conectar com o Shizuku Binder/API para injetar a camada Vulkan sem necessidade de computador/root.
- **LAUNCHER-03**: Auto-detectar pacotes instalados (`com.miHoYo.GenshinImpact`, `com.miHoYo.GenshinImpact.samsung`, `com.yuanshen.site`).
