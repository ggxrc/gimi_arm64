package com.gimi.launcher.ui.screens

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.navigation.NavGraph.Companion.findStartDestination
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.gimi.launcher.ui.navigation.GimiNavGraph
import com.gimi.launcher.ui.navigation.Screen
import com.gimi.launcher.ui.viewmodel.DashboardViewModel
import com.gimi.launcher.ui.viewmodel.ModManagerViewModel
import com.gimi.launcher.ui.viewmodel.SettingsViewModel

val screens = listOf(
    Screen.Dashboard,
    Screen.ModManager,
    Screen.Settings
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun GimiApp(
    dashboardViewModel: DashboardViewModel = DashboardViewModel(),
    modManagerViewModel: ModManagerViewModel = ModManagerViewModel(),
    settingsViewModel: SettingsViewModel = SettingsViewModel()
) {
    val navController = rememberNavController()
    val navBackStackEntry by navController.currentBackStackEntryAsState()
    val currentRoute = navBackStackEntry?.destination?.route

    Scaffold(
        bottomBar = {
            NavigationBar {
                screens.forEach { screen ->
                    NavigationBarItem(
                        selected = currentRoute == screen.route,
                        onClick = {
                            navController.navigate(screen.route) {
                                popUpTo(navController.graph.findStartDestination().id) {
                                    saveState = true
                                }
                                launchSingleTop = true
                                restoreState = true
                            }
                        },
                        label = { Text(screen.title) },
                        icon = {
                            Text(
                                text = when (screen) {
                                    Screen.Dashboard -> "⚡"
                                    Screen.ModManager -> "📦"
                                    Screen.Settings -> "⚙️"
                                }
                            )
                        }
                    )
                }
            }
        }
    ) { innerPadding ->
        Surface(modifier = Modifier.padding(innerPadding)) {
            GimiNavGraph(
                navController = navController,
                dashboardScreen = { DashboardScreen(viewModel = dashboardViewModel) },
                modManagerScreen = { ModManagerScreen(viewModel = modManagerViewModel) },
                settingsScreen = { SettingsScreen(viewModel = settingsViewModel) }
            )
        }
    }
}
