# Phase 2: Parser de Configurações 3dmigoto & Gestão de Hashes - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-08-03
**Phase:** 02-parser-de-configuracoes-3dmigoto-gestao-de-hashes
**Areas discussed:** Estratégia de parsing de `.ini`, Algoritmo de hashing, Estratégia de cache

---

## Parser de Configurações `.ini`

| Option | Description | Selected |
|--------|-------------|----------|
| Implementação própria do zero | Parser C++ escrito manualmente | |
| `inih` (C header-only) | Biblioteca leve e comprovada, zero dependências, NDK-compatible | ✓ |
| Boost.PropertyTree | Header-heavy, dependência pesada para mobile | |

**Notes:** `inih` foi escolhido por ser extremamente leve (dois arquivos), amplamente testado e compatível com o NDK sem modificações. A classe `IniParser` envolve a API C com uma interface C++20 mais ergonômica.

---

## Algoritmo de Hashing de Recursos

| Option | Description | Selected |
|--------|-------------|----------|
| CRC32C (instrução ARM) | Nativo em ARMv8.1+, mas não portável para todos os Androids | |
| MurmurHash3 | Rápido mas não mais mantido ativamente | |
| xxHash64 | Header-only, ~10 GB/s em ARM64, zero alocações, bem mantido | ✓ |

**Notes:** xxHash64 selecionado por sua velocidade superior em ARM64 e pela API minimalista. O resultado é truncado para `uint32_t` (32 bits) para manter compatibilidade com o formato de hash de 8 hex dígitos do 3dmigoto.

---

## Estratégia de Cache

| Option | Description | Selected |
|--------|-------------|----------|
| Cache por frame (limpar a cada frame) | Simples mas ineficiente para buffers estáticos | |
| Cache por handle Vulkan (invalidado em destroy) | O(1) lookup com eviction baseada em eventos de destruição | ✓ |
| Cache com TTL (time-to-live) | Complexidade desnecessária para este caso de uso | |

**Notes:** Cache por handle Vulkan é ideal pois buffers de modelos em jogos como Genshin Impact são tipicamente estáticos (mesma geometria por muitos frames). A invalidação é event-driven via interceptação de `vkDestroyBuffer`/`vkDestroyImage`.

---

## Agent's Discretion

- Divisão interna dos arquivos `.h` / `.cpp` dentro de `include/config/` e `src/config/`.
- Estratégia exata de normalização (trim/lowercase) de chaves no parser.
- Tamanho exato do sample buffer para hashing (padrão: 4096 bytes).
