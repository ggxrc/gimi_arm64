package com.gimi.launcher.jni;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class GimiNativeBridge {
    static {
        try {
            System.loadLibrary("gimi_arm64");
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    public static native ModInfo[] nativeScanMods(String path);
    public static native boolean nativeToggleMod(String modPath, boolean enable);
    public static native int nativeInjectLayer(String packageName);
    public static native int nativeGetLayerStatus();

    public static List<ModInfo> scanMods(String path) {
        try {
            ModInfo[] result = nativeScanMods(path != null ? path : "/sdcard/GIMI/Mods");
            return result != null ? Arrays.asList(result) : new ArrayList<ModInfo>();
        } catch (Throwable e) {
            e.printStackTrace();
            return new ArrayList<ModInfo>();
        }
    }

    public static boolean toggleMod(String modPath, boolean enable) {
        try {
            return nativeToggleMod(modPath, enable);
        } catch (Throwable e) {
            e.printStackTrace();
            return false;
        }
    }

    public static int injectLayer(String packageName) {
        try {
            return nativeInjectLayer(packageName);
        } catch (Throwable e) {
            e.printStackTrace();
            return -1;
        }
    }

    public static int getLayerStatus() {
        try {
            return nativeGetLayerStatus();
        } catch (Throwable e) {
            e.printStackTrace();
            return 0;
        }
    }
}
