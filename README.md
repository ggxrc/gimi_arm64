# gimi_arm64

**3dmigoto Model Importer for Android ARM64 — Non-destructive Vulkan/GLES Graphics Hook**

> Port of GIMI (Genshin Impact Model Importer) to Android ARM64, enabling 3D model, texture, and shader replacement via a non-destructive Vulkan Layer / Graphics Hook without modifying the game APK.

---

## Project Status

| Phase | Description | Status |
|-------|-------------|--------|
| 1 | Engine de Interceptação Vulkan ARM64 & Infraestrutura Nativa | ✅ In Progress |
| 2 | Parser de Configurações 3dmigoto & Gestão de Hashes | ⏳ Pending |
| 3 | Substituição de Modelos & Shader Fixes | ⏳ Pending |
| 4 | Pipeline de Texturas & Compatibilidade de Formatos Móveis | ⏳ Pending |

---

## Architecture

```
Genshin Impact (Android ARM64)
        │
        ▼  Vulkan / GLES Calls
┌────────────────────────────┐
│   libgimi_arm64.so         │
│   (Vulkan Layer / EGL Hook)│
│   ─────────────────────── │
│   GraphicsDispatcher       │  ← detects Vulkan vs GLES
│   VulkanLayerRegistry      │  ← dispatch table (RAM only)
│   EGLHook (Dobby)          │  ← eglGetProcAddress hook
└────────────────────────────┘
        │
        ▼  (Phase 2+) Hash Lookup → Mod Override
┌────────────────────────────┐
│   3dmigoto Config Engine   │
│   Resource Replacement     │
│   Format Transpiler        │
└────────────────────────────┘
        │
        ▼
    GPU Hardware
```

---

## Build Requirements

- **Android NDK**: r26b or later
- **CMake**: 3.22+
- **ABI**: `arm64-v8a` only
- **C++ Standard**: C++20

### Build Instructions

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk-r26b

cmake -B build \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-30 \
  -DCMAKE_BUILD_TYPE=Release

ninja -C build
```

Output: `build/libgimi_arm64.so`

### Optional: Dobby Integration (for GLES Hook)

```bash
# Clone Dobby as a submodule
git submodule add https://github.com/jmpews/Dobby.git third_party/Dobby
```

Then add to `CMakeLists.txt`:
```cmake
add_subdirectory(third_party/Dobby)
target_link_libraries(gimi_arm64 PRIVATE dobby)
```

---

## Project Structure

```
gimi_arm64/
├── CMakeLists.txt                  # NDK C++20 build system
├── VkLayer_gimi_arm64.json         # Vulkan Layer manifest (loader discovery)
├── include/
│   ├── utils/
│   │   └── logger.h                # LOGD/LOGI/LOGW/LOGE macros
│   └── graphics/
│       ├── vulkan_layer.h          # Vulkan Layer dispatch tables
│       ├── egl_hook.h              # EGL/GLES hook (Dobby-based)
│       └── graphics_dispatcher.h  # Unified API detection & routing
└── src/
    ├── main.cpp                    # Library constructor/destructor
    ├── utils/
    │   └── logger.cpp
    └── graphics/
        ├── vulkan_layer.cpp
        ├── egl_hook.cpp
        └── graphics_dispatcher.cpp
```

---

## Non-Destructive Guarantee

All interception is performed **exclusively in RAM**:
- ✅ No APK modification
- ✅ No on-disk ELF patching  
- ✅ No SHA256 checksum violations
- ✅ No anti-cheat trigger paths

---

## Logcat Output

```
adb logcat -s gimi_arm64
```

Expected on successful load:
```
I/gimi_arm64: ╔══════════════════════════════════════════════════╗
I/gimi_arm64: ║        gimi_arm64  v0.1.0                        ║
I/gimi_arm64: ║  Vulkan/GLES Graphics Hook — arm64-v8a           ║
I/gimi_arm64: ╚══════════════════════════════════════════════════╝
I/gimi_arm64: GraphicsDispatcher: Vulkan layer active (loader-managed).
I/gimi_arm64: gimi_arm64 ready. Non-destructive interception active.
```
