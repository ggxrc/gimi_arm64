package com.gimi.launcher.ui.navigation

sealed class Screen(val route: String, val title: String) {
    object Dashboard : Screen("dashboard", "Dashboard")
    object ModManager : Screen("mod_manager", "Mod Manager")
    object Settings : Screen("settings", "Settings")
}
