<!-- GSD:project-start source:PROJECT.md -->

## Project

**gimi_arm64**

gimi_arm64 é o port do GIMI (Genshin Impact Model Importer, baseado no 3dmigoto) para a arquitetura mobile ARM64, focado inicialmente no Android. O projeto permite a substituição e injeção de modelos 3D, texturas e correções de shaders (como Orfix.ini e Txfix) diretamente no renderizador do jogo através de um Vulkan Layer / Graphics Hook não-destrutivo.

**Core Value:** Interceptar e substituir recursos gráficos (modelos, texturas e shaders) de forma não-destrutiva via camada de gráficos Vulkan/GLES, mantendo compatibilidade total com os mods e fixes existentes do 3dmigoto sem modificar os arquivos originais do jogo.

### Constraints

- **Platform**: Android ARM64 — Arquitetura de execução nativa do jogo no mobile.
- **Graphics API**: Vulkan / OpenGL ES — APIs de renderização utilizadas pelo jogo no Android.
- **Security / Anti-Cheat**: Non-destructive hooking — Interceptação feita exclusivamente na camada gráfica sem alterar o APK original ou memória do executável base.

<!-- GSD:project-end -->

<!-- GSD:stack-start source:research/STACK.md -->

## Technology Stack

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

# Environment setup (Android NDK r26+)

## Sources

- Vulkan Layer Specification (Khronos Group)
- 3dmigoto Open Source Specification (DirectX 11 Model Importer)
- Android NDK Graphics API Documentation

<!-- GSD:stack-end -->

<!-- GSD:conventions-start source:CONVENTIONS.md -->

## Conventions

Conventions not yet established. Will populate as patterns emerge during development.
<!-- GSD:conventions-end -->

<!-- GSD:architecture-start source:ARCHITECTURE.md -->

## Architecture

Architecture not yet mapped. Follow existing patterns found in the codebase.
<!-- GSD:architecture-end -->

<!-- GSD:skills-start source:skills/ -->

## Project Skills

No project skills found. Add skills to any of: `.agents/skills/`, `.agents/skills/`, `.cursor/skills/`, `.github/skills/`, or `.codex/skills/` with a `SKILL.md` index file.
<!-- GSD:skills-end -->

<!-- GSD:workflow-start source:GSD defaults -->

## GSD Workflow Enforcement

Before using Edit, Write, or other file-changing tools, start work through a GSD command so planning artifacts and execution context stay in sync.

Use these entry points:

- `/gsd-quick` for small fixes, doc updates, and ad-hoc tasks
- `/gsd-debug` for investigation and bug fixing
- `/gsd-execute-phase` for planned phase work

Do not make direct repo edits outside a GSD workflow unless the user explicitly asks to bypass it.
<!-- GSD:workflow-end -->

<!-- GSD:profile-start -->

## Developer Profile

> Profile not yet configured. Run `/gsd-profile-user` to generate your developer profile.
> This section is managed by `generate-claude-profile` -- do not edit manually.
<!-- GSD:profile-end -->
