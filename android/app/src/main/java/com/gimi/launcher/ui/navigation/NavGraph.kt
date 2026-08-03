package com.gimi.launcher.ui.navigation

import androidx.compose.runtime.Composable
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable

@Composable
fun GimiNavGraph(
    navController: NavHostController,
    dashboardScreen: @Composable () -> Unit = {},
    modManagerScreen: @Composable () -> Unit = {},
    settingsScreen: @Composable () -> Unit = {}
) {
    NavHost(
        navController = navController,
        startDestination = Screen.Dashboard.route
    ) {
        composable(Screen.Dashboard.route) {
            dashboardScreen()
        }
        composable(Screen.ModManager.route) {
            modManagerScreen()
        }
        composable(Screen.Settings.route) {
            settingsScreen()
        }
    }
}
