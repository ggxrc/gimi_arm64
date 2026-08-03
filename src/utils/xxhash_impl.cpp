// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — xxHash Implementation Unit
//
// This is the SOLE compilation unit for xxHash in the project.
// By defining XXH_IMPLEMENTATION before the include we get the full
// implementation in this one .cpp file without duplicating symbols.
//
// Compile flags applied by CMake: -O3 — ensures auto-vectorization on ARM64.
// ─────────────────────────────────────────────────────────────────────────────

#include "xxhash.h"

#include <stdint.h>
#include <string.h>  // memcpy

// ─── xxHash64 Implementation ──────────────────────────────────────────────────
// Portable C implementation that compiles clean on NDK r26b+ / ARM64.

static uint64_t XXH_read64(const void* ptr) {
    uint64_t v;
    memcpy(&v, ptr, 8);
    return v;
}

static uint32_t XXH_read32(const void* ptr) {
    uint32_t v;
    memcpy(&v, ptr, 4);
    return v;
}

static uint64_t XXH_rotl64(uint64_t x, int r) {
    return (x << r) | (x >> (64 - r));
}

static uint32_t XXH_rotl32(uint32_t x, int r) {
    return (x << r) | (x >> (32 - r));
}

static uint64_t XXH64_round(uint64_t acc, uint64_t input) {
    acc += input * XXH_PRIME64_2;
    acc = XXH_rotl64(acc, 31);
    acc *= XXH_PRIME64_1;
    return acc;
}

static uint64_t XXH64_mergeRound(uint64_t acc, uint64_t val) {
    val = XXH64_round(0, val);
    acc ^= val;
    acc = acc * XXH_PRIME64_1 + XXH_PRIME64_4;
    return acc;
}

static uint64_t XXH64_avalanche(uint64_t h64) {
    h64 ^= h64 >> 33;
    h64 *= XXH_PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= XXH_PRIME64_3;
    h64 ^= h64 >> 32;
    return h64;
}

uint64_t XXH64(const void* input, size_t len, uint64_t seed) {
    const uint8_t* p   = (const uint8_t*)input;
    const uint8_t* end = p + len;
    uint64_t h64;

    if (len >= 32) {
        const uint8_t* limit = end - 32;
        uint64_t v1 = seed + XXH_PRIME64_1 + XXH_PRIME64_2;
        uint64_t v2 = seed + XXH_PRIME64_2;
        uint64_t v3 = seed + 0;
        uint64_t v4 = seed - XXH_PRIME64_1;

        do {
            v1 = XXH64_round(v1, XXH_read64(p)); p += 8;
            v2 = XXH64_round(v2, XXH_read64(p)); p += 8;
            v3 = XXH64_round(v3, XXH_read64(p)); p += 8;
            v4 = XXH64_round(v4, XXH_read64(p)); p += 8;
        } while (p <= limit);

        h64 = XXH_rotl64(v1, 1) + XXH_rotl64(v2, 7) +
              XXH_rotl64(v3, 12) + XXH_rotl64(v4, 18);

        h64 = XXH64_mergeRound(h64, v1);
        h64 = XXH64_mergeRound(h64, v2);
        h64 = XXH64_mergeRound(h64, v3);
        h64 = XXH64_mergeRound(h64, v4);
    } else {
        h64 = seed + XXH_PRIME64_5;
    }

    h64 += (uint64_t)len;

    // Process remaining 8-byte chunks
    while (p + 8 <= end) {
        uint64_t k1 = XXH64_round(0, XXH_read64(p));
        h64 ^= k1;
        h64 = XXH_rotl64(h64, 27) * XXH_PRIME64_1 + XXH_PRIME64_4;
        p  += 8;
    }

    // Process remaining 4-byte chunk
    if (p + 4 <= end) {
        h64 ^= (uint64_t)(XXH_read32(p)) * XXH_PRIME64_1;
        h64 = XXH_rotl64(h64, 23) * XXH_PRIME64_2 + XXH_PRIME64_3;
        p  += 4;
    }

    // Process remaining bytes
    while (p < end) {
        h64 ^= (uint64_t)(*p) * XXH_PRIME64_5;
        h64 = XXH_rotl64(h64, 11) * XXH_PRIME64_1;
        p++;
    }

    return XXH64_avalanche(h64);
}

// ─── xxHash32 Implementation ──────────────────────────────────────────────────
static uint32_t XXH32_round(uint32_t acc, uint32_t input) {
    acc += input * XXH_PRIME32_2;
    acc = XXH_rotl32(acc, 13);
    acc *= XXH_PRIME32_1;
    return acc;
}

uint32_t XXH32(const void* input, size_t len, uint32_t seed) {
    const uint8_t* p   = (const uint8_t*)input;
    const uint8_t* end = p + len;
    uint32_t h32;

    if (len >= 16) {
        const uint8_t* limit = end - 16;
        uint32_t v1 = seed + XXH_PRIME32_1 + XXH_PRIME32_2;
        uint32_t v2 = seed + XXH_PRIME32_2;
        uint32_t v3 = seed + 0;
        uint32_t v4 = seed - XXH_PRIME32_1;

        do {
            v1 = XXH32_round(v1, XXH_read32(p)); p += 4;
            v2 = XXH32_round(v2, XXH_read32(p)); p += 4;
            v3 = XXH32_round(v3, XXH_read32(p)); p += 4;
            v4 = XXH32_round(v4, XXH_read32(p)); p += 4;
        } while (p <= limit);

        h32 = XXH_rotl32(v1, 1) + XXH_rotl32(v2, 7) +
              XXH_rotl32(v3, 12) + XXH_rotl32(v4, 18);
    } else {
        h32 = seed + XXH_PRIME32_5;
    }

    h32 += (uint32_t)len;

    while (p + 4 <= end) {
        h32 += XXH_read32(p) * XXH_PRIME32_3;
        h32 = XXH_rotl32(h32, 17) * XXH_PRIME32_4;
        p  += 4;
    }
    while (p < end) {
        h32 += (*p) * XXH_PRIME32_5;
        h32 = XXH_rotl32(h32, 11) * XXH_PRIME32_1;
        p++;
    }

    h32 ^= h32 >> 15;
    h32 *= XXH_PRIME32_2;
    h32 ^= h32 >> 13;
    h32 *= XXH_PRIME32_3;
    h32 ^= h32 >> 16;
    return h32;
}
