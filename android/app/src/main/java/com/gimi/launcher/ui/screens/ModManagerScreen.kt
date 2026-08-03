package com.gimi.launcher.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.gimi.launcher.native.ModInfo
import com.gimi.launcher.ui.viewmodel.ModManagerViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ModManagerScreen(viewModel: ModManagerViewModel) {
    val state by viewModel.uiState.collectAsState()

    val filteredMods = remember(state.mods, state.searchQuery) {
        if (state.searchQuery.isBlank()) {
            state.mods
        } else {
            state.mods.filter { it.name.contains(state.searchQuery, ignoreCase = true) }
        }
    }

    val activeCount = remember(state.mods) { state.mods.count { it.isEnabled } }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Text(
            text = "Mod Manager",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold
        )

        // Search Bar & Counter Header
        OutlinedTextField(
            value = state.searchQuery,
            onValueChange = { viewModel.setSearchQuery(it) },
            label = { Text("Search Mods") },
            placeholder = { Text("Filter by mod name...") },
            modifier = Modifier.fillMaxWidth(),
            singleLine = true
        )

        // Summary Bar
        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.secondaryContainer
            )
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "Total Mods Found: ${state.mods.size}",
                    style = MaterialTheme.typography.bodyLarge,
                    fontWeight = FontWeight.SemiBold
                )
                Surface(
                    color = MaterialTheme.colorScheme.primary,
                    shape = MaterialTheme.shapes.small
                ) {
                    Text(
                        text = "$activeCount Active",
                        color = Color.White,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp),
                        style = MaterialTheme.typography.bodyMedium,
                        fontWeight = FontWeight.Bold
                    )
                }
            }
        }

        if (state.isLoading) {
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
            ) {
                CircularProgressIndicator()
            }
        } else if (filteredMods.isEmpty()) {
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = if (state.mods.isEmpty()) "No mods found in /sdcard/GIMI/Mods/\nAdd mods or check directory settings." else "No mods matching '${state.searchQuery}'",
                    style = MaterialTheme.typography.bodyLarge,
                    color = Color.Gray
                )
            }
        } else {
            LazyColumn(
                verticalArrangement = Arrangement.spacedBy(8.dp),
                modifier = Modifier.fillMaxSize()
            ) {
                items(filteredMods, key = { it.path }) { mod ->
                    ModItemCard(
                        mod = mod,
                        onToggle = { viewModel.toggleMod(mod) }
                    )
                }
            }
        }
    }
}

@Composable
fun ModItemCard(mod: ModInfo, onToggle: (Boolean) -> Unit) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = if (mod.isEnabled) MaterialTheme.colorScheme.surfaceVariant else Color(0xFFF5F5F5)
        )
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = mod.name,
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold
                )
                Spacer(modifier = Modifier.height(4.dp))
                Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                    Text(
                        text = "📄 ${mod.iniCount} ini files",
                        style = MaterialTheme.typography.bodySmall,
                        color = Color.DarkGray
                    )
                    Text(
                        text = "🖼️ ${mod.textureCount} textures",
                        style = MaterialTheme.typography.bodySmall,
                        color = Color.DarkGray
                    )
                }
            }

            Switch(
                checked = mod.isEnabled,
                onCheckedChange = { onToggle(it) }
            )
        }
    }
}
