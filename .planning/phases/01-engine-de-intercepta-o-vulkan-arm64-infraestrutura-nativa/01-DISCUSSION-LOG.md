# Phase 1: Engine de Interceptação Vulkan ARM64 & Infraestrutura Nativa - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-08-03
**Phase:** 01-engine-de-intercepta-o-vulkan-arm64-infraestrutura-nativa
**Areas discussed:** Arquitetura de Interceptação Gráfica (Dual-API Vulkan + OpenGL ES), Toolchain Nativa Android NDK

---

## Interceptação Gráfica (Vulkan & OpenGL ES)

| Option | Description | Selected |
|--------|-------------|----------|
| Apenas Vulkan | Interceptação focada exclusivamente na API Vulkan | |
| Vulkan + OpenGL ES em paralelo | Suporte estático a ambas as APIs | |
| Detecção Dinâmica Dual-API | Engine detecta e intercepta dinamicamente Vulkan ou OpenGL ES em tempo de execução | ✓ |

**User's choice:** Detecção dinâmica em tempo de execução (Vulkan + OpenGL ES).
**Notes:** O usuário enfatizou a necessidade crítica de suportar aparelhos onde o Genshin Impact roda via OpenGL ES em vez de Vulkan.

---

## the agent's Discretion

- Estrutura interna de arquivos do CMake/C++ nativo.
- Seleção de utilitários de resolução de símbolos nativos (ex: Dobby).

## Deferred Ideas

- Overlay in-game ImGui (adiado para v2)
- Hot-reloading de mods (adiado para v2)
