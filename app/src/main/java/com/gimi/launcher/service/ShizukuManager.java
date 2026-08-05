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

    public static String binderError = "None";

    public static boolean checkAndForceBinder(Context context) {
        if (Shizuku.pingBinder()) {
            binderError = "SUCCESS";
            return true;
        }
        binderError = "Shizuku.pingBinder() is false. The provider is registered in manifest, but binder was not received yet. Try again in 2 seconds.";
        return false;
    }

    public static boolean isShizukuRunning(Context context) {
        try {
            return checkAndForceBinder(context);
        } catch (Throwable e) {
            e.printStackTrace();
        }
        return false;
    }

    public static String grantSecureSettingsViaShizuku(Context context) {
        checkAndForceBinder(context);
        String pkg = context.getPackageName();
        String[] cmd = new String[]{
            "pm", "grant", pkg, "android.permission.WRITE_SECURE_SETTINGS"
        };
        return executeAdbCommandWithResult(cmd);
    }

    public static String injectVulkanLayerViaShizuku(Context context, String targetPackage) {
        checkAndForceBinder(context);
        String res = executeAdbCommandWithResult(new String[]{"settings", "put", "global", "enable_gpu_debug_layers", "1"});
        if (!"SUCCESS".equals(res)) return res;
        
        res = executeAdbCommandWithResult(new String[]{"settings", "put", "global", "gpu_debug_app", targetPackage});
        if (!"SUCCESS".equals(res)) return res;
        
        res = executeAdbCommandWithResult(new String[]{"settings", "put", "global", "gpu_debug_layer_app", context.getPackageName()});
        if (!"SUCCESS".equals(res)) return res;
        
        res = executeAdbCommandWithResult(new String[]{"settings", "put", "global", "gpu_debug_layers", "VK_LAYER_GIMI_ARM64"});
        if (!"SUCCESS".equals(res)) return res;
        
        return executeAdbCommandWithResult(new String[]{"settings", "put", "global", "gpu_debug_layers_gles", "libgimi_arm64.so"});
    }

    public static String executeAdbCommandWithResult(String[] command) {
        try {
            if (!Shizuku.pingBinder()) {
                return "ERROR: Shizuku binder is not active or authorized. Please open Shizuku and authorize GIMI Launcher.\n" +
                       "Binder Diagnostics: " + binderError;
            }
            java.lang.reflect.Method method = Shizuku.class.getDeclaredMethod(
                "newProcess", String[].class, String[].class, String.class
            );
            method.setAccessible(true);
            java.lang.Process p = (java.lang.Process) method.invoke(null, command, null, null);
            int exitCode = p.waitFor();
            if (exitCode == 0) {
                return "SUCCESS";
            } else {
                return "ERROR: Process exited with code " + exitCode;
            }
        } catch (Throwable e) {
            return "EXCEPTION: " + e.getMessage() + "\n" + android.util.Log.getStackTraceString(e);
        }
    }
}
