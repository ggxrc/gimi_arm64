cmake_minimum_required(VERSION 3.22)

project(gimi_arm64 VERSION 0.1.0 LANGUAGES CXX)

# ─── C++ Standard ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─── Build Flags ─────────────────────────────────────────────────────────────
add_compile_options(
    -O3
    -Wall
    -Wextra
    -Wpedantic
    -fvisibility=hidden
    -fno-rtti
)

# ─── NDK / Android System Libraries ──────────────────────────────────────────
if(EXISTS "/system/lib64/liblog.so")
    set(log-lib "/system/lib64/liblog.so")
else()
    set(log-lib "log")
endif()

if(EXISTS "/system/lib64/libandroid.so")
    set(android-lib "/system/lib64/libandroid.so")
else()
    set(android-lib "android")
endif()

if(EXISTS "/system/lib64/libvulkan.so")
    set(vulkan-lib "/system/lib64/libvulkan.so")
else()
    set(vulkan-lib "vulkan")
endif()

# ─── Source Files ─────────────────────────────────────────────────────────────
set(GIMI_SOURCES
    src/main.cpp
    src/utils/logger.cpp
    src/utils/xxhash_impl.cpp             # Phase 02-02: xxHash implementation unit
    # Phase 01-02: Vulkan Layer / EGL Dispatch Table
    src/graphics/vulkan_layer.cpp
    src/graphics/egl_hook.cpp
    src/graphics/graphics_dispatcher.cpp
    # Phase 02-01: .ini Parser & ModDatabase
    src/config/ini_parser.cpp
    src/config/mod_config.cpp
    # Phase 02-02: Hash Engine
    src/hash/hash_registry.cpp
    src/hash/resource_hash_engine.cpp
    # Phase 03-01: Draw Call Interception & Mesh Swapper
    src/graphics/command_buffer_state.cpp
    src/mesh/mesh_swapper.cpp
    # Phase 03-02: Vertex Layout Adapter
    src/mesh/dxgi_vulkan_format.cpp
    src/mesh/vertex_layout_adapter.cpp
    # Phase 03-03: Shader Fix Engine
    src/shaders/spirv_hasher.cpp
    src/shaders/shader_fix_engine.cpp
    # Phase 04: Texture Pipeline
    src/textures/image_state_tracker.cpp
    src/textures/texture_loader.cpp
    src/textures/texture_swapper.cpp
    # Phase 05: Android Launcher App & Injector Manager
    src/launcher/mod_manager_service.cpp
    src/launcher/shizuku_layer_injector.cpp
    # inih C library (third_party)
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/inih/ini.c
)

# ─── Executable: Launcher Integration Tests ──────────────────────────────────
add_executable(test_launcher_integration
    tests/test_launcher_integration.cpp
    src/launcher/mod_manager_service.cpp
    src/launcher/shizuku_layer_injector.cpp
    src/utils/logger.cpp
)
target_include_directories(test_launcher_integration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_link_libraries(test_launcher_integration PRIVATE
    ${log-lib}
)

# ─── Shared Library Target ────────────────────────────────────────────────────
add_library(gimi_arm64 SHARED ${GIMI_SOURCES})

# ─── Include Directories ──────────────────────────────────────────────────────────
include_directories(SYSTEM
    /usr/include
    /data/data/com.termux/files/usr/include
)

target_include_directories(gimi_arm64
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party            # Vulkan / EGL / KHR headers
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/inih      # inih (Phase 02-01)
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/xxhash    # xxHash (Phase 02-02)
)

target_link_libraries(gimi_arm64
    PRIVATE
        ${log-lib}
        ${android-lib}
        ${vulkan-lib}
)

# ─── Linker Flags ─────────────────────────────────────────────────────────────
# Strip unused symbols and prevent exporting internal symbols
target_link_options(gimi_arm64
    PRIVATE
        -Wl,--exclude-libs,ALL
        -Wl,--gc-sections
)
