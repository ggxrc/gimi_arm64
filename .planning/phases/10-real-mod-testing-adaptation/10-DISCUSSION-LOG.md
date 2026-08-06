# Phase 10: Testes Reais, Adaptação de Mods & Notificação de Controle - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-08-06
**Phase:** 10-real-mod-testing-adaptation
**Areas discussed:** Visual Style & UI Refactoring, Dump & Foreground Service Removal, Open-Source Documentation

---

## Visual Style & UI Refactoring (Compose)

| Option | Description | Selected |
|--------|-------------|----------|
| Minimalista Escura Moderna | Fundo grafite/preto, cartões com bordas arredondadas de 16dp, destaques em ciano e verde neon | ✓ |
| Material Design 3 Padrão | Cores nativas do sistema com suporte a tema dinâmico | |
| Glassmorphism / Estilo Gaming | Cartões semi-transparentes com efeito de desfoque sobre um fundo gradiente sutil | |

**User's choice:** Minimalista Escura Moderna
**Notes:** Definido visual escuro moderno com cantos arredondados de 16dp e destaque ciano/verde neon para estados ativos.

---

## UI Navigation

| Option | Description | Selected |
|--------|-------------|----------|
| Manter as 3 abas inferiores | Dashboard, Mod Manager, Settings | ✓ |
| Simplificar para 2 abas | Launcher e Mod Manager | |
| Tela única rolável | Tudo unificado para acesso rápido | |

**User's choice:** Manter as 3 abas inferiores
**Notes:** Manter a barra de navegação inferior com 3 abas para melhor estruturação das responsabilidades.

---

## Mod Items Layout

| Option | Description | Selected |
|--------|-------------|----------|
| Cards individuais elegantes | Cantos arredondados, switch à direita, busca no topo e contador de ativos | ✓ |
| Lista ultra-compacta | Apenas nome e switch para maior densidade | |
| Cards com expansão detalhada | Ao tocar, revela arquivos internos associados | |

**User's choice:** Cards individuais elegantes
**Notes:** Cards com cantos arredondados e switch simples à direita na aba Mod Manager.

---

## Dump Removal Scope

| Option | Description | Selected |
|--------|-------------|----------|
| Remover completamente | Apagar o código de gravação em disco no C++ e remover a pasta de dump | ✓ |
| Desativar e ocultar | Manter o código C++ intacto, mas desativado por padrão e oculto na UI | |

**User's choice:** Remover completamente
**Notes:** Exclusão definitiva do código de escrita e do diretório `/sdcard/GIMI/Dump/` para simplificar a codebase.

---

## Background Services Scope

| Option | Description | Selected |
|--------|-------------|----------|
| Remover ambos | Remover GimiForegroundService e GimiOverlayService por completo | ✓ |
| Remover apenas o ForegroundService | Remover notificação fixa, mas manter menu flutuante em jogo | |

**User's choice:** Remover ambos
**Notes:** Remover qualquer serviço ativo de segundo plano para deixar o Launcher o mais limpo possível.

---

## Documentation Language

| Option | Description | Selected |
|--------|-------------|----------|
| Bilíngue | README.md principal em Inglês, com tradução completa em README.pt-br.md | ✓ |
| Apenas em Inglês | Foco 100% internacional | |
| Apenas em Português | Foco 100% nacional | |

**User's choice:** Bilíngue
**Notes:** README principal em Inglês com link e tradução completa para o Português em README.pt-br.md.

---

## Technical Documentation Sections

| Option | Description | Selected |
|--------|-------------|----------|
| Detalhamento técnico da Vulkan Layer | Como a libgimi_arm64 intercepta a renderização de forma não-destrutiva | ✓ |
| Guia de contribuição | Como submeter novos mod loaders ou patches de shader | ✓ |
| Guia de configuração Shizuku | Detalhes sobre a configuração ADB / Shizuku | |
| Instruções de build Termux/Gradle | Passos detalhados para build local e corporativo | |

**User's choice:** Detalhamento técnico da Vulkan Layer, Guia de contribuição
**Notes:** Explicar a interceptação gráfica de forma aprofundada e prover guia de como contribuir.

---

## the agent's Discretion

Nenhuma decisão foi deixada ao arbítrio da inteligência artificial.

## Deferred Ideas

Nenhuma ideia foi postergada para fases futuras.
