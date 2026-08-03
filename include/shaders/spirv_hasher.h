#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — SPIR-V Bytecode Hasher
//
// Computes the 3dmigoto-compatible 32-bit hash of SPIR-V shader bytecode
// using xxHash64 (truncated). Used to identify shaders at vkCreateShaderModule
// time for applying Orfix/Txfix rules.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdint>
#include <cstddef>

namespace gimi {

// Compute the 3dmigoto-compatible hash of SPIR-V bytecode.
// code: pointer to SPIR-V uint32_t words
// code_size_bytes: total size in bytes (must be a multiple of 4)
// Returns: truncated 32-bit hash (lower 32 bits of xxHash64)
uint32_t compute_spirv_hash(const uint32_t* code, size_t code_size_bytes) noexcept;

} // namespace gimi
