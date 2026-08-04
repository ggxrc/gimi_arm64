package com.gimi.launcher.ui.viewmodel

import androidx.lifecycle.ViewModel
import com.gimi.launcher.jni.GimiNativeBridge
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

data class DashboardUiState(
    val layerStatus: Int = 0,
    val isShizukuAvailable: Boolean = false,
    val selectedPackage: String = "com.miHoYo.GenshinImpact",
    val statusMessage: String = "Ready"
)

class DashboardViewModel : ViewModel() {
    private val _uiState = MutableStateFlow(DashboardUiState())
    val uiState: StateFlow<DashboardUiState> = _uiState.asStateFlow()

    init {
        refreshStatus()
    }

    fun refreshStatus() {
        val status = GimiNativeBridge.getLayerStatus()
        val isShizuku = GimiNativeBridge.isShizukuAvailable()
        _uiState.value = _uiState.value.copy(
            layerStatus = status,
            isShizukuAvailable = isShizuku
        )
    }

    fun selectPackage(packageName: String) {
        _uiState.value = _uiState.value.copy(selectedPackage = packageName)
    }

    fun injectLayer() {
        val result = GimiNativeBridge.injectLayer(_uiState.value.selectedPackage)
        _uiState.value = _uiState.value.copy(
            statusMessage = if (result == 0) "Layer injected successfully" else "Injection failed"
        )
        refreshStatus()
    }
}
