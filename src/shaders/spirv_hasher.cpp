// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — SPIR-V Bytecode Hasher Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "shaders/spirv_hasher.h"
#include "xxhash.h"

namespace gimi {

uint32_t compute_spirv_hash(const uint32_t* code, size_t code_size_bytes) noexcept {
    if (!code || code_size_bytes == 0) return 0;

    uint64_t h64 = XXH64(code, code_size_bytes, /*seed=*/0);
    return static_cast<uint32_t>(h64 & 0xFFFFFFFF);
}

} // namespace gimi
