package com.gimi.launcher.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.gimi.launcher.ui.viewmodel.DashboardViewModel

data class GamePackageOption(val label: String, val packageName: String)

val GAME_PACKAGES = listOf(
    GamePackageOption("Play Store", "com.miHoYo.GenshinImpact"),
    GamePackageOption("Galaxy Store", "com.miHoYo.GI.samsung"),
    GamePackageOption("China Server", "com.yuanshen.site"),
    GamePackageOption("Bilibili", "com.miHoYo.ys.bilibili")
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DashboardScreen(viewModel: DashboardViewModel) {
    val state by viewModel.uiState.collectAsState()
    var expandedDropdown by remember { mutableStateOf(false) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
            .verticalScroll(rememberScrollState()),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Text(
            text = "GIMI Launcher Dashboard",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold
        )

        // 1. Vulkan Layer Status Card
        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(
                containerColor = if (state.layerStatus > 0) Color(0xFFE8F5E9) else Color(0xFFFFEBEE)
            )
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = "Vulkan Layer Status",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold
                )
                Spacer(modifier = Modifier.height(4.dp))
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        text = if (state.layerStatus > 0) "🟢 ACTIVE (Hook Loaded)" else "🔴 INACTIVE (Layer Disconnected)",
                        style = MaterialTheme.typography.bodyLarge,
                        color = if (state.layerStatus > 0) Color(0xFF2E7D32) else Color(0xFFC62828),
                        fontWeight = FontWeight.Bold
                    )
                }
                Spacer(modifier = Modifier.height(4.dp))
                Text(
                    text = "Status Code: ${state.layerStatus}",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
        }

        // 2. Shizuku / ADB Status Card
        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(
                containerColor = if (state.isShizukuAvailable) Color(0xFFE3F2FD) else Color(0xFFFFF3E0)
            )
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = "Shizuku / ADB Rootless Service",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold
                )
                Spacer(modifier = Modifier.height(4.dp))
                Text(
                    text = if (state.isShizukuAvailable) "⚡ Shizuku Service Active & Permission Granted" else "⚠️ Shizuku Service Not Detected / Standalone Mode",
                    style = MaterialTheme.typography.bodyMedium,
                    color = if (state.isShizukuAvailable) Color(0xFF1565C0) else Color(0xFFEF6C00)
                )
            }
        }

        // 3. Game Package Dropdown / Chips Selector
        Card(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = "Target Game Variant",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold
                )
                Spacer(modifier = Modifier.height(8.dp))

                ExposedDropdownMenuBox(
                    expanded = expandedDropdown,
                    onExpandedChange = { expandedDropdown = !expandedDropdown }
                ) {
                    OutlinedTextField(
                        value = GAME_PACKAGES.find { it.packageName == state.selectedPackage }?.label ?: state.selectedPackage,
                        onValueChange = {},
                        readOnly = true,
                        label = { Text("Package Variant") },
                        trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(expanded = expandedDropdown) },
                        modifier = Modifier
                            .fillMaxWidth()
                            .menuAnchor()
                    )
                    ExposedDropdownMenu(
                        expanded = expandedDropdown,
                        onDismissRequest = { expandedDropdown = false }
                    ) {
                        GAME_PACKAGES.forEach { option ->
                            DropdownMenuItem(
                                text = { Text("${option.label} (${option.packageName})") },
                                onClick = {
                                    viewModel.selectPackage(option.packageName)
                                    expandedDropdown = false
                                }
                            )
                        }
                    }
                }

                Spacer(modifier = Modifier.height(8.dp))

                // Chips Selector
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    GAME_PACKAGES.forEach { pkg ->
                        FilterChip(
                            selected = (state.selectedPackage == pkg.packageName),
                            onClick = { viewModel.selectPackage(pkg.packageName) },
                            label = { Text(pkg.label, fontSize = 12.sp) }
                        )
                    }
                }
            }
        }

        // 4. Activate / Inject Button
        Button(
            onClick = { viewModel.injectLayer() },
            modifier = Modifier
                .fillMaxWidth()
                .height(56.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = MaterialTheme.colorScheme.primary
            )
        ) {
            Text(
                text = "INJECT VULKAN LAYER & LAUNCH",
                fontSize = 16.sp,
                fontWeight = FontWeight.Bold
            )
        }

        // Status Message Banner
        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.surfaceVariant
            )
        ) {
            Row(
                modifier = Modifier.padding(16.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "System Output: ${state.statusMessage}",
                    style = MaterialTheme.typography.bodyMedium,
                    fontWeight = FontWeight.Medium
                )
            }
        }
    }
}
