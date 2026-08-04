package com.gimi.launcher.jni

object GimiNativeBridge {
    init {
        try {
            System.loadLibrary("gimi_arm64")
        } catch (e: UnsatisfiedLinkError) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    external fun nativeScanMods(path: String): Array<ModInfo>

    @JvmStatic
    external fun nativeToggleMod(modPath: String, enable: Boolean): Boolean

    @JvmStatic
    external fun nativeInjectLayer(packageName: String): Int

    @JvmStatic
    external fun nativeGetLayerStatus(): Int

    @JvmStatic
    external fun nativeIsShizukuAvailable(): Boolean

    fun scanMods(path: String = "/sdcard/GIMI/Mods"): List<ModInfo> {
        return try {
            nativeScanMods(path).toList()
        } catch (e: Throwable) {
            emptyList()
        }
    }

    fun toggleMod(modPath: String, enable: Boolean): Boolean {
        return try {
            nativeToggleMod(modPath, enable)
        } catch (e: Throwable) {
            false
        }
    }

    fun injectLayer(packageName: String): Int {
        return try {
            nativeInjectLayer(packageName)
        } catch (e: Throwable) {
            -1
        }
    }

    fun getLayerStatus(): Int {
        return try {
            nativeGetLayerStatus()
        } catch (e: Throwable) {
            0
        }
    }

    fun isShizukuAvailable(): Boolean {
        return try {
            nativeIsShizukuAvailable()
        } catch (e: Throwable) {
            false
        }
    }
}
