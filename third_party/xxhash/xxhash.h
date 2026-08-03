/*
 * xxHash - Extremely Fast Hash algorithm
 * Header File — single-file implementation mode
 *
 * Copyright (C) 2012-2023 Yann Collet
 * BSD 2-Clause License — https://github.com/Cyan4973/xxHash
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef XXHASH_H_INCLUDED
#define XXHASH_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

/* ---- Public API ---------------------------------------------------------- */

typedef uint32_t XXH32_hash_t;
typedef uint64_t XXH64_hash_t;

/* XXH32: 32-bit hash */
XXH32_hash_t XXH32(const void* input, size_t length, uint32_t seed);

/* XXH64: 64-bit hash (primary algorithm for gimi_arm64) */
XXH64_hash_t XXH64(const void* input, size_t length, uint64_t seed);

/* ---- ARM64 Optimized Primes ---------------------------------------------- */
#define XXH_PRIME64_1  UINT64_C(0x9E3779B185EBCA87)
#define XXH_PRIME64_2  UINT64_C(0xC2B2AE3D27D4EB4F)
#define XXH_PRIME64_3  UINT64_C(0x165667B19E3779F9)
#define XXH_PRIME64_4  UINT64_C(0x85EBCA77C2B2AE63)
#define XXH_PRIME64_5  UINT64_C(0x27D4EB2F165667C5)

#define XXH_PRIME32_1  UINT32_C(0x9E3779B1)
#define XXH_PRIME32_2  UINT32_C(0x85EBCA77)
#define XXH_PRIME32_3  UINT32_C(0xC2B2AE3D)
#define XXH_PRIME32_4  UINT32_C(0x27D4EB2F)
#define XXH_PRIME32_5  UINT32_C(0x165667B1)

#endif /* XXHASH_H_INCLUDED */
