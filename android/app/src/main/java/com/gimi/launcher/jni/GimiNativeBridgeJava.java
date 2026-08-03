package com.gimi.launcher.jni;

public class GimiNativeBridgeJava {
    static {
        try {
            System.loadLibrary("gimi_arm64");
        } catch (UnsatisfiedLinkError e) {
            e.printStackTrace();
        }
    }

    public static native ModInfoJava[] nativeScanMods(String path);
    public static native boolean nativeToggleMod(String modPath, boolean enable);
    public static native int nativeInjectLayer(String packageName);
    public static native int nativeGetLayerStatus();

    public static ModInfoJava[] scanMods(String path) {
        try {
            return nativeScanMods(path);
        } catch (Throwable e) {
            return new ModInfoJava[0];
        }
    }

    public static boolean toggleMod(String modPath, boolean enable) {
        try {
            return nativeToggleMod(modPath, enable);
        } catch (Throwable e) {
            return false;
        }
    }

    public static int injectLayer(String packageName) {
        try {
            return nativeInjectLayer(packageName);
        } catch (Throwable e) {
            return -1;
        }
    }

    public static int getLayerStatus() {
        try {
            return nativeGetLayerStatus();
        } catch (Throwable e) {
            return 0;
        }
    }
}
