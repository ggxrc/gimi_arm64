# gimi_arm64

## What This Is

gimi_arm64 é o port do GIMI (Genshin Impact Model Importer, baseado no 3dmigoto) para a arquitetura mobile ARM64, focado inicialmente no Android. O projeto permite a substituição e injeção de modelos 3D, texturas e correções de shaders (como Orfix.ini e Txfix) diretamente no renderizador do jogo através de um Vulkan Layer / Graphics Hook não-destrutivo.

## Core Value

Interceptar e substituir recursos gráficos (modelos, texturas e shaders) de forma não-destrutiva via camada de gráficos Vulkan/GLES, mantendo compatibilidade total com os mods e fixes existentes do 3dmigoto sem modificar os arquivos originais do jogo.

## Requirements

### Validated

- [x] Suporte a injeção via Vulkan Layer / Graphics Hooking em ARM64 no Android (Validated in Phase 1 & 8.6)
- [x] Parsing e execução de arquivos de configuração `.ini` do 3dmigoto (Orfix.ini, Txfix.ini e overrides) (Validated in Phase 2)
- [x] Interceptação e substituição de Vertex Buffers, Index Buffers e Shaders (Validated in Phase 3)
- [x] Suporte a substituição e conversão de texturas (DDS / BC7 para formatos móveis ASTC/ETC2) (Validated in Phase 4)
- [x] Interceptação puramente não-destrutiva no pipeline gráfico para evitar detecção e banimentos (Validated in Phase 1 & 8)

### Active

(None - all core requirements successfully delivered and validated)

### Out of Scope

- [ ] Modificação de arquivos do jogo no disco (APK, arquivos de asset originais) — para prevenir detecção por anti-cheat
- [ ] Injeção direta de código na lógica interna do jogo — manter escopo estritamente na camada de renderização gráfica
- [ ] Suporte inicial a plataformas iOS — foco inicial exclusivo em Android ARM64

## Context

O 3dmigoto é a base do GIMI no PC (Windows DirectX 11), permitindo a comunidade criar mods visuais e correções de iluminação/shader (Orfix/Txfix). Trazer essa capacidade para dispositivos móveis Android exige interceptar a API gráfica nativa (Vulkan / OpenGL ES em ARM64) sem violar a integridade do pacote do jogo original. Com a conclusão de todas as 10 fases planejadas, a interceptação Vulkan/GLES, o JNI bridge, o launcher minimalista escuro e a documentação bilíngue foram completamente consolidados e validados.

## Constraints

- **Platform**: Android ARM64 — Arquitetura de execução nativa do jogo no mobile.
- **Graphics API**: Vulkan / OpenGL ES — APIs de renderização utilizadas pelo jogo no Android.
- **Security / Anti-Cheat**: Non-destructive hooking — Interceptação feita exclusivamente na camada gráfica sem alterar o APK original ou memória do executável base.

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Abordagem Vulkan Layer / Graphics Hooking | Permite interceptar recursos gráficos sem alterar arquivos do jogo | Validated (Phase 1 & 8) |
| Compatibilidade nativa com .ini do 3dmigoto | Permite reutilizar a vasta biblioteca de mods e fixes existentes | Validated (Phase 2) |
| Launcher App Nativo (Compose) com Shizuku | Facilita a injeção da camada Vulkan e a gestão visual de mods sem exigir root | Validated (Phase 5, 7, 8.6) |
| Remoção de Dumping e Serviços em Background | Simplifica o launcher e reduz o impacto na performance e segurança (anti-cheat) | Validated (Phase 10) |

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd-transition`):
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to Key Decisions
5. "What This Is" still accurate? → Update if drifted

**After each milestone** (via `/gsd-complete-milestone`):
1. Full review of all sections
2. Core Value check — still the right priority?
3. Audit Out of Scope — reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-08-06 after Phase 10 completion*
