package com.gimi.launcher.service;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import com.gimi.launcher.jni.GimiNativeBridge;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;

/**
 * GimiOverlayService — Floating In-Game Control Widget & Live Log Viewer.
 *
 * Displays a draggable, semi-transparent overlay widget on top of Genshin Impact.
 * Provides instant tactile feedback for:
 *   ⚡ Hot Reload (rescan .ini and textures)
 *   📸 Dump Toggle & File Counter (live count of /sdcard/GIMI/Dump/*.buf)
 *   📄 Quick Log Viewer (overlay logcat / gimi_render.log reader)
 */
public class GimiOverlayService extends Service {

    private WindowManager windowManager;
    private View overlayView;
    private View collapsedIcon;
    private LinearLayout expandedLayout;
    private LinearLayout logBoxView;

    private TextView dumpBadgeText;
    private TextView logContentText;
    private Button dumpToggleBtn;

    private boolean isDumpEnabled = false;
    private boolean isExpanded = true;
    private final Handler handler = new Handler(Looper.getMainLooper());
    private Runnable logUpdateRunnable;

    @Override
    public void onCreate() {
        super.onCreate();
        try {
            windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
            createOverlayUI();
            startLogUpdater();
        } catch (Throwable e) {
            e.printStackTrace();
            stopSelf();
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        if (logUpdateRunnable != null) {
            handler.removeCallbacks(logUpdateRunnable);
        }
        if (windowManager != null && overlayView != null) {
            try {
                windowManager.removeView(overlayView);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        super.onDestroy();
    }

    private void createOverlayUI() {
        int layoutFlag = Build.VERSION.SDK_INT >= Build.VERSION_CODES.O
            ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
            : WindowManager.LayoutParams.TYPE_PHONE;

        final WindowManager.LayoutParams params = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            layoutFlag,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            PixelFormat.TRANSLUCENT
        );

        params.gravity = Gravity.TOP | Gravity.START;
        params.x = 40;
        params.y = 150;

        // Container Layout
        final LinearLayout container = new LinearLayout(this);
        container.setOrientation(LinearLayout.VERTICAL);

        // 1. Collapsed Bubble Icon
        collapsedIcon = new TextView(this);
        ((TextView) collapsedIcon).setText("🎮 GIMI");
        ((TextView) collapsedIcon).setTextColor(Color.BLACK);
        ((TextView) collapsedIcon).setTextSize(13f);
        ((TextView) collapsedIcon).setTypeface(null, Typeface.BOLD);
        collapsedIcon.setPadding(24, 16, 24, 16);
        collapsedIcon.setBackground(createDrawable("#00E676", 24f));
        collapsedIcon.setVisibility(View.GONE);

        collapsedIcon.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                isExpanded = true;
                collapsedIcon.setVisibility(View.GONE);
                expandedLayout.setVisibility(View.VISIBLE);
            }
        });

        // 2. Expanded Bar Layout
        expandedLayout = new LinearLayout(this);
        expandedLayout.setOrientation(LinearLayout.VERTICAL);
        expandedLayout.setPadding(16, 12, 16, 12);
        expandedLayout.setBackground(createDrawable("#CC121212", 20f));

        // Header Row (Title + Drag handle + Minimize)
        LinearLayout headerRow = new LinearLayout(this);
        headerRow.setOrientation(LinearLayout.HORIZONTAL);
        headerRow.setGravity(Gravity.CENTER_VERTICAL);

        TextView title = new TextView(this);
        title.setText("🎮 GIMI Controls");
        title.setTextColor(Color.WHITE);
        title.setTextSize(13f);
        title.setTypeface(null, Typeface.BOLD);

        LinearLayout.LayoutParams titleParams = new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f);
        title.setLayoutParams(titleParams);

        TextView minimizeBtn = new TextView(this);
        minimizeBtn.setText(" ➖ ");
        minimizeBtn.setTextColor(Color.parseColor("#B0BEC5"));
        minimizeBtn.setTextSize(14f);
        minimizeBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                isExpanded = false;
                expandedLayout.setVisibility(View.GONE);
                if (logBoxView != null) logBoxView.setVisibility(View.GONE);
                collapsedIcon.setVisibility(View.VISIBLE);
            }
        });

        headerRow.addView(title);
        headerRow.addView(minimizeBtn);
        expandedLayout.addView(headerRow);

        // Buttons Row
        LinearLayout btnRow = new LinearLayout(this);
        btnRow.setOrientation(LinearLayout.HORIZONTAL);
        btnRow.setPadding(0, 8, 0, 4);

        Button reloadBtn = new Button(this);
        reloadBtn.setText("⚡ Reload");
        reloadBtn.setTextSize(11f);
        reloadBtn.setAllCaps(false);
        reloadBtn.setTextColor(Color.BLACK);
        reloadBtn.setTypeface(null, Typeface.BOLD);
        reloadBtn.setBackground(createDrawable("#00E676", 12f));
        LinearLayout.LayoutParams p1 = new LinearLayout.LayoutParams(0, 75, 1.0f);
        p1.setMargins(0, 0, 4, 0);
        reloadBtn.setLayoutParams(p1);
        reloadBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                GimiNativeBridge.reloadMods();
                Toast.makeText(GimiOverlayService.this, "⚡ Mods Recarregados!", Toast.LENGTH_SHORT).show();
            }
        });

        dumpToggleBtn = new Button(this);
        dumpToggleBtn.setText("📸 Dump OFF");
        dumpToggleBtn.setTextSize(11f);
        dumpToggleBtn.setAllCaps(false);
        dumpToggleBtn.setTextColor(Color.WHITE);
        dumpToggleBtn.setBackground(createDrawable("#37474F", 12f));
        LinearLayout.LayoutParams p2 = new LinearLayout.LayoutParams(0, 75, 1.0f);
        p2.setMargins(2, 0, 2, 0);
        dumpToggleBtn.setLayoutParams(p2);
        dumpToggleBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                isDumpEnabled = !isDumpEnabled;
                GimiNativeBridge.setDumpEnabled(isDumpEnabled);
                updateDumpButtonUI();
                Toast.makeText(
                    GimiOverlayService.this,
                    isDumpEnabled ? "📸 Dump ATIVADO → /sdcard/GIMI/Dump/" : "📸 Dump DESATIVADO",
                    Toast.LENGTH_SHORT
                ).show();
            }
        });

        Button logToggleBtn = new Button(this);
        logToggleBtn.setText("📄 Log");
        logToggleBtn.setTextSize(11f);
        logToggleBtn.setAllCaps(false);
        logToggleBtn.setTextColor(Color.BLACK);
        logToggleBtn.setTypeface(null, Typeface.BOLD);
        logToggleBtn.setBackground(createDrawable("#80D8FF", 12f));
        LinearLayout.LayoutParams p3 = new LinearLayout.LayoutParams(0, 75, 1.0f);
        p3.setMargins(4, 0, 0, 0);
        logToggleBtn.setLayoutParams(p3);
        logToggleBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (logBoxView.getVisibility() == View.GONE) {
                    logBoxView.setVisibility(View.VISIBLE);
                    refreshLogViewer();
                } else {
                    logBoxView.setVisibility(View.GONE);
                }
            }
        });

        btnRow.addView(reloadBtn);
        btnRow.addView(dumpToggleBtn);
        btnRow.addView(logToggleBtn);
        expandedLayout.addView(btnRow);

        // Status Badge Text
        dumpBadgeText = new TextView(this);
        dumpBadgeText.setText("Dumps: 0 arquivos em /sdcard/GIMI/Dump/");
        dumpBadgeText.setTextColor(Color.parseColor("#B0BEC5"));
        dumpBadgeText.setTextSize(10f);
        dumpBadgeText.setPadding(0, 4, 0, 0);
        expandedLayout.addView(dumpBadgeText);

        // 3. Floating Quick Log Viewer
        logBoxView = new LinearLayout(this);
        logBoxView.setOrientation(LinearLayout.VERTICAL);
        logBoxView.setPadding(12, 12, 12, 12);
        logBoxView.setBackground(createDrawable("#EE0A0A0A", 16f));
        logBoxView.setVisibility(View.GONE);

        LinearLayout.LayoutParams logBoxParams = new LinearLayout.LayoutParams(550, 300);
        logBoxParams.setMargins(0, 8, 0, 0);
        logBoxView.setLayoutParams(logBoxParams);

        TextView logTitle = new TextView(this);
        logTitle.setText("📄 Live Render Log (gimi_render.log)");
        logTitle.setTextColor(Color.parseColor("#00E676"));
        logTitle.setTextSize(11f);
        logTitle.setTypeface(null, Typeface.BOLD);
        logBoxView.addView(logTitle);

        ScrollView logScroll = new ScrollView(this);
        logScroll.setLayoutParams(new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
        ));

        logContentText = new TextView(this);
        logContentText.setText("Aguardando dados...");
        logContentText.setTextColor(Color.parseColor("#ECEFF1"));
        logContentText.setTextSize(9f);
        logContentText.setTypeface(Typeface.MONOSPACE);

        logScroll.addView(logContentText);
        logBoxView.addView(logScroll);

        container.addView(collapsedIcon);
        container.addView(expandedLayout);
        container.addView(logBoxView);

        // Drag Listener
        container.setOnTouchListener(new View.OnTouchListener() {
            private int initialX, initialY;
            private float initialTouchX, initialTouchY;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        initialX = params.x;
                        initialY = params.y;
                        initialTouchX = event.getRawX();
                        initialTouchY = event.getRawY();
                        return true;
                    case MotionEvent.ACTION_MOVE:
                        params.x = initialX + (int) (event.getRawX() - initialTouchX);
                        params.y = initialY + (int) (event.getRawY() - initialTouchY);
                        windowManager.updateViewLayout(overlayView, params);
                        return true;
                }
                return false;
            }
        });

        overlayView = container;
        windowManager.addView(overlayView, params);
        updateDumpCount();
    }

    private void updateDumpButtonUI() {
        if (dumpToggleBtn == null) return;
        if (isDumpEnabled) {
            dumpToggleBtn.setText("📸 Dump ON");
            dumpToggleBtn.setTextColor(Color.BLACK);
            dumpToggleBtn.setBackground(createDrawable("#FF9100", 12f));
        } else {
            dumpToggleBtn.setText("📸 Dump OFF");
            dumpToggleBtn.setTextColor(Color.WHITE);
            dumpToggleBtn.setBackground(createDrawable("#37474F", 12f));
        }
    }

    private void updateDumpCount() {
        try {
            File dumpDir = new File("/sdcard/GIMI/Dump");
            int count = 0;
            if (dumpDir.exists() && dumpDir.isDirectory()) {
                File[] files = dumpDir.listFiles();
                if (files != null) count = files.length;
            }
            if (dumpBadgeText != null) {
                dumpBadgeText.setText("Dumps: " + count + " arquivos em /sdcard/GIMI/Dump/");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void startLogUpdater() {
        logUpdateRunnable = new Runnable() {
            @Override
            public void run() {
                try {
                    updateDumpCount();
                    if (logBoxView != null && logBoxView.getVisibility() == View.VISIBLE) {
                        refreshLogViewer();
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                handler.postDelayed(this, 2000);
            }
        };
        handler.post(logUpdateRunnable);
    }

    private void refreshLogViewer() {
        try {
            File logFile = new File("/sdcard/GIMI/gimi_render.log");
            if (!logFile.exists()) {
                if (logContentText != null) logContentText.setText("Log file /sdcard/GIMI/gimi_render.log não criado ainda.");
                return;
            }

            BufferedReader reader = new BufferedReader(new FileReader(logFile));
            List<String> lines = new ArrayList<>();
            String line;
            while ((line = reader.readLine()) != null) {
                lines.add(line);
            }
            reader.close();

            int start = Math.max(0, lines.size() - 20);
            StringBuilder sb = new StringBuilder();
            for (int i = start; i < lines.size(); i++) {
                sb.append(lines.get(i)).append("\n");
            }

            if (logContentText != null) {
                logContentText.setText(sb.toString());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private GradientDrawable createDrawable(String colorHex, float cornerRadius) {
        GradientDrawable drawable = new GradientDrawable();
        drawable.setColor(Color.parseColor(colorHex));
        drawable.setCornerRadius(cornerRadius);
        return drawable;
    }
}
