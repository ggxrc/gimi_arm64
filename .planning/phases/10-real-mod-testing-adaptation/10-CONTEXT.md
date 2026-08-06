# Phase 10: Testes Reais, Adaptação de Mods & Notificação de Controle - Context

**Gathered:** 2026-08-06
**Status:** Ready for planning

<domain>
## Phase Boundary

Esta fase (v1.0 Final) foca na preparação do projeto gimi_arm64 para sua publicação open-source. Ela engloba a refatoração visual completa da interface do aplicativo Android (Jetpack Compose) para um design premium e amigável, a simplificação da arquitetura através da remoção completa das funcionalidades de Dump e serviços em segundo plano, e a elaboração de documentação detalhada bilíngue explicando a injeção nativa de camadas Vulkan/GLES.

</domain>

<decisions>
## Implementation Decisions

### 1. Visual Style & UI Refactoring (Compose)
- **D-01:** Utilizar o estilo de design "Minimalista Escura Moderna", aplicando um fundo grafite/preto, cartões elegantes com cantos arredondados de 16dp e destaques nas cores ciano e verde neon para exibir estados ativos e de sucesso (como Shizuku conectado e Vulkan ativo).
- **D-02:** Manter a barra de navegação inferior com 3 abas (Dashboard, Mod Manager, Settings) para organizar claramente as responsabilidades do Launcher.
- **D-03:** Refatorar a listagem de itens da aba Mod Manager para utilizar cards individuais limpos com cantos arredondados, switch de ativação rápida à direita, busca textual em tempo real no topo e contador de mods ativos.

### 2. Dump & Background Services Removal
- **D-04:** Remover completamente a engine C++ de Dump de hashes e o salvamento em disco de arquivos `.buf` / `.dds`. A criação e dependência do diretório `/sdcard/GIMI/Dump/` devem ser eliminadas.
- **D-05:** Deletar os serviços em segundo plano `GimiForegroundService` (notificação persistente de controle) e `GimiOverlayService` (menu flutuante em jogo), simplificando a arquitetura do Launcher para atuar apenas como configurador de ambiente (Settings.Global) e lançador limpo do jogo, sem processos adicionais ativos durante a execução do Genshin Impact.

### 3. Open-Source Documentation
- **D-06:** Estrutura bilíngue de documentação. O `README.md` principal na raiz do projeto será escrito em Inglês técnico focado na comunidade internacional de modding, com uma tradução integral em Português disponível em `README.pt-br.md`.
- **D-07:** Produzir detalhamento aprofundado explicando como funciona a interceptação de camadas gráficas Vulkan/EGL Hooks no Android de forma não-destrutiva, além de um Guia de Contribuição detalhado instruindo a comunidade sobre como submeter novos patches de shader ou melhorias no loader.

### the agent's Discretion
- Nenhuma decisão foi delegada ao arbítrio da IA; todos os gray areas essenciais foram acordados diretamente com o usuário.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Android Vulkan Layer Documentation
- `app/src/main/assets/vulkan/explicit_layer.d/VkLayer_gimi_arm64.json` — Arquivo de manifesto da camada gráfica Vulkan.

### Launcher Interface Configuration
- `app/src/main/java/com/gimi/launcher/MainActivity.java` — Classe central que renderiza a interface do app.
- `app/src/main/java/com/gimi/launcher/service/ShizukuManager.java` — Módulo responsável pelas chamadas privilegiadas via Shizuku.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `ShizukuManager.java`: Já implementa a conexão via Binder e o tratamento de permissões para concessão de `WRITE_SECURE_SETTINGS` e injeção do Vulkan.
- Estilos de cards e drawable helpers já definidos no Java do launcher que podem ser remapeados/aprimorados na refatoração visual.

### Established Patterns
- Configuração de ambiente gráfico no Android usando `Settings.Global` para injeção transparente sem root via Shizuku.

### Integration Points
- `MainActivity.java`: Ponto central onde os cliques nos botões de navegação, injeção e alteração de switches de mods devem se integrar.

</code_context>

<specifics>
## Specific Ideas

- O design visual deve parecer premium e limpo, evitando layouts cinzas ou sem contraste. Cores de contraste selecionadas: Grafite profundo para background, cinza-azulado médio para cards e bordas, verde neon/ciano para acentos de sucesso.

</specifics>

<deferred>
## Deferred Ideas

- Nenhuma ideia ou recurso adicional foi postergado para esta fase.

</deferred>

---

*Phase: 10-real-mod-testing-adaptation*
*Context gathered: 2026-08-06*
