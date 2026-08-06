# gimi_arm64

**3dmigoto Model Importer for Android ARM64 — Non-destructive Vulkan & OpenGL ES Graphics Hook**

`gimi_arm64` is a mobile port of the Genshin Impact Model Importer (GIMI, based on 3dmigoto) for the ARM64 architecture, specifically targeting Android. It allows 3D model overrides, texture replacements, and shader corrections (such as `Orfix.ini` and `Txfix.ini` rules) directly inside the game's graphics pipeline. 

All of this is done **non-destructively**: there is no APK modification, no root access required, and no changes are made to on-disk game files, ensuring anti-cheat safety.

---

## How It Works (Technical Mechanism)

The graphics hooking mechanism is transparent and operates entirely in memory (RAM).

```
Genshin Impact (Android ARM64 Process)
         │
         ▼  Vulkan / GLES API Calls
┌────────────────────────────────────────┐
│ libgimi_arm64.so                       │
│ (Vulkan Layer / EGL Dispatch Table)    │
│ ────────────────────────────────────── │
│  - Vulkan Layer intercepting draws     │  ← Intercepts vkCmdDrawIndexed
│  - EGL Hook (Dobby redirection)        │  ← Redirects OpenGL ES entry points
└────────────────────────────────────────┘
         │
         ▼  Hash Match & Override
┌────────────────────────────────────────┐
│ ModManagerService & MeshSwapper        │
│  - Parsed 3dmigoto (.ini) mods         │  ← Reads from /sdcard/GIMI/Mods/
│  - Vertex/Index Buffer replacements    │  ← Remaps stride & attributes
└────────────────────────────────────────┘
         │
         ▼
    GPU Hardware
```

### 1. Vulkan Layer Injection
Under Vulkan, the application is injected using the system loader's implicit layer mechanism. The Android system enables debug layers if configured in `Settings.Global`. The launcher coordinates with Android via secure settings to specify:
- `enable_gpu_debug_layers = 1`
- `gpu_debug_app = <game_package>`
- `gpu_debug_layer_app = com.gimi.launcher`
- `gpu_debug_layers = VK_LAYER_GIMI_ARM64`

This causes the Android Vulkan loader to automatically chain our layer library (`libgimi_arm64.so`) inside the game's Vulkan instance, allowing us to intercept draw calls (e.g., `vkCmdDrawIndexed` and `vkCmdBindVertexBuffers`) and swap resources when their xxHash64 digests match a mod.

### 2. OpenGL ES / EGL Redirection
For GLES interception, `libgimi_arm64.so` hooks `eglGetProcAddress` using the Dobby library. This dynamically intercepts the return pointers for OpenGL drawing functions like `glDrawElements` and buffer updates, swapping buffer bindings in memory when target resources are matched.

---

## Features

- **No Root Required**: Works completely in user space.
- **Vulkan & GLES Interception**: Fully supports both Vulkan 1.3 and OpenGL ES 3.2 rendering pipelines.
- **3dmigoto Config Compatibility**: Parses and applies `.ini` section overrides (vertex buffers, index buffers, shader overrides, etc.).
- **Shader Fixes**: Executes SPIR-V shader patcheing and handles layout adjustments to fix shadows and transparency issues (`Orfix`/`Txfix` rules).
- **Mobile Texture Pipeline**: Supports transparent loading and substitution of mobile-native texture formats (ASTC, ETC2, RGBA8).
- **Dynamic Manager UI**: Search, filter, and toggle individual mods dynamically through a premium modern dark-themed launcher app.

---

## Setup & Prerequisites

Because `gimi_arm64` updates secure settings to inject the graphics layer, it requires the Android permission `WRITE_SECURE_SETTINGS`.

### Step 1: Install Shizuku
To grant permissions without a PC, it is recommended to use **Shizuku**:
1. Download Shizuku from the Google Play Store or GitHub.
2. Open Shizuku and follow the instructions to start it via **Wireless Debugging** (no root needed).
3. Once Shizuku is running, open the **GIMI Launcher** and click the **"Conceder Permissão Auto via Shizuku"** button.

### Step 2: Alternative (PC via ADB)
If you prefer using a PC, connect your device via USB with USB Debugging enabled, and run the following command:
```bash
adb shell pm grant com.gimi.launcher android.permission.WRITE_SECURE_SETTINGS
```

---

## How to Install and Manage Mods

1. Create a directory named `/sdcard/GIMI/Mods/` on your device.
2. Extract your 3dmigoto mod folders into `/sdcard/GIMI/Mods/`. Each mod should be in its own subfolder containing the `.ini` configuration and `.buf`/`.dds` assets.
3. Open **GIMI Launcher**.
4. Navigate to the **Mod Manager** tab at the bottom to search, verify ini counts, and toggle individual mods on or off.
5. Go back to the **Dashboard** tab, select the target game variant (e.g. Global/Play Store, Samsung, China), and click **"INJECT LAYER & LAUNCH GAME"**.

---

## Build from Source

### Using Gradle
Build the project using standard Android Gradle:
```bash
./gradlew assembleDebug
```

### Lightweight Build (Termux / NDK Script)
If building directly on an Android device via Termux or in a lightweight CLI environment:
```bash
bash build_termux.sh
```
This script compiles the C++ native targets (`libgimi_arm64.so`), compiles Java/Kotlin sources, converts them to dex, and packages and signs the signed `GIMI-Launcher.apk` package.

---

## Contribution Guide

We welcome contributions to improve interception stability, config parsing coverage, or the UI.

### Guidelines:
- **Clean C++20**: Native engine code should be written in modern C++20 using standard dispatch templates and RAII patterns.
- **Safety First**: Never introduce memory patches that modify code segments of the target game. Intercept only at the standard driver layer boundaries (Vulkan Layer / EGL Dispatch).
- **Clean Commits**: Follow structured commit messages (e.g., `feat(subsystem): description`, `fix(subsystem): fix bug`).

---

## License

This project is licensed under the MIT License - see the LICENSE file for details.
