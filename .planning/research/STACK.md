# Technology Stack: gimi_arm64

**Project:** gimi_arm64 (3dmigoto Model Importer for Android ARM64)
**Researched:** 2026-08-03

## Recommended Stack

### Core Framework & Interception
| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| Android NDK | r26b+ | Native C++20 Development | Core performance required for real-time graphics hooking on ARM64 |
| Vulkan API & Layer Spec | 1.3 | Vulkan Graphics Interception | Cleanest, non-destructive method to intercept draw calls and descriptors on Android |
| Dobby / Substrate | Latest | ARM64 Inline Hooking | Lightweight symbol & function hooking for EGL/Vulkan entrypoints |
| iniParser / inih | Latest | 3dmigoto `.ini` Parser | C/C++ lightweight parser compatible with 3dmigoto syntax (Orfix.ini, Txfix, etc.) |

### Graphics & Asset Processing
| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| SPIRV-Cross | Latest | Shader Reflection & Translation | Converts shader bytecode/inputs between DX11/HLSL paradigms and Vulkan SPIR-V |
| ASTC / ETC2 Texture Tools | Latest | Mobile Texture Format Support | Converts PC DDS/BC7 textures to mobile-compatible GPU compressed textures |
| Dear ImGui (Vulkan) | 1.90+ | In-game Overlay (Optional) | Lightweight overlay for mod status, hot-reloading, and debug info |

## Alternatives Considered

| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| Hooking | Vulkan Layer Spec + Dobby | APK Patching / Modifying APK | Modifying APK violates game integrity and triggers anti-cheat bans |
| Texture Format | ASTC / ETC2 | Raw RGBA8 | Raw RGBA textures consume excessive VRAM on mobile GPUs causing OOM crashes |

## Installation & Build Requirements

```bash
# Environment setup (Android NDK r26+)
export ANDROID_NDK_HOME=/path/to/ndk
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a
ninja -C build
```

## Sources
- Vulkan Layer Specification (Khronos Group)
- 3dmigoto Open Source Specification (DirectX 11 Model Importer)
- Android NDK Graphics API Documentation
