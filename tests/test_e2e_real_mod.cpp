// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — E2E Test: Real Mod Parsing & Simulation
// ─────────────────────────────────────────────────────────────────────────────

#include "config/mod_config.h"
#include "config/ini_parser.h"
#include "mesh/mesh_swapper.h"
#include "textures/texture_loader.h"
#include "utils/logger.h"

#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

void test_mod_parsing() {
    std::cout << "[TEST] Running Real Mod Parsing Tests..." << std::endl;

    std::string mod_dir = "/sdcard/GIMI/Mods/Paimon Redesign by Rain_9";
    std::string ini_path = mod_dir + "/Paimon.ini";
    
    gimi::ModDatabase db;
    gimi::IniParser parser;
    bool success = parser.load(ini_path, db);
    assert(success && "Failed to load Paimon.ini");
    (void)success;

    // Validar extração das seções [TextureOverride...]
    // hash = 1f7f0600 (PaimonPosition)
    uint32_t hash_pos = 0x1f7f0600;
    assert(db.has_texture_override(hash_pos));
    
    // hash = 5b4cdc9c (PaimonHead)
    uint32_t hash_head = 0x5b4cdc9c;
    assert(db.has_texture_override(hash_head));
    (void)hash_head;

    // Validar overrides de buffers/texturas em resources
    const auto* res_pos = db.find_resource("ResourcePaimonPosition");
    assert(res_pos != nullptr);
    assert(res_pos->filename == "PaimonPosition.buf");
    assert(res_pos->type == "Buffer");
    (void)res_pos;

    const auto* res_tex = db.find_resource("ResourcePaimonHeadDiffuse");
    assert(res_tex != nullptr);
    assert(res_tex->filename == "PaimonHeadDiffuse.dds");
    (void)res_tex;

    std::cout << "[TEST] Real Mod Parsing Tests PASSED!" << std::endl;
}

void test_buffer_loading() {
    std::cout << "[TEST] Running Buffer/Texture Loading Tests..." << std::endl;

    std::string mod_dir = "/sdcard/GIMI/Mods/Paimon Redesign by Rain_9";

    auto check_file_exists_and_size = [](const std::string& path, size_t expected_min_size) {
        assert(std::filesystem::exists(path) && "File does not exist");
        size_t size = std::filesystem::file_size(path);
        assert(size >= expected_min_size && "File is too small or empty");
        (void)size;
        (void)expected_min_size;
    };

    // Validar buffers binários
    check_file_exists_and_size(mod_dir + "/PaimonPosition.buf", 100);
    check_file_exists_and_size(mod_dir + "/PaimonTexcoord.buf", 100);
    check_file_exists_and_size(mod_dir + "/PaimonHead.ib", 100);

    // Validar textura DDS
    check_file_exists_and_size(mod_dir + "/PaimonHeadDiffuse.dds", 100);

    // Mock register override for MeshSwapper
    // We cast some dummy values to VkBuffer to simulate the registration
    VkBuffer dummy_vb = reinterpret_cast<VkBuffer>(0x1000);
    VkBuffer dummy_ib = reinterpret_cast<VkBuffer>(0x2000);

    uint32_t hash_pos = 0x1f7f0600;
    gimi::MeshSwapper::instance().register_override(
        hash_pos, dummy_vb, dummy_ib, 30046, 0, 0
    );

    // Simulate Draw Call
    auto override_result = gimi::MeshSwapper::instance().try_swap(hash_pos);
    assert(override_result.should_override == true);
    assert(override_result.override_vb == dummy_vb);
    assert(override_result.override_ib == dummy_ib);
    assert(override_result.index_count == 30046);
    (void)override_result;

    std::cout << "[TEST] Buffer/Texture Loading Tests PASSED!" << std::endl;
}

int main() {
    std::cout << "=== GIMI ARM64 E2E Real Mod Tests ===" << std::endl;
    
    // Check if the mod directory exists before running tests
    if (!std::filesystem::exists("/sdcard/GIMI/Mods/Paimon Redesign by Rain_9/Paimon.ini")) {
        std::cerr << "[ERROR] Real mod files not found at /sdcard/GIMI/Mods/Paimon Redesign by Rain_9/. Cannot run test." << std::endl;
        return 1;
    }

    test_mod_parsing();
    test_buffer_loading();
    
    std::cout << "=== All E2E Real Mod Tests Passed Successfully! ===" << std::endl;
    return 0;
}
