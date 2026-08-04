package com.gimi.launcher;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.Typeface;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.*;

import rikka.shizuku.Shizuku;

import com.gimi.launcher.jni.GimiNativeBridgeJava;
import com.gimi.launcher.jni.ModInfoJava;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {
    private String selectedPackage = "com.miHoYo.GenshinImpact";
    private String modsDirectoryPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/GIMI/Mods";

    private LinearLayout contentContainer;
    private TextView statusOutputView;
    private LinearLayout logsContainer;
    private List<String> logMessages = new ArrayList<>();

    // Track which view is currently displayed for onResume refresh
    private int currentView = 0; // 0=dashboard, 1=modmanager, 2=settings

    private final Shizuku.OnBinderReceivedListener BINDER_RECEIVED_LISTENER = () -> {
        runOnUiThread(() -> {
            logMessages.add("[INFO] Shizuku binder connected ✔");
            refreshCurrentView();
        });
    };

    private final Shizuku.OnRequestPermissionResultListener PERMISSION_RESULT_LISTENER = (requestCode, grantResult) -> {
        runOnUiThread(() -> {
            boolean granted = grantResult == PackageManager.PERMISSION_GRANTED;
            logMessages.add("[SHIZUKU] Permission result: " + (granted ? "GRANTED ✔" : "DENIED ❌"));
            refreshCurrentView();
        });
    };

    private final android.content.BroadcastReceiver shizukuBinderReceiver = new android.content.BroadcastReceiver() {
        @Override
        public void onReceive(android.content.Context context, Intent intent) {
            try {
                if (intent != null) {
                    intent.setExtrasClassLoader(moe.shizuku.api.BinderContainer.class.getClassLoader());
                    moe.shizuku.api.BinderContainer container = intent.getParcelableExtra("moe.shizuku.privileged.api.intent.extra.BINDER");
                    if (container != null && container.binder != null) {
                        Shizuku.onBinderReceived(container.binder, getPackageName());
                        logMessages.add("[SHIZUKU] Binder received via broadcast ✔");
                        runOnUiThread(() -> refreshCurrentView());
                    }
                }
            } catch (Throwable e) {
                logMessages.add("[WARN] Shizuku broadcast error: " + e.getMessage());
            }
        }
    };

    private void registerShizukuReceiverSafe() {
        try {
            android.content.IntentFilter filter = new android.content.IntentFilter("moe.shizuku.api.action.BINDER_RECEIVED");
            if (Build.VERSION.SDK_INT >= 33) {
                registerReceiver(shizukuBinderReceiver, filter, Context.RECEIVER_EXPORTED);
            } else {
                registerReceiver(shizukuBinderReceiver, filter);
            }
            logMessages.add("[INFO] Registered Shizuku broadcast receiver safely ✔");
        } catch (Throwable e) {
            logMessages.add("[WARN] Could not register Shizuku receiver: " + e.getMessage());
        }
    }

    private void tryDirectShizukuBinderRequest() {
        if (Shizuku.pingBinder()) return;
        try {
            Uri uri = Uri.parse("content://moe.shizuku.manager.shizuku");
            Bundle reply = getContentResolver().call(uri, "getBinder", null, null);
            if (reply != null) {
                reply.setClassLoader(moe.shizuku.api.BinderContainer.class.getClassLoader());
                moe.shizuku.api.BinderContainer container = reply.getParcelable("moe.shizuku.privileged.api.intent.extra.BINDER");
                if (container != null && container.binder != null) {
                    Shizuku.onBinderReceived(container.binder, getPackageName());
                    logMessages.add("[SHIZUKU] Direct binder query successful ✔");
                }
            }
        } catch (Throwable e) {
            logMessages.add("[WARN] Direct Shizuku binder query: " + e.getMessage());
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        logMessages.add("[INFO] GIMI Launcher initialized");
        logMessages.add("[INFO] Native library libgimi_arm64.so loaded");

        // ─── Attach Shizuku binder and listeners for real-time permission updates ────
        try {
            registerShizukuReceiverSafe();
            tryDirectShizukuBinderRequest();
            Shizuku.addBinderReceivedListener(BINDER_RECEIVED_LISTENER);
            Shizuku.addRequestPermissionResultListener(PERMISSION_RESULT_LISTENER);
            logMessages.add("[INFO] Initialized Shizuku binder for process");
        } catch (Throwable e) {
            logMessages.add("[WARN] Could not initialize Shizuku binder: " + e.getMessage());
        }

        // ─── Task 1: Check and request MANAGE_EXTERNAL_STORAGE at startup ────
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                logMessages.add("[WARN] MANAGE_EXTERNAL_STORAGE not granted — requesting...");
                requestAllFilesAccess();
            } else {
                logMessages.add("[INFO] MANAGE_EXTERNAL_STORAGE granted ✔");
            }
        }

        // ─── Task 3: Read layer status from Java Settings.Global API ─────────
        boolean layerActive = isLayerActiveFromSettings();
        logMessages.add("[INFO] Vulkan layer status (Settings.Global): " + (layerActive ? "ACTIVE" : "INACTIVE"));

        LinearLayout mainLayout = new LinearLayout(this);
        mainLayout.setOrientation(LinearLayout.VERTICAL);
        mainLayout.setBackgroundColor(Color.parseColor("#121212"));

        // Header Title
        TextView header = new TextView(this);
        header.setText("GIMI Launcher (ARM64)");
        header.setTextSize(22);
        header.setTextColor(Color.WHITE);
        header.setTypeface(null, Typeface.BOLD);
        header.setPadding(32, 32, 32, 16);
        header.setGravity(Gravity.CENTER_HORIZONTAL);
        mainLayout.addView(header);

        // Content Area
        contentContainer = new LinearLayout(this);
        contentContainer.setOrientation(LinearLayout.VERTICAL);
        LinearLayout.LayoutParams contentParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, 0, 1.0f);
        contentContainer.setLayoutParams(contentParams);
        contentContainer.setPadding(24, 16, 24, 16);
        mainLayout.addView(contentContainer);

        // Bottom Navigation Bar
        LinearLayout bottomBar = new LinearLayout(this);
        bottomBar.setOrientation(LinearLayout.HORIZONTAL);
        bottomBar.setBackgroundColor(Color.parseColor("#1E1E1E"));
        bottomBar.setPadding(16, 16, 16, 16);

        Button tabDashboard = createNavButton("⚡ Dashboard");
        Button tabModManager = createNavButton("📦 Mod Manager");
        Button tabSettings = createNavButton("⚙️ Settings");

        tabDashboard.setOnClickListener(v -> { currentView = 0; showDashboardView(); });
        tabModManager.setOnClickListener(v -> { currentView = 1; showModManagerView(); });
        tabSettings.setOnClickListener(v -> { currentView = 2; showSettingsView(); });

        bottomBar.addView(tabDashboard);
        bottomBar.addView(tabModManager);
        bottomBar.addView(tabSettings);
        mainLayout.addView(bottomBar);

        setContentView(mainLayout);
        showDashboardView();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            unregisterReceiver(shizukuBinderReceiver);
        } catch (Throwable e) {}
        try {
            Shizuku.removeBinderReceivedListener(BINDER_RECEIVED_LISTENER);
            Shizuku.removeRequestPermissionResultListener(PERMISSION_RESULT_LISTENER);
        } catch (Throwable e) {}
    }

    private void refreshCurrentView() {
        if (contentContainer != null) {
            switch (currentView) {
                case 0: showDashboardView(); break;
                case 1: showModManagerView(); break;
                case 2: showSettingsView(); break;
            }
        }
    }

    // ─── Task 1: Re-check permissions and refresh on return from Settings ────
    @Override
    protected void onResume() {
        super.onResume();
        if (contentContainer != null) {
            // Refresh the current view to pick up permission changes
            switch (currentView) {
                case 0: showDashboardView(); break;
                case 1: showModManagerView(); break;
                case 2: showSettingsView(); break;
            }
        }
    }

    // ─── Task 1: Request All Files Access (MANAGE_EXTERNAL_STORAGE) ──────────
    private void requestAllFilesAccess() {
        try {
            Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
            intent.setData(Uri.parse("package:" + getPackageName()));
            startActivity(intent);
        } catch (Exception e) {
            // Fallback for devices that don't support the package-specific intent
            try {
                Intent intent = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                startActivity(intent);
            } catch (Exception e2) {
                logMessages.add("[ERROR] Could not open All Files Access settings: " + e2.getMessage());
                Toast.makeText(this,
                    "Please grant 'All Files Access' manually in Settings > Apps > GIMI Launcher",
                    Toast.LENGTH_LONG).show();
            }
        }
    }

    // ─── Task 2: Inject Vulkan Layer via Java Settings.Global API ─────────────
    private boolean injectVulkanLayer(String packageName) {
        // Method 1: Shizuku (Rootless ADB)
        if (checkShizukuPermission()) {
            try {
                String[] cmds = {
                    "settings put global enable_gpu_debug_layers 1",
                    "settings put global gpu_debug_app " + packageName,
                    "settings put global gpu_debug_layer_app " + getPackageName(),
                    "settings put global gpu_debug_layers libgimi_arm64.so"
                };
                for (String cmd : cmds) {
                    Process p = Shizuku.newProcess(new String[]{"sh", "-c", cmd}, null, null);
                    p.waitFor();
                }
                logMessages.add("[INJECT] Layer injection via Shizuku succeeded for " + packageName);
                return true;
            } catch (Exception e) {
                logMessages.add("[ERROR] Shizuku execution failed: " + e.getMessage());
            }
        }

        // Method 2: Direct Settings API (Requires WRITE_SECURE_SETTINGS manually granted)
        try {
            android.content.ContentResolver resolver = getContentResolver();

            // Enable GPU debug layers
            Settings.Global.putInt(resolver, "enable_gpu_debug_layers", 1);

            // Set target game package
            Settings.Global.putString(resolver, "gpu_debug_app", packageName);

            // Set the layer provider package (this app)
            Settings.Global.putString(resolver, "gpu_debug_layer_app", getPackageName());

            // Set the Vulkan layer name
            Settings.Global.putString(resolver, "gpu_debug_layers", "libgimi_arm64.so");

            logMessages.add("[INJECT] Layer injection via Settings.Global succeeded for " + packageName);
            return true;
        } catch (SecurityException se) {
            logMessages.add("[ERROR] SecurityException — WRITE_SECURE_SETTINGS not granted");
            Toast.makeText(this,
                "Permission denied. Run via ADB:\nadb shell pm grant com.gimi.launcher android.permission.WRITE_SECURE_SETTINGS",
                Toast.LENGTH_LONG).show();
            return false;
        } catch (Exception e) {
            logMessages.add("[ERROR] Layer injection failed: " + e.getMessage());
            return false;
        }
    }

    // ─── Task 3: Check layer status from Java Settings.Global API ────────────
    private boolean isLayerActiveFromSettings() {
        if (checkShizukuPermission()) {
            try {
                Process p = Shizuku.newProcess(new String[]{"sh", "-c", "settings get global gpu_debug_layers"}, null, null);
                java.io.BufferedReader reader = new java.io.BufferedReader(new java.io.InputStreamReader(p.getInputStream()));
                String line = reader.readLine();
                p.waitFor();
                if (line != null && line.contains("libgimi_arm64")) {
                    return true;
                }
            } catch (Throwable e) {
                // Fallback to ContentResolver
            }
        }
        try {
            String layers = Settings.Global.getString(getContentResolver(), "gpu_debug_layers");
            return layers != null && layers.contains("libgimi_arm64");
        } catch (Throwable e) {
            return false;
        }
    }

    // ─── Task 3: Check if WRITE_SECURE_SETTINGS is granted ──────────────────
    private boolean hasSecureSettingsPermission() {
        try {
            return checkCallingOrSelfPermission("android.permission.WRITE_SECURE_SETTINGS")
                    == PackageManager.PERMISSION_GRANTED;
        } catch (Throwable e) {
            return false;
        }
    }

    // ─── Shizuku Integration ─────────────────────────────────────────────────
    private boolean isShizukuAvailable() {
        try {
            return Shizuku.pingBinder();
        } catch (Throwable e) {
            return false;
        }
    }

    private boolean checkShizukuPermission() {
        try {
            if (!isShizukuAvailable()) return false;
            if (Shizuku.isPreV11() || Shizuku.getVersion() < 11) return false;
            return Shizuku.checkSelfPermission() == PackageManager.PERMISSION_GRANTED;
        } catch (Throwable e) {
            return false;
        }
    }

    private void requestShizukuPermission() {
        try {
            if (isShizukuAvailable()) {
                Shizuku.requestPermission(0);
            }
        } catch (Throwable e) {
            logMessages.add("[ERROR] Failed to request Shizuku permission: " + e.getMessage());
        }
    }

    private boolean grantSecureSettingsViaShizuku() {
        if (!checkShizukuPermission()) {
            Toast.makeText(this, "Shizuku not authorized yet", Toast.LENGTH_SHORT).show();
            return false;
        }
        try {
            String cmd = "pm grant " + getPackageName() + " android.permission.WRITE_SECURE_SETTINGS";
            Process p = Shizuku.newProcess(new String[]{"sh", "-c", cmd}, null, null);
            int exitCode = p.waitFor();
            if (exitCode == 0) {
                logMessages.add("[SHIZUKU] Granted WRITE_SECURE_SETTINGS via Shizuku ✔");
                Toast.makeText(this, "WRITE_SECURE_SETTINGS granted via Shizuku! ✔", Toast.LENGTH_LONG).show();
                refreshCurrentView();
                return true;
            } else {
                logMessages.add("[ERROR] Shizuku pm grant returned exit code: " + exitCode);
                Toast.makeText(this, "Shizuku failed to grant permission (exit code " + exitCode + ")", Toast.LENGTH_SHORT).show();
                return false;
            }
        } catch (Throwable e) {
            logMessages.add("[ERROR] Shizuku grant exception: " + e.getMessage());
            Toast.makeText(this, "Error: " + e.getMessage(), Toast.LENGTH_SHORT).show();
            return false;
        }
    }

    private Button createNavButton(String label) {
        Button btn = new Button(this);
        btn.setText(label);
        btn.setTextColor(Color.WHITE);
        btn.setTextSize(12);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                0, ViewGroup.LayoutParams.WRAP_CONTENT, 1.0f);
        params.setMargins(4, 0, 4, 0);
        btn.setLayoutParams(params);
        return btn;
    }

    // ─── 1. Dashboard Screen ───────────────────────────────────────────────────
    private void showDashboardView() {
        contentContainer.removeAllViews();
        ScrollView scrollView = new ScrollView(this);
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);

        // ─── Task 1: Storage Permission Status Card ──────────────────────────
        boolean hasStorageAccess = true;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            hasStorageAccess = Environment.isExternalStorageManager();
        }

        if (!hasStorageAccess) {
            CardViewBuilder storageCard = new CardViewBuilder(this, "#E65100");
            storageCard.addTitle("⚠️ Storage Permission Required");
            storageCard.addSubtitle("MANAGE_EXTERNAL_STORAGE not granted.\nMods cannot be scanned from " + modsDirectoryPath);

            Button grantBtn = new Button(this);
            grantBtn.setText("GRANT ALL FILES ACCESS");
            grantBtn.setBackgroundColor(Color.parseColor("#FF6D00"));
            grantBtn.setTextColor(Color.WHITE);
            grantBtn.setTypeface(null, Typeface.BOLD);
            grantBtn.setOnClickListener(v -> requestAllFilesAccess());
            storageCard.addCustomView(grantBtn);
            layout.addView(storageCard.build());
        }

        // ─── Task 3: Vulkan Layer Status (read from Settings.Global) ─────────
        boolean isActive = isLayerActiveFromSettings();
        CardViewBuilder layerCard = new CardViewBuilder(this, isActive ? "#1B5E20" : "#B71C1C");
        layerCard.addTitle("Vulkan Layer Status");
        layerCard.addSubtitle(isActive ? "🟢 ACTIVE (libgimi_arm64.so injected)" : "🔴 INACTIVE (Layer not configured)");

        // Show actual Settings.Global values for debugging
        try {
            String debugApp = Settings.Global.getString(getContentResolver(), "gpu_debug_app");
            String debugLayers = Settings.Global.getString(getContentResolver(), "gpu_debug_layers");
            if (debugLayers != null && !debugLayers.isEmpty()) {
                layerCard.addDetail("gpu_debug_layers: " + debugLayers);
            }
            if (debugApp != null && !debugApp.isEmpty()) {
                layerCard.addDetail("gpu_debug_app: " + debugApp);
            }
        } catch (Exception e) {
            // Ignore — reading Settings.Global doesn't require special permissions
        }
        layout.addView(layerCard.build());

        // ─── Task 3: WRITE_SECURE_SETTINGS / Shizuku Permission Status ───────
        boolean hasSecureSettings = hasSecureSettingsPermission();
        boolean hasShizukuAuth = checkShizukuPermission();
        boolean isShizukuRunning = isShizukuAvailable();
        boolean canInject = hasSecureSettings || hasShizukuAuth;

        CardViewBuilder permCard = new CardViewBuilder(this, canInject ? "#0D47A1" : "#4A148C");
        permCard.addTitle("Injection Permission");

        if (hasSecureSettings) {
            permCard.addSubtitle("⚡ WRITE_SECURE_SETTINGS Granted (System Ready)");
        } else if (hasShizukuAuth) {
            permCard.addSubtitle("⚡ Shizuku Authorized — Click below to grant system permission:");
            Button grantSecBtn = new Button(this);
            grantSecBtn.setText("GRANT SECURE SETTINGS VIA SHIZUKU");
            grantSecBtn.setBackgroundColor(Color.parseColor("#2E7D32"));
            grantSecBtn.setTextColor(Color.WHITE);
            grantSecBtn.setTypeface(null, Typeface.BOLD);
            grantSecBtn.setOnClickListener(v -> grantSecureSettingsViaShizuku());
            permCard.addCustomView(grantSecBtn);
        } else if (isShizukuRunning) {
            permCard.addSubtitle("⚠️ Shizuku Found — Authorization Required");
            Button shizBtn = new Button(this);
            shizBtn.setText("AUTHORIZE SHIZUKU");
            shizBtn.setBackgroundColor(Color.parseColor("#6200EE"));
            shizBtn.setTextColor(Color.WHITE);
            shizBtn.setOnClickListener(v -> requestShizukuPermission());
            permCard.addCustomView(shizBtn);
        } else {
            permCard.addSubtitle("⚠️ Permission Not Granted — Start Shizuku or use ADB");
            permCard.addDetail("Run via ADB:");
            permCard.addDetail("adb shell pm grant " + getPackageName() + " android.permission.WRITE_SECURE_SETTINGS");

            Button retryShizukuBtn = new Button(this);
            retryShizukuBtn.setText("CONNECT / RE-DETECT SHIZUKU");
            retryShizukuBtn.setBackgroundColor(Color.parseColor("#3700B3"));
            retryShizukuBtn.setTextColor(Color.WHITE);
            retryShizukuBtn.setOnClickListener(v -> {
                try {
                    registerShizukuReceiverSafe();
                    tryDirectShizukuBinderRequest();
                    logMessages.add("[SHIZUKU] Attempting binder connection...");
                    if (Shizuku.pingBinder()) {
                        Toast.makeText(this, "Shizuku connected! ✔", Toast.LENGTH_SHORT).show();
                    } else {
                        Toast.makeText(this, "Shizuku binder not available yet. Ensure Shizuku app is running.", Toast.LENGTH_LONG).show();
                    }
                    refreshCurrentView();
                } catch (Throwable t) {
                    Toast.makeText(this, "Could not connect to Shizuku: " + t.getMessage(), Toast.LENGTH_SHORT).show();
                }
            });
            permCard.addCustomView(retryShizukuBtn);
        }
        layout.addView(permCard.build());

        // Card 3: Game Package Selector
        CardViewBuilder pkgCard = new CardViewBuilder(this, "#212121");
        pkgCard.addTitle("Target Game Variant");

        RadioGroup radioGroup = new RadioGroup(this);
        radioGroup.setOrientation(LinearLayout.VERTICAL);
        String[][] packages = {
                {"Play Store", "com.miHoYo.GenshinImpact"},
                {"Galaxy Store", "com.miHoYo.GI.samsung"},
                {"China Server", "com.yuanshen.site"},
                {"Bilibili", "com.miHoYo.ys.bilibili"}
        };

        for (int i = 0; i < packages.length; i++) {
            String[] pkg = packages[i];
            RadioButton rb = new RadioButton(this);
            rb.setId(View.generateViewId());
            rb.setText(pkg[0] + " (" + pkg[1] + ")");
            rb.setTextColor(Color.WHITE);
            if (pkg[1].equals(selectedPackage)) {
                rb.setChecked(true);
            }
            radioGroup.addView(rb);
        }

        radioGroup.setOnCheckedChangeListener((group, checkedId) -> {
            RadioButton checkedRb = group.findViewById(checkedId);
            if (checkedRb != null) {
                String text = checkedRb.getText().toString();
                for (String[] pkg : packages) {
                    if (text.contains(pkg[1])) {
                        selectedPackage = pkg[1];
                        break;
                    }
                }
            }
        });
        pkgCard.addCustomView(radioGroup);
        layout.addView(pkgCard.build());

        // ─── Task 2: Inject Button — uses Java Settings.Global API directly ──
        Button injectBtn = new Button(this);
        injectBtn.setText(canInject ? "INJECT VULKAN LAYER & LAUNCH" : "INJECT VULKAN LAYER (PERMISSION NEEDED)");
        injectBtn.setBackgroundColor(Color.parseColor(canInject ? "#6200EE" : "#424242"));
        injectBtn.setTextColor(Color.WHITE);
        injectBtn.setTextSize(14);
        injectBtn.setTypeface(null, Typeface.BOLD);
        injectBtn.setPadding(16, 24, 16, 24);
        injectBtn.setOnClickListener(v -> {
            boolean success = injectVulkanLayer(selectedPackage);
            String msg;
            if (success) {
                msg = "Layer injected successfully for " + selectedPackage;
                // Launch the game after successful injection
                try {
                    Intent launchIntent = getPackageManager().getLaunchIntentForPackage(selectedPackage);
                    if (launchIntent != null) {
                        startActivity(launchIntent);
                        logMessages.add("[LAUNCH] Starting " + selectedPackage);
                    } else {
                        logMessages.add("[WARN] Game package not installed: " + selectedPackage);
                        Toast.makeText(this, "Game not installed: " + selectedPackage, Toast.LENGTH_SHORT).show();
                    }
                } catch (Exception e) {
                    logMessages.add("[ERROR] Failed to launch game: " + e.getMessage());
                }
            } else {
                msg = "Injection failed — check permissions";
            }
            if (statusOutputView != null) {
                statusOutputView.setText("System Output: " + msg);
            }
            logMessages.add("[INJECT] " + msg);
            // Refresh dashboard to update layer status card
            showDashboardView();
        });
        layout.addView(injectBtn);

        // Status Banner
        CardViewBuilder statusCard = new CardViewBuilder(this, "#333333");
        statusOutputView = new TextView(this);
        statusOutputView.setText("System Output: Ready");
        statusOutputView.setTextColor(Color.YELLOW);
        statusOutputView.setTextSize(13);
        statusCard.addCustomView(statusOutputView);
        layout.addView(statusCard.build());

        scrollView.addView(layout);
        contentContainer.addView(scrollView);
    }

    // ─── Java Mod Scanner & Toggle (Bypasses SELinux restrictions & handles DISABLED_) ─
    public static ModInfoJava[] scanModsJava(String path) {
        List<ModInfoJava> list = new ArrayList<>();
        try {
            File dir = new File(path);
            if (dir.exists() && dir.isDirectory()) {
                File[] files = dir.listFiles();
                if (files != null) {
                    for (File f : files) {
                        if (f.isDirectory()) {
                            String name = f.getName();
                            boolean isDisabled = name.startsWith("DISABLED_") || name.startsWith("disabled_") || name.endsWith(".disabled");
                            
                            String cleanName = name;
                            if (name.startsWith("DISABLED_") || name.startsWith("disabled_")) {
                                cleanName = name.substring(9);
                            } else if (name.endsWith(".disabled")) {
                                cleanName = name.substring(0, name.length() - 9);
                            }

                            int iniCount = 0;
                            File[] inner = f.listFiles();
                            if (inner != null) {
                                for (File in : inner) {
                                    if (in.isFile() && in.getName().toLowerCase().endsWith(".ini")) {
                                        iniCount++;
                                    }
                                }
                            }

                            list.add(new ModInfoJava(cleanName, f.getAbsolutePath(), !isDisabled, iniCount, 0));
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (!list.isEmpty()) {
            return list.toArray(new ModInfoJava[0]);
        }
        return GimiNativeBridgeJava.scanMods(path);
    }

    public static boolean toggleModJava(String modPath, boolean enable) {
        try {
            File f = new File(modPath);
            if (!f.exists() || !f.isDirectory()) return false;

            String name = f.getName();
            File parent = f.getParentFile();
            if (parent == null) return false;

            File target;
            if (!enable) {
                // Disable mod: prepend DISABLED_
                if (!name.startsWith("DISABLED_") && !name.startsWith("disabled_")) {
                    String cleanName = name.endsWith(".disabled") ? name.substring(0, name.length() - 9) : name;
                    target = new File(parent, "DISABLED_" + cleanName);
                } else {
                    return true;
                }
            } else {
                // Enable mod: remove DISABLED_ prefix or .disabled suffix
                String cleanName = name;
                if (name.startsWith("DISABLED_") || name.startsWith("disabled_")) {
                    cleanName = name.substring(9);
                } else if (name.endsWith(".disabled")) {
                    cleanName = name.substring(0, name.length() - 9);
                }
                target = new File(parent, cleanName);
            }
            boolean success = f.renameTo(target);
            GimiNativeBridgeJava.toggleMod(modPath, enable);
            return success;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    // ─── 2. Mod Manager Screen ─────────────────────────────────────────────────
    private void showModManagerView() {
        contentContainer.removeAllViews();
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);

        TextView title = new TextView(this);
        title.setText("Mod Manager");
        title.setTextSize(18);
        title.setTextColor(Color.WHITE);
        title.setTypeface(null, Typeface.BOLD);
        layout.addView(title);

        // ─── Task 1: Show storage permission warning if not granted ──────────
        boolean hasStorageAccess = true;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            hasStorageAccess = Environment.isExternalStorageManager();
        }

        if (!hasStorageAccess) {
            CardViewBuilder storageWarning = new CardViewBuilder(this, "#E65100");
            storageWarning.addTitle("⚠️ Storage Access Required");
            storageWarning.addSubtitle("Grant 'All Files Access' to scan mods.");

            Button grantBtn = new Button(this);
            grantBtn.setText("GRANT ACCESS");
            grantBtn.setBackgroundColor(Color.parseColor("#FF6D00"));
            grantBtn.setTextColor(Color.WHITE);
            grantBtn.setOnClickListener(v -> requestAllFilesAccess());
            storageWarning.addCustomView(grantBtn);
            layout.addView(storageWarning.build());
        }

        // Search Input
        EditText searchInput = new EditText(this);
        searchInput.setHint("Search mods...");
        searchInput.setHintTextColor(Color.GRAY);
        searchInput.setTextColor(Color.WHITE);
        searchInput.setBackgroundColor(Color.parseColor("#2C2C2C"));
        searchInput.setPadding(16, 16, 16, 16);
        layout.addView(searchInput);

        ModInfoJava[] mods = scanModsJava(modsDirectoryPath);

        int activeCount = 0;
        for (ModInfoJava m : mods) {
            if (m.isEnabled) activeCount++;
        }

        TextView counterView = new TextView(this);
        counterView.setText("Total Mods Found: " + mods.length + " | Active: " + activeCount);
        counterView.setTextColor(Color.CYAN);
        counterView.setPadding(0, 16, 0, 16);
        layout.addView(counterView);

        // Mods List Container
        ScrollView scrollView = new ScrollView(this);
        LinearLayout modsListLayout = new LinearLayout(this);
        modsListLayout.setOrientation(LinearLayout.VERTICAL);

        if (mods.length == 0) {
            TextView emptyText = new TextView(this);
            if (!hasStorageAccess) {
                emptyText.setText("Storage permission not granted.\nPlease grant 'All Files Access' above, then return to this screen.");
            } else {
                emptyText.setText("No mods found in " + modsDirectoryPath + "\nAdd mods to path or change in Settings.");
            }
            emptyText.setTextColor(Color.GRAY);
            emptyText.setPadding(16, 32, 16, 32);
            modsListLayout.addView(emptyText);
        } else {
            for (ModInfoJava mod : mods) {
                LinearLayout modCard = new LinearLayout(this);
                modCard.setOrientation(LinearLayout.HORIZONTAL);
                modCard.setBackgroundColor(Color.parseColor(mod.isEnabled ? "#1E2A1E" : "#2A2A2A"));
                modCard.setPadding(16, 16, 16, 16);

                LinearLayout infoLayout = new LinearLayout(this);
                infoLayout.setOrientation(LinearLayout.VERTICAL);
                LinearLayout.LayoutParams infoParams = new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1.0f);
                infoLayout.setLayoutParams(infoParams);

                TextView modName = new TextView(this);
                modName.setText(mod.name);
                modName.setTextColor(Color.WHITE);
                modName.setTypeface(null, Typeface.BOLD);
                infoLayout.addView(modName);

                TextView modDetails = new TextView(this);
                modDetails.setText((mod.isEnabled ? "🟢 ENABLED" : "🔴 DISABLED") + " | 📄 " + mod.iniCount + " .ini file(s)");
                modDetails.setTextColor(mod.isEnabled ? Color.GREEN : Color.GRAY);
                modDetails.setTextSize(11);
                infoLayout.addView(modDetails);

                modCard.addView(infoLayout);

                Switch toggle = new Switch(this);
                toggle.setChecked(mod.isEnabled);
                toggle.setOnCheckedChangeListener((buttonView, isChecked) -> {
                    boolean ok = toggleModJava(mod.path, isChecked);
                    if (ok) {
                        logMessages.add("[MOD] Toggled " + mod.name + " -> " + (isChecked ? "ENABLED" : "DISABLED (DISABLED_)"));
                    } else {
                        Toast.makeText(MainActivity.this, "Failed to rename mod folder", Toast.LENGTH_SHORT).show();
                    }
                    showModManagerView();
                });
                modCard.addView(toggle);

                modsListLayout.addView(modCard);
                View spacer = new View(this);
                spacer.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 8));
                modsListLayout.addView(spacer);
            }
        }

        scrollView.addView(modsListLayout);
        layout.addView(scrollView);
        contentContainer.addView(layout);
    }

    // ─── 3. Settings Screen ────────────────────────────────────────────────────
    private void showSettingsView() {
        contentContainer.removeAllViews();
        ScrollView scrollView = new ScrollView(this);
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);

        TextView title = new TextView(this);
        title.setText("Settings & Info");
        title.setTextSize(18);
        title.setTextColor(Color.WHITE);
        title.setTypeface(null, Typeface.BOLD);
        layout.addView(title);

        // Card 1: Mod Path
        CardViewBuilder pathCard = new CardViewBuilder(this, "#212121");
        pathCard.addTitle("Mod Root Directory Path");
        EditText pathInput = new EditText(this);
        pathInput.setText(modsDirectoryPath);
        pathInput.setTextColor(Color.WHITE);
        pathInput.setBackgroundColor(Color.parseColor("#2C2C2C"));
        pathCard.addCustomView(pathInput);

        Button saveBtn = new Button(this);
        saveBtn.setText("Save Path");
        saveBtn.setOnClickListener(v -> {
            modsDirectoryPath = pathInput.getText().toString();
            logMessages.add("[CONFIG] Mods path updated to: " + modsDirectoryPath);
            Toast.makeText(this, "Mods path updated", Toast.LENGTH_SHORT).show();
        });
        pathCard.addCustomView(saveBtn);
        layout.addView(pathCard.build());

        // Card 2: Permission Status
        CardViewBuilder permCard2 = new CardViewBuilder(this, "#212121");
        permCard2.addTitle("Permission Status");

        boolean hasStorage = true;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            hasStorage = Environment.isExternalStorageManager();
        }
        permCard2.addDetail("MANAGE_EXTERNAL_STORAGE: " + (hasStorage ? "✔ Granted" : "❌ Not Granted"));
        permCard2.addDetail("WRITE_SECURE_SETTINGS: " + (hasSecureSettingsPermission() ? "✔ Granted" : "❌ Not Granted"));
        permCard2.addDetail("Shizuku Status: " + (isShizukuAvailable() ? (checkShizukuPermission() ? "✔ Authorized" : "⚠️ Needs Auth") : "❌ Not Running"));
        layout.addView(permCard2.build());

        // Card 3: App & Engine Version Info
        CardViewBuilder infoCard = new CardViewBuilder(this, "#212121");
        infoCard.addTitle("App & Engine Details");
        infoCard.addDetail("Launcher Version: 1.0.0 (ARM64)");
        infoCard.addDetail("Architecture: ARM64 (aarch64)");
        infoCard.addDetail("Graphics API: Vulkan 1.3");
        infoCard.addDetail("Hook Core: VK_LAYER_GIMI_arm64");
        layout.addView(infoCard.build());

        // Card 4: System Logs
        CardViewBuilder logsCard = new CardViewBuilder(this, "#1E1E1E");
        logsCard.addTitle("System Logs");
        logsContainer = new LinearLayout(this);
        logsContainer.setOrientation(LinearLayout.VERTICAL);
        for (String msg : logMessages) {
            TextView logLine = new TextView(this);
            logLine.setText(msg);
            logLine.setTextColor(Color.GREEN);
            logLine.setTextSize(11);
            logLine.setTypeface(Typeface.MONOSPACE);
            logsContainer.addView(logLine);
        }
        logsCard.addCustomView(logsContainer);
        layout.addView(logsCard.build());

        scrollView.addView(layout);
        contentContainer.addView(scrollView);
    }

    // Helper Card View Builder
    private static class CardViewBuilder {
        private LinearLayout card;

        public CardViewBuilder(Activity context, String bgColor) {
            card = new LinearLayout(context);
            card.setOrientation(LinearLayout.VERTICAL);
            card.setBackgroundColor(Color.parseColor(bgColor));
            card.setPadding(24, 24, 24, 24);
            LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
            params.setMargins(0, 0, 0, 16);
            card.setLayoutParams(params);
        }

        public void addTitle(String titleText) {
            TextView tv = new TextView(card.getContext());
            tv.setText(titleText);
            tv.setTextColor(Color.WHITE);
            tv.setTextSize(15);
            tv.setTypeface(null, Typeface.BOLD);
            card.addView(tv);
        }

        public void addSubtitle(String subText) {
            TextView tv = new TextView(card.getContext());
            tv.setText(subText);
            tv.setTextColor(Color.LTGRAY);
            tv.setTextSize(13);
            tv.setPadding(0, 4, 0, 4);
            card.addView(tv);
        }

        public void addDetail(String detailText) {
            TextView tv = new TextView(card.getContext());
            tv.setText(detailText);
            tv.setTextColor(Color.GRAY);
            tv.setTextSize(12);
            card.addView(tv);
        }

        public void addCustomView(View view) {
            card.addView(view);
        }

        public LinearLayout build() {
            return card;
        }
    }
}
