package com.gimi.launcher.ui.viewmodel

import androidx.lifecycle.ViewModel
import com.gimi.launcher.native.GimiNativeBridge
import com.gimi.launcher.native.ModInfo
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

data class ModManagerUiState(
    val mods: List<ModInfo> = emptyList(),
    val searchQuery: String = "",
    val isLoading: Boolean = false
)

class ModManagerViewModel : ViewModel() {
    private val _uiState = MutableStateFlow(ModManagerUiState())
    val uiState: StateFlow<ModManagerUiState> = _uiState.asStateFlow()

    init {
        loadMods()
    }

    fun loadMods(modsPath: String = "/sdcard/GIMI/Mods") {
        _uiState.value = _uiState.value.copy(isLoading = true)
        val scanned = GimiNativeBridge.scanMods(modsPath)
        _uiState.value = _uiState.value.copy(mods = scanned, isLoading = false)
    }

    fun toggleMod(mod: ModInfo) {
        val success = GimiNativeBridge.toggleMod(mod.path, !mod.isEnabled)
        if (success) {
            loadMods()
        }
    }

    fun setSearchQuery(query: String) {
        _uiState.value = _uiState.value.copy(searchQuery = query)
    }
}
