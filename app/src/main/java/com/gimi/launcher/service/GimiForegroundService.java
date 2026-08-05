package com.gimi.launcher.service;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.IBinder;
import android.widget.Toast;

import com.gimi.launcher.MainActivity;
import com.gimi.launcher.jni.GimiNativeBridge;

/**
 * GimiForegroundService — Persistent notification with hot-reload and dump controls.
 *
 * Shows a sticky notification in the Android notification bar while the Vulkan Layer
 * is active, providing two action buttons:
 *   ⚡ Recarregar Mods  — triggers nativeReloadMods() to refresh .ini and textures
 *   📸 Dump Hashes      — toggles nativeSetDumpEnabled() for hash extraction
 *
 * The notification remains active until the user explicitly reverts the layer
 * or stops the service from the app.
 */
public class GimiForegroundService extends Service {

    private static final String CHANNEL_ID = "gimi_control_channel";
    private static final int NOTIFICATION_ID = 1337;

    public static final String ACTION_HOT_RELOAD = "com.gimi.launcher.ACTION_HOT_RELOAD";
    public static final String ACTION_TOGGLE_DUMP = "com.gimi.launcher.ACTION_TOGGLE_DUMP";

    private boolean dumpEnabled = false;
    private BroadcastReceiver actionReceiver;

    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        registerActionReceiver();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Notification notification = buildNotification();
        startForeground(NOTIFICATION_ID, notification);
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        if (actionReceiver != null) {
            try {
                unregisterReceiver(actionReceiver);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        super.onDestroy();
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                "GIMI Layer Control",
                NotificationManager.IMPORTANCE_LOW
            );
            channel.setDescription("Controls for the active Vulkan Layer (hot-reload, dump hashes)");
            channel.setShowBadge(false);

            NotificationManager nm = getSystemService(NotificationManager.class);
            if (nm != null) {
                nm.createNotificationChannel(channel);
            }
        }
    }

    private Notification buildNotification() {
        // Open app intent
        Intent openIntent = new Intent(this, MainActivity.class);
        openIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        PendingIntent openPending = PendingIntent.getActivity(
            this, 0, openIntent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );

        // Hot-reload action
        Intent reloadIntent = new Intent(ACTION_HOT_RELOAD);
        PendingIntent reloadPending = PendingIntent.getBroadcast(
            this, 1, reloadIntent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );

        // Dump toggle action
        Intent dumpIntent = new Intent(ACTION_TOGGLE_DUMP);
        PendingIntent dumpPending = PendingIntent.getBroadcast(
            this, 2, dumpIntent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );

        String dumpLabel = dumpEnabled ? "\uD83D\uDCF8 Dump ON" : "\uD83D\uDCF8 Dump OFF";

        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder = new Notification.Builder(this, CHANNEL_ID);
        } else {
            builder = new Notification.Builder(this);
        }

        builder.setSmallIcon(android.R.drawable.ic_menu_manage)
               .setContentTitle("GIMI ARM64 — Vulkan Layer Active")
               .setContentText("Layer injetada. Use os botões para controlar mods.")
               .setContentIntent(openPending)
               .setOngoing(true)
               .addAction(new Notification.Action.Builder(
                   null, "⚡ Recarregar Mods", reloadPending
               ).build())
               .addAction(new Notification.Action.Builder(
                   null, dumpLabel, dumpPending
               ).build());

        return builder.build();
    }

    private void registerActionReceiver() {
        actionReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (ACTION_HOT_RELOAD.equals(action)) {
                    GimiNativeBridge.reloadMods();
                    Toast.makeText(context, "⚡ Mods recarregados!", Toast.LENGTH_SHORT).show();
                } else if (ACTION_TOGGLE_DUMP.equals(action)) {
                    dumpEnabled = !dumpEnabled;
                    GimiNativeBridge.setDumpEnabled(dumpEnabled);
                    Toast.makeText(context,
                        dumpEnabled ? "\uD83D\uDCF8 Dump ATIVADO → /sdcard/GIMI/Dump/" : "\uD83D\uDCF8 Dump DESATIVADO",
                        Toast.LENGTH_SHORT
                    ).show();
                    // Update notification to reflect new dump state
                    NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
                    if (nm != null) {
                        nm.notify(NOTIFICATION_ID, buildNotification());
                    }
                }
            }
        };

        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_HOT_RELOAD);
        filter.addAction(ACTION_TOGGLE_DUMP);

        if (Build.VERSION.SDK_INT >= 33) {
            registerReceiver(actionReceiver, filter, Context.RECEIVER_NOT_EXPORTED);
        } else {
            registerReceiver(actionReceiver, filter);
        }
    }
}
