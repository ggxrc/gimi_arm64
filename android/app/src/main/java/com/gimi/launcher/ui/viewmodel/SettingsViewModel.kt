package com.gimi.launcher.ui.viewmodel

import androidx.lifecycle.ViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

data class SettingsUiState(
    val modsPath: String = "/sdcard/GIMI/Mods",
    val appVersion: String = "1.0.0 (ARM64)",
    val vulkanVersion: String = "Vulkan 1.3",
    val logs: List<String> = listOf(
        "[INFO] GIMI Launcher initialized",
        "[INFO] Native library libgimi_arm64.so loaded",
        "[INFO] Vulkan graphics layer status: OK",
        "[INFO] Target path set to /sdcard/GIMI/Mods"
    )
)

class SettingsViewModel : ViewModel() {
    private val _uiState = MutableStateFlow(SettingsUiState())
    val uiState: StateFlow<SettingsUiState> = _uiState.asStateFlow()

    fun updateModsPath(newPath: String) {
        _uiState.value = _uiState.value.copy(modsPath = newPath)
        addLog("[CONFIG] Mods path updated to: $newPath")
    }

    fun addLog(message: String) {
        val currentLogs = _uiState.value.logs.toMutableList()
        currentLogs.add(message)
        _uiState.value = _uiState.value.copy(logs = currentLogs)
    }

    fun clearLogs() {
        _uiState.value = _uiState.value.copy(logs = emptyList())
    }
}
