package com.gimi.launcher.service;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.InputStreamReader;

/**
 * ShizukuManager — Direct Shizuku IPC & ADB Shell Command Executor.
 *
 * Interacts with Shizuku Service (UID 2000 shell context) without requiring PC connection.
 * Allows one-click grant of WRITE_SECURE_SETTINGS, execution of settings put global,
 * and Vulkan Layer injection natively.
 */
public class ShizukuManager {

    private static final String SHIZUKU_MANAGER_PACKAGE = "moe.shizuku.privileged.api";
    private static final Uri SHIZUKU_URI = Uri.parse("content://moe.shizuku.manager.shizuku");

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
            Bundle bundle = context.getContentResolver().call(SHIZUKU_URI, "getBinder", null, null);
            if (bundle != null) {
                IBinder binder = bundle.getBinder("binder");
                return binder != null && binder.isBinderAlive();
            }
        } catch (Throwable e) {
            e.printStackTrace();
        }
        return false;
    }

    public static boolean grantSecureSettingsViaShizuku(Context context) {
        String pkg = context.getPackageName();
        String cmd = "pm grant " + pkg + " android.permission.WRITE_SECURE_SETTINGS";
        return executeAdbCommand(context, cmd);
    }

    public static boolean injectVulkanLayerViaShizuku(Context context, String targetPackage, String libPath) {
        String cmd = "settings put global enable_gpu_debug_layers 1 && " +
                     "settings put global gpu_debug_app " + targetPackage + " && " +
                     "settings put global gpu_debug_layer_app " + context.getPackageName() + " && " +
                     "settings put global gpu_debug_layers libgimi_arm64.so && " +
                     "settings put global gpu_debug_layers_gles libgimi_arm64.so";
        return executeAdbCommand(context, cmd);
    }

    public static boolean executeAdbCommand(Context context, String command) {
        try {
            // Attempt 1: Execute via Shizuku Service binder / shizuku shell
            Process p = Runtime.getRuntime().exec(new String[]{"/data/local/tmp/shizuku", "-c", command});
            int exitCode = p.waitFor();
            if (exitCode == 0) return true;
        } catch (Throwable ignored) {}

        try {
            // Attempt 2: Direct sh execution if app has shell/root rights
            Process p = Runtime.getRuntime().exec(new String[]{"sh", "-c", command});
            int exitCode = p.waitFor();
            if (exitCode == 0) return true;
        } catch (Throwable ignored) {}

        return false;
    }
}
