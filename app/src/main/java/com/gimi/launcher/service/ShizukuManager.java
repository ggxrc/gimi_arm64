package com.gimi.launcher.service;

import android.content.Context;
import android.content.pm.PackageManager;
import rikka.shizuku.Shizuku;

/**
 * ShizukuManager — Direct Shizuku SDK API & ADB Shell Command Executor.
 *
 * Interacts with Shizuku Service (UID 2000 shell context) using Shizuku SDK IPC.
 * Allows one-click grant of WRITE_SECURE_SETTINGS and Vulkan Layer injection natively.
 */
public class ShizukuManager {

    public static boolean isShizukuInstalled(Context context) {
        try {
            context.getPackageManager().getPackageInfo("moe.shizuku.privileged.api", 0);
            return true;
        } catch (PackageManager.NameNotFoundException e) {
            try {
                context.getPackageManager().getPackageInfo("moe.shizuku.manager", 0);
                return true;
            } catch (PackageManager.NameNotFoundException e2) {
                return false;
            }
        }
    }

    public static boolean isShizukuRunning(Context context) {
        try {
            return Shizuku.pingBinder();
        } catch (Throwable e) {
            e.printStackTrace();
        }
        return false;
    }

    public static boolean grantSecureSettingsViaShizuku(Context context) {
        String pkg = context.getPackageName();
        String[] cmd = new String[]{
            "pm", "grant", pkg, "android.permission.WRITE_SECURE_SETTINGS"
        };
        return executeAdbCommand(cmd);
    }

    public static boolean injectVulkanLayerViaShizuku(Context context, String targetPackage) {
        executeAdbCommand(new String[]{"settings", "put", "global", "enable_gpu_debug_layers", "1"});
        executeAdbCommand(new String[]{"settings", "put", "global", "gpu_debug_app", targetPackage});
        executeAdbCommand(new String[]{"settings", "put", "global", "gpu_debug_layer_app", context.getPackageName()});
        executeAdbCommand(new String[]{"settings", "put", "global", "gpu_debug_layers", "VK_LAYER_GIMI_ARM64"});
        executeAdbCommand(new String[]{"settings", "put", "global", "gpu_debug_layers_gles", "libgimi_arm64.so"});
        return true;
    }

    public static boolean executeAdbCommand(String[] command) {
        try {
            if (!Shizuku.pingBinder()) return false;
            java.lang.reflect.Method method = Shizuku.class.getDeclaredMethod(
                "newProcess", String[].class, String[].class, String.class
            );
            method.setAccessible(true);
            java.lang.Process p = (java.lang.Process) method.invoke(null, command, null, null);
            int exitCode = p.waitFor();
            return exitCode == 0;
        } catch (Throwable e) {
            e.printStackTrace();
            return false;
        }
    }
}
