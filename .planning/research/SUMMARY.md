# Research Summary: gimi_arm64

**Domain:** Android ARM64 Graphics Hooking & 3dmigoto Model Importer
**Researched:** 2026-08-03
**Overall confidence:** HIGH

## Executive Summary

O projeto gimi_arm64 busca trazer o ecossistema de mods visuais do GIMI (3dmigoto) do Windows/DirectX 11 para o Android/ARM64. A pesquisa confirma que a abordagem técnica ideal é utilizar um **Vulkan Layer / EGL Graphics Interception Framework** em C++ nativo via Android NDK. Essa arquitetura permite interceptar e substituir chamadas de desenho, vertex/index buffers, texturas e correções de shaders (como Orfix.ini e Txfix) diretamente no pipeline gráfico, de forma completamente não-destrutiva para garantir segurança contra verificações de integridade de anti-cheat.

## Key Findings

**Stack:** Android NDK (C++20), Vulkan API / Layer Spec, Dobby Hooking Engine, SPIRV-Cross, inih parser.
**Architecture:** Pipeline dividido em 4 camadas: Interception Layer -> Config Engine (.ini) -> Resource Replacer -> Vulkan GPU Dispatch Table.
**Critical pitfall:** Risco de banimento por anti-cheat caso haja modificação de arquivos de jogo no disco. *Solução:* Interceptação gráfica puramente em memória e sem alteração do APK.

## Implications for Roadmap

Sugestão de estrutura de fases para o roteiro:

1. **Fase 1: Engine de Interceptação Vulkan ARM64 & Infraestrutura Nativa**
   - Configuração do projeto Android NDK/C++20
   - Implementação da camada de interceptação Vulkan (Vulkan Layer / Entrypoints)
   - Validação de hook não-destrutivo sem impactar a estabilidade do jogo

2. **Fase 2: Parser de Configurações 3dmigoto & Gestão de Hashes**
   - Parser C++ compatível com a sintaxe de `.ini` do 3dmigoto (`[TextureOverride]`, `Orfix.ini`, `Txfix.ini`)
   - Algoritmo de hashing de vertex buffers / index buffers em tempo real
   - Sistema de correspondência de hashes (Match Engine)

3. **Fase 3: Substituição de Recursos (Vertex, Index & Shader Overrides)**
   - Mapeamento e substituição de Vertex Buffers e Index Buffers para troca de modelos 3D
   - Tradução/Aplicações de correções de shaders (Orfix / Txfix) no pipeline Vulkan

4. **Fase 4: Pipeline de Texturas & Compatibilidade de Formatos Móveis**
   - Interceptação e substituição de texturas na memória VRAM
   - Suporte e conversão para formatos móveis (ASTC/ETC2)

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Tecnologias padrão da indústria para NDK e Vulkan no Android |
| Features | HIGH | Mapeadas diretamente das capacidades do 3dmigoto PC |
| Architecture | HIGH | Modelo baseado no padrão oficial Vulkan Layer Khronos |
| Pitfalls | HIGH | Fator crítico de anti-cheat bem delimitado |

## Gaps to Address

- Testes de desempenho específicos para GPUs Mali e Adreno em dispositivos Android ARM64 de entrada/intermediários.
