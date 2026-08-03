// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Mesh Swapper Engine Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include "mesh/mesh_swapper.h"
#include "config/mod_config.h"
#include "utils/logger.h"

#include <unordered_map>

namespace gimi {

MeshSwapper& MeshSwapper::instance() noexcept {
    static MeshSwapper s_swapper;
    return s_swapper;
}

MeshOverrideResult MeshSwapper::try_swap(uint32_t active_vb_hash) noexcept {
    MeshOverrideResult result;

    // ── Check pre-registered overrides first (VRAM-resident modded buffers) ──
    auto it = m_overrides.find(active_vb_hash);
    if (it != m_overrides.end()) {
        result.should_override = true;
        result.override_vb     = it->second.vb;
        result.override_ib     = it->second.ib;
        result.index_count     = it->second.index_count;
        result.vertex_offset   = it->second.vertex_offset;
        result.first_index     = it->second.first_index;
        LOGD("MeshSwapper: override hit for hash 0x%08X → VB=%p IB=%p indexCount=%u",
             active_vb_hash, (void*)result.override_vb, (void*)result.override_ib,
             result.index_count);
        return result;
    }

    // ── Check ModDatabase for texture override rules ─────────────────────────
    // Even if no VRAM buffer is registered yet, we check the database to log
    // that a rule exists (useful for debugging / future hot-loading).
    const auto& db = ModConfig::instance().db();
    if (db.has_texture_override(active_vb_hash)) {
        LOGD("MeshSwapper: rule exists for hash 0x%08X but no VRAM buffer registered yet",
             active_vb_hash);
    }

    return result;
}

void MeshSwapper::register_override(uint32_t hash,
                                     VkBuffer vb, VkBuffer ib,
                                     uint32_t index_count,
                                     uint32_t vertex_offset,
                                     uint32_t first_index) noexcept {
    OverrideEntry entry;
    entry.vb           = vb;
    entry.ib           = ib;
    entry.index_count  = index_count;
    entry.vertex_offset = vertex_offset;
    entry.first_index  = first_index;
    m_overrides[hash] = entry;

    LOGI("MeshSwapper: registered override for hash 0x%08X — VB=%p IB=%p indexCount=%u",
         hash, (void*)vb, (void*)ib, index_count);
}

} // namespace gimi
