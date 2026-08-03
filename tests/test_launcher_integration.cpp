// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Integration Tests: Launcher & Shizuku Injector Service
// ─────────────────────────────────────────────────────────────────────────────

#include "launcher/mod_manager_service.h"
#include "launcher/shizuku_layer_injector.h"
#include "utils/logger.h"

#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>

void test_mod_manager_service() {
    std::cout << "[TEST] Running ModManagerService integration tests..." << std::endl;

    std::string test_dir = "/tmp/gimi_test_mods";
    std::error_code ec;
    std::filesystem::create_directories(test_dir + "/TestModA", ec);
    std::filesystem::create_directories(test_dir + "/TestModB.disabled", ec);

    // Create a dummy ini file
    std::ofstream ini_file(test_dir + "/TestModA/mod.ini");
    ini_file << "[TextureOverrideA]\nhash = 12345678\n";
    ini_file.close();

    auto mods = gimi::ModManagerService::instance().scan_mods(test_dir);
    assert(mods.size() == 2);

    bool modA_found = false, modB_found = false;
    for (const auto& mod : mods) {
        if (mod.name == "TestModA") {
            assert(mod.enabled == true);
            assert(mod.resource_count == 1);
            modA_found = true;
        } else if (mod.name == "TestModB") {
            assert(mod.enabled == false);
            modB_found = true;
        }
    }
    assert(modA_found && modB_found);

    // Test toggle
    bool toggled = gimi::ModManagerService::instance().set_mod_enabled(test_dir + "/TestModA", false);
    assert(toggled);
    assert(std::filesystem::exists(test_dir + "/TestModA.disabled"));

    // Cleanup
    std::filesystem::remove_all(test_dir, ec);
    std::cout << "[TEST] ModManagerService integration tests PASSED!" << std::endl;
}

void test_shizuku_layer_injector() {
    std::cout << "[TEST] Running ShizukuLayerInjector package mapping tests..." << std::endl;

    assert(gimi::ShizukuLayerInjector::get_package_name(gimi::GameDistribution::PlayStore) == "com.miHoYo.GenshinImpact");
    assert(gimi::ShizukuLayerInjector::get_package_name(gimi::GameDistribution::GalaxyStore) == "com.miHoYo.GenshinImpact.samsung");
    assert(gimi::ShizukuLayerInjector::get_package_name(gimi::GameDistribution::ChinaServer) == "com.yuanshen.site");
    assert(gimi::ShizukuLayerInjector::get_package_name(gimi::GameDistribution::Bilibili) == "com.miHoYo.ys.bilibili");

    std::cout << "[TEST] ShizukuLayerInjector package mapping tests PASSED!" << std::endl;
}

int main() {
    std::cout << "=== GIMI ARM64 Launcher Integration Tests ===" << std::endl;
    test_mod_manager_service();
    test_shizuku_layer_injector();
    std::cout << "=== All Launcher Integration Tests Passed Successfully! ===" << std::endl;
    return 0;
}
