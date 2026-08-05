package com.gimi.launcher;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.gimi.launcher.jni.GimiNativeBridge;
import com.gimi.launcher.jni.ModInfo;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {

    private LinearLayout mainContainer;
    private Button tabDashboardBtn;
    private Button tabModsBtn;
    private Button tabSettingsBtn;

    private ScrollView dashboardView;
    private ScrollView modsView;
    private ScrollView settingsView;

    // Dashboard views
    private TextView vulkanStatusText;
    private TextView adbStatusText;
    private TextView adbCommandBox;
    private TextView manualCommandsBox;
    private Button toggleManualBtn;
    private Spinner gameSpinner;
    private Button injectBtn;
    private Button revertBtn;
    private TextView outputLogText;

    // Mods views
    private EditText searchEditText;
    private TextView modCountText;
    private LinearLayout modsListContainer;

    // Data
    private final String[] packageNames = new String[]{
        "com.miHoYo.GenshinImpact",
        "com.miHoYo.GI.samsung",
        "com.yuanshen.site",
        "com.miHoYo.ys.bilibili"
    };
    private final String[] baseGameNames = new String[]{
        "Global / Play Store",
        "Galaxy Store (Samsung)",
        "China Server (Official)",
        "Bilibili (China)"
    };
    private String[] displayGameNames;
    private String selectedPackage = packageNames[0];
    private List<ModInfo> currentModsList = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        try {
            ensureModsDirectory();
        } catch (Throwable e) {
            e.printStackTrace();
        }

        try {
            setupUI();
        } catch (Throwable e) {
            e.printStackTrace();
        }

        try {
            checkStoragePermission();
        } catch (Throwable e) {
            e.printStackTrace();
        }

        try {
            refreshDashboardStatus();
        } catch (Throwable e) {
            e.printStackTrace();
        }

        try {
            refreshModsList();
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        try {
            refreshDashboardStatus();
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    private void ensureModsDirectory() {
        File modsDir = new File("/sdcard/GIMI/Mods");
        if (!modsDir.exists()) {
            modsDir.mkdirs();
        }
    }

    private void checkStoragePermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                try {
                    Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                    intent.setData(Uri.parse("package:" + getPackageName()));
                    startActivity(intent);
                } catch (Exception e) {
                    try {
                        Intent intent = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                        startActivity(intent);
                    } catch (Exception e2) {
                        Toast.makeText(this, "Grant All Files Access manually in Settings", Toast.LENGTH_LONG).show();
                    }
                }
            }
        }
    }

    private void setupUI() {
        LinearLayout rootLayout = new LinearLayout(this);
        rootLayout.setOrientation(LinearLayout.VERTICAL);
        rootLayout.setBackgroundColor(Color.parseColor("#121212"));
        rootLayout.setLayoutParams(new ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
        ));
        rootLayout.setPadding(24, 48, 24, 24);

        // Header Layout
        LinearLayout headerLayout = new LinearLayout(this);
        headerLayout.setOrientation(LinearLayout.VERTICAL);
        headerLayout.setPadding(8, 8, 8, 20);

        TextView titleText = new TextView(this);
        titleText.setText("GIMI Launcher");
        titleText.setTextColor(Color.WHITE);
        titleText.setTextSize(26f);
        titleText.setTypeface(null, Typeface.BOLD);

        TextView subtitleText = new TextView(this);
        subtitleText.setText("Vulkan 3D Model Importer • ARM64 Android Native");
        subtitleText.setTextColor(Color.parseColor("#B0BEC5"));
        subtitleText.setTextSize(13f);

        headerLayout.addView(titleText);
        headerLayout.addView(subtitleText);
        rootLayout.addView(headerLayout);

        // Navigation Tab Bar
        LinearLayout tabBar = new LinearLayout(this);
        tabBar.setOrientation(LinearLayout.HORIZONTAL);
        LinearLayout.LayoutParams tabParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        tabParams.setMargins(0, 0, 0, 20);
        tabBar.setLayoutParams(tabParams);

        tabDashboardBtn = createTabButton("Dashboard", true);
        tabModsBtn = createTabButton("Mod Manager", false);
        tabSettingsBtn = createTabButton("Settings", false);

        tabDashboardBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switchTab(0);
            }
        });

        tabModsBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switchTab(1);
            }
        });

        tabSettingsBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switchTab(2);
            }
        });

        tabBar.addView(tabDashboardBtn);
        tabBar.addView(tabModsBtn);
        tabBar.addView(tabSettingsBtn);
        rootLayout.addView(tabBar);

        // Main Content Container
        mainContainer = new LinearLayout(this);
        mainContainer.setOrientation(LinearLayout.VERTICAL);
        mainContainer.setLayoutParams(new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            0,
            1.0f
        ));

        // Create Tab Views
        dashboardView = createDashboardView();
        modsView = createModsView();
        settingsView = createSettingsView();

        mainContainer.addView(dashboardView);
        mainContainer.addView(modsView);
        mainContainer.addView(settingsView);

        rootLayout.addView(mainContainer);
        setContentView(rootLayout);

        switchTab(0);
    }

    private Button createTabButton(String text, boolean active) {
        Button btn = new Button(this);
        btn.setText(text);
        btn.setTextSize(13f);
        btn.setAllCaps(false);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f);
        params.setMargins(4, 0, 4, 0);
        btn.setLayoutParams(params);
        updateTabButtonState(btn, active);
        return btn;
    }

    private void updateTabButtonState(Button btn, boolean active) {
        GradientDrawable drawable = new GradientDrawable();
        drawable.setCornerRadius(16f);
        if (active) {
            drawable.setColor(Color.parseColor("#00E676"));
            btn.setTextColor(Color.BLACK);
            btn.setTypeface(null, Typeface.BOLD);
        } else {
            drawable.setColor(Color.parseColor("#263238"));
            btn.setTextColor(Color.WHITE);
            btn.setTypeface(null, Typeface.NORMAL);
        }
        btn.setBackground(drawable);
    }

    private void switchTab(int tabIndex) {
        dashboardView.setVisibility(tabIndex == 0 ? View.VISIBLE : View.GONE);
        modsView.setVisibility(tabIndex == 1 ? View.VISIBLE : View.GONE);
        settingsView.setVisibility(tabIndex == 2 ? View.VISIBLE : View.GONE);

        updateTabButtonState(tabDashboardBtn, tabIndex == 0);
        updateTabButtonState(tabModsBtn, tabIndex == 1);
        updateTabButtonState(tabSettingsBtn, tabIndex == 2);

        if (tabIndex == 1) {
            refreshModsList();
        }
    }

    // ─── Dashboard Tab ──────────────────────────────────────────────────────────
    private ScrollView createDashboardView() {
        ScrollView scrollView = new ScrollView(this);
        scrollView.setLayoutParams(new ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
        ));

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(4, 4, 4, 16);

        // 1. Vulkan Layer Status Card
        LinearLayout vulkanCard = createCardLayout();
        TextView vulkanTitle = new TextView(this);
        vulkanTitle.setText("Vulkan Layer Status");
        vulkanTitle.setTextColor(Color.WHITE);
        vulkanTitle.setTextSize(16f);
        vulkanTitle.setTypeface(null, Typeface.BOLD);

        vulkanStatusText = new TextView(this);
        vulkanStatusText.setText("🔴 INACTIVE (Layer Disconnected)");
        vulkanStatusText.setTextColor(Color.parseColor("#FF5252"));
        vulkanStatusText.setTextSize(14f);
        vulkanStatusText.setTypeface(null, Typeface.BOLD);
        vulkanStatusText.setPadding(0, 8, 0, 0);

        vulkanCard.addView(vulkanTitle);
        vulkanCard.addView(vulkanStatusText);
        layout.addView(vulkanCard);

        // 2. ADB Privilege Status Card
        LinearLayout adbCard = createCardLayout();
        TextView adbTitle = new TextView(this);
        adbTitle.setText("ADB Privilege Status (WRITE_SECURE_SETTINGS)");
        adbTitle.setTextColor(Color.WHITE);
        adbTitle.setTextSize(16f);
        adbTitle.setTypeface(null, Typeface.BOLD);

        adbStatusText = new TextView(this);
        adbStatusText.setText("⚠️ Privilege Pending");
        adbStatusText.setTextColor(Color.parseColor("#FF9100"));
        adbStatusText.setTextSize(14f);
        adbStatusText.setTypeface(null, Typeface.BOLD);
        adbStatusText.setPadding(0, 6, 0, 8);

        TextView adbInstructionText = new TextView(this);
        adbInstructionText.setText("Run this ADB command on PC / Termux / LADB to grant privileges:");
        adbInstructionText.setTextColor(Color.parseColor("#B0BEC5"));
        adbInstructionText.setTextSize(12f);

        final String grantCommand = "adb shell pm grant com.gimi.launcher android.permission.WRITE_SECURE_SETTINGS";

        adbCommandBox = new TextView(this);
        adbCommandBox.setText(grantCommand);
        adbCommandBox.setTextColor(Color.parseColor("#80D8FF"));
        adbCommandBox.setTextSize(12f);
        adbCommandBox.setTypeface(Typeface.MONOSPACE);
        adbCommandBox.setBackgroundColor(Color.parseColor("#000000"));
        adbCommandBox.setPadding(16, 16, 16, 16);
        adbCommandBox.setTextIsSelectable(true);

        LinearLayout.LayoutParams cmdParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        cmdParams.setMargins(0, 8, 0, 8);
        adbCommandBox.setLayoutParams(cmdParams);
        adbCommandBox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                copyToClipboard(grantCommand, "Comando copiado!");
            }
        });

        LinearLayout btnRow = new LinearLayout(this);
        btnRow.setOrientation(LinearLayout.HORIZONTAL);
        LinearLayout.LayoutParams btnRowParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        btnRowParams.setMargins(0, 4, 0, 4);
        btnRow.setLayoutParams(btnRowParams);

        Button copyBtn = new Button(this);
        copyBtn.setText("Copy Command 📋");
        copyBtn.setTextSize(12f);
        copyBtn.setAllCaps(false);
        LinearLayout.LayoutParams copyParams = new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f);
        copyParams.setMargins(0, 0, 4, 0);
        copyBtn.setLayoutParams(copyParams);
        copyBtn.setBackground(createButtonDrawable("#263238"));
        copyBtn.setTextColor(Color.WHITE);
        copyBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                copyToClipboard(grantCommand, "Comando copiado!");
            }
        });

        Button checkBtn = new Button(this);
        checkBtn.setText("Check Permission 🔄");
        checkBtn.setTextSize(12f);
        checkBtn.setAllCaps(false);
        LinearLayout.LayoutParams checkParams = new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f);
        checkParams.setMargins(4, 0, 0, 0);
        checkBtn.setLayoutParams(checkParams);
        checkBtn.setBackground(createButtonDrawable("#00E676"));
        checkBtn.setTextColor(Color.BLACK);
        checkBtn.setTypeface(null, Typeface.BOLD);
        checkBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                refreshDashboardStatus();
            }
        });

        btnRow.addView(copyBtn);
        btnRow.addView(checkBtn);

        toggleManualBtn = new Button(this);
        toggleManualBtn.setText("Show Manual ADB Settings Commands ▼");
        toggleManualBtn.setTextSize(11f);
        toggleManualBtn.setAllCaps(false);
        toggleManualBtn.setBackground(null);
        toggleManualBtn.setTextColor(Color.parseColor("#80D8FF"));
        toggleManualBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (manualCommandsBox.getVisibility() == View.GONE) {
                    manualCommandsBox.setVisibility(View.VISIBLE);
                    toggleManualBtn.setText("Hide Manual ADB Settings Commands ▲");
                } else {
                    manualCommandsBox.setVisibility(View.GONE);
                    toggleManualBtn.setText("Show Manual ADB Settings Commands ▼");
                }
            }
        });

        final String manualCmdsText = "adb shell settings put global enable_gpu_debug_layers 1\n" +
            "adb shell settings put global gpu_debug_app " + selectedPackage + "\n" +
            "adb shell settings put global gpu_debug_layer_app com.gimi.launcher\n" +
            "adb shell settings put global gpu_debug_layers libgimi_arm64.so";

        manualCommandsBox = new TextView(this);
        manualCommandsBox.setText(manualCmdsText);
        manualCommandsBox.setTextColor(Color.parseColor("#A5D6A7"));
        manualCommandsBox.setTextSize(11f);
        manualCommandsBox.setTypeface(Typeface.MONOSPACE);
        manualCommandsBox.setBackgroundColor(Color.parseColor("#05190E"));
        manualCommandsBox.setPadding(16, 16, 16, 16);
        manualCommandsBox.setVisibility(View.GONE);
        manualCommandsBox.setTextIsSelectable(true);

        LinearLayout.LayoutParams manualParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        manualParams.setMargins(0, 4, 0, 4);
        manualCommandsBox.setLayoutParams(manualParams);
        manualCommandsBox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                copyToClipboard(manualCmdsText, "Comandos manuais copiados!");
            }
        });

        adbCard.addView(adbTitle);
        adbCard.addView(adbStatusText);
        adbCard.addView(adbInstructionText);
        adbCard.addView(adbCommandBox);
        adbCard.addView(btnRow);
        adbCard.addView(toggleManualBtn);
        adbCard.addView(manualCommandsBox);
        layout.addView(adbCard);

        // 3. Target Game Variant & Injection Control Card
        LinearLayout gameCard = createCardLayout();
        TextView gameTitle = new TextView(this);
        gameTitle.setText("Target Game Variant & Controls");
        gameTitle.setTextColor(Color.WHITE);
        gameTitle.setTextSize(16f);
        gameTitle.setTypeface(null, Typeface.BOLD);

        // Auto Detect Installed Game Packages
        displayGameNames = new String[packageNames.length];
        int firstInstalledIndex = 0;
        boolean foundInstalled = false;

        for (int i = 0; i < packageNames.length; i++) {
            boolean isInstalled = isPackageInstalled(packageNames[i]);
            if (isInstalled) {
                displayGameNames[i] = baseGameNames[i] + " [INSTALADO 🟢]";
                if (!foundInstalled) {
                    firstInstalledIndex = i;
                    foundInstalled = true;
                }
            } else {
                displayGameNames[i] = baseGameNames[i] + " [Não instalado ⚪]";
            }
        }

        gameSpinner = new Spinner(this);
        ArrayAdapter<String> spinnerAdapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, displayGameNames);
        gameSpinner.setAdapter(spinnerAdapter);
        if (foundInstalled) {
            gameSpinner.setSelection(firstInstalledIndex);
            selectedPackage = packageNames[firstInstalledIndex];
        }

        LinearLayout.LayoutParams spinnerParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        spinnerParams.setMargins(0, 8, 0, 12);
        gameSpinner.setLayoutParams(spinnerParams);

        gameSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                selectedPackage = packageNames[position];
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {}
        });

        // Action Buttons Row
        injectBtn = new Button(this);
        injectBtn.setText("🚀 INJECT LAYER & LAUNCH GAME");
        injectBtn.setTextSize(14f);
        injectBtn.setAllCaps(true);
        injectBtn.setTypeface(null, Typeface.BOLD);
        injectBtn.setTextColor(Color.BLACK);
        injectBtn.setBackground(createButtonDrawable("#00E676"));

        LinearLayout.LayoutParams injectParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            120
        );
        injectParams.setMargins(0, 8, 0, 8);
        injectBtn.setLayoutParams(injectParams);
        injectBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                doInjectAndLaunchGame();
            }
        });

        revertBtn = new Button(this);
        revertBtn.setText("🛑 RESET / DESATIVAR VULKAN LAYER");
        revertBtn.setTextSize(13f);
        revertBtn.setAllCaps(true);
        revertBtn.setTypeface(null, Typeface.BOLD);
        revertBtn.setTextColor(Color.WHITE);
        revertBtn.setBackground(createButtonDrawable("#D32F2F"));

        LinearLayout.LayoutParams revertParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            100
        );
        revertParams.setMargins(0, 0, 0, 12);
        revertBtn.setLayoutParams(revertParams);
        revertBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                doRevertLayer();
            }
        });

        outputLogText = new TextView(this);
        outputLogText.setText("System Output: Ready");
        outputLogText.setTextColor(Color.parseColor("#ECEFF1"));
        outputLogText.setTextSize(12f);
        outputLogText.setPadding(12, 12, 12, 12);
        outputLogText.setBackgroundColor(Color.parseColor("#263238"));

        gameCard.addView(gameTitle);
        gameCard.addView(gameSpinner);
        gameCard.addView(injectBtn);
        gameCard.addView(revertBtn);
        gameCard.addView(outputLogText);
        layout.addView(gameCard);

        scrollView.addView(layout);
        return scrollView;
    }

    private boolean isPackageInstalled(String packageName) {
        try {
            getPackageManager().getPackageInfo(packageName, 0);
            return true;
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }
    }

    private void refreshDashboardStatus() {
        int status = GimiNativeBridge.getLayerStatus();
        boolean isGranted = checkSelfPermission("android.permission.WRITE_SECURE_SETTINGS") == PackageManager.PERMISSION_GRANTED;

        String currentGpuApp = Settings.Global.getString(getContentResolver(), "gpu_debug_app");
        int layerEnabled = Settings.Global.getInt(getContentResolver(), "enable_gpu_debug_layers", 0);

        if (layerEnabled == 1 && currentGpuApp != null && !currentGpuApp.isEmpty()) {
            vulkanStatusText.setText("🟢 ACTIVE (Hook Injected for " + currentGpuApp + ")");
            vulkanStatusText.setTextColor(Color.parseColor("#00E676"));
        } else if (status > 0) {
            vulkanStatusText.setText("🟢 ACTIVE (Hook Loaded)");
            vulkanStatusText.setTextColor(Color.parseColor("#00E676"));
        } else {
            vulkanStatusText.setText("🔴 INACTIVE (Layer Disconnected / Reverted)");
            vulkanStatusText.setTextColor(Color.parseColor("#FF5252"));
        }

        if (isGranted) {
            adbStatusText.setText("⚡ Privileges Granted: WRITE_SECURE_SETTINGS Active");
            adbStatusText.setTextColor(Color.parseColor("#00E676"));
        } else {
            adbStatusText.setText("⚠️ Privilege Pending: WRITE_SECURE_SETTINGS Missing");
            adbStatusText.setTextColor(Color.parseColor("#FF9100"));
        }
    }

    private void doInjectAndLaunchGame() {
        try {
            Settings.Global.putInt(getContentResolver(), "enable_gpu_debug_layers", 1);
            Settings.Global.putString(getContentResolver(), "gpu_debug_app", selectedPackage);
            Settings.Global.putString(getContentResolver(), "gpu_debug_layer_app", getPackageName());
            Settings.Global.putString(getContentResolver(), "gpu_debug_layers", "libgimi_arm64.so");

            refreshDashboardStatus();

            Intent launchIntent = getPackageManager().getLaunchIntentForPackage(selectedPackage);
            if (launchIntent != null) {
                outputLogText.setText("System Output: Layer injected! Launching game " + selectedPackage + "...");
                outputLogText.setTextColor(Color.parseColor("#00E676"));
                Toast.makeText(this, "Injetado com sucesso! Abrindo o jogo...", Toast.LENGTH_SHORT).show();
                startActivity(launchIntent);
            } else {
                outputLogText.setText("System Output: Layer enabled, but package " + selectedPackage + " is NOT installed!");
                outputLogText.setTextColor(Color.parseColor("#FF9100"));
                Toast.makeText(this, "Layer ativada, mas o pacote " + selectedPackage + " não está instalado!", Toast.LENGTH_LONG).show();
            }
        } catch (SecurityException e) {
            outputLogText.setText("SecurityException: WRITE_SECURE_SETTINGS not granted. Run ADB command above!");
            outputLogText.setTextColor(Color.parseColor("#FF5252"));
            Toast.makeText(this, "Execute o comando ADB de permissão!", Toast.LENGTH_SHORT).show();
        } catch (Throwable e) {
            outputLogText.setText("Error: " + e.getMessage());
            outputLogText.setTextColor(Color.parseColor("#FF5252"));
        }
    }

    private void doRevertLayer() {
        try {
            Settings.Global.putInt(getContentResolver(), "enable_gpu_debug_layers", 0);
            Settings.Global.putString(getContentResolver(), "gpu_debug_app", "");
            Settings.Global.putString(getContentResolver(), "gpu_debug_layer_app", "");
            Settings.Global.putString(getContentResolver(), "gpu_debug_layers", "");

            outputLogText.setText("System Output: Reverted Vulkan debug layer settings (Layer Disabled).");
            outputLogText.setTextColor(Color.parseColor("#80D8FF"));
            refreshDashboardStatus();
            Toast.makeText(this, "Vulkan Layer desativada e revertida!", Toast.LENGTH_SHORT).show();
        } catch (SecurityException e) {
            outputLogText.setText("SecurityException: WRITE_SECURE_SETTINGS not granted.");
            outputLogText.setTextColor(Color.parseColor("#FF5252"));
        } catch (Throwable e) {
            outputLogText.setText("Error: " + e.getMessage());
            outputLogText.setTextColor(Color.parseColor("#FF5252"));
        }
    }

    // ─── Mod Manager Tab ────────────────────────────────────────────────────────
    private ScrollView createModsView() {
        ScrollView scrollView = new ScrollView(this);
        scrollView.setLayoutParams(new ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
        ));

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(4, 4, 4, 16);

        LinearLayout searchCard = createCardLayout();
        TextView title = new TextView(this);
        title.setText("Mod Manager");
        title.setTextColor(Color.WHITE);
        title.setTextSize(18f);
        title.setTypeface(null, Typeface.BOLD);

        searchEditText = new EditText(this);
        searchEditText.setHint("Search mods...");
        searchEditText.setHintTextColor(Color.parseColor("#78909C"));
        searchEditText.setTextColor(Color.WHITE);
        searchEditText.setTextSize(14f);
        searchEditText.setPadding(16, 16, 16, 16);
        searchEditText.setBackgroundColor(Color.parseColor("#263238"));
        searchEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                filterModsList(s.toString());
            }

            @Override
            public void afterTextChanged(Editable s) {}
        });

        modCountText = new TextView(this);
        modCountText.setText("Total Mods: 0");
        modCountText.setTextColor(Color.parseColor("#B0BEC5"));
        modCountText.setTextSize(13f);
        modCountText.setPadding(0, 12, 0, 4);

        LinearLayout actionRow = new LinearLayout(this);
        actionRow.setOrientation(LinearLayout.HORIZONTAL);
        LinearLayout.LayoutParams actionParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        actionParams.setMargins(0, 4, 0, 4);
        actionRow.setLayoutParams(actionParams);

        Button enableAllBtn = new Button(this);
        enableAllBtn.setText("Enable All ✅");
        enableAllBtn.setTextSize(11f);
        enableAllBtn.setAllCaps(false);
        LinearLayout.LayoutParams enableParams = new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f);
        enableParams.setMargins(0, 0, 2, 0);
        enableAllBtn.setLayoutParams(enableParams);
        enableAllBtn.setBackground(createButtonDrawable("#00E676"));
        enableAllBtn.setTextColor(Color.BLACK);
        enableAllBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                batchToggleMods(true);
            }
        });

        Button disableAllBtn = new Button(this);
        disableAllBtn.setText("Disable All ❌");
        disableAllBtn.setTextSize(11f);
        disableAllBtn.setAllCaps(false);
        LinearLayout.LayoutParams disableParams = new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f);
        disableParams.setMargins(2, 0, 2, 0);
        disableAllBtn.setLayoutParams(disableParams);
        disableAllBtn.setBackground(createButtonDrawable("#D32F2F"));
        disableAllBtn.setTextColor(Color.WHITE);
        disableAllBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                batchToggleMods(false);
            }
        });

        Button refreshBtn = new Button(this);
        refreshBtn.setText("Scan 🔄");
        refreshBtn.setTextSize(11f);
        refreshBtn.setAllCaps(false);
        LinearLayout.LayoutParams refreshParams = new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f);
        refreshParams.setMargins(2, 0, 0, 0);
        refreshBtn.setLayoutParams(refreshParams);
        refreshBtn.setBackground(createButtonDrawable("#00B0FF"));
        refreshBtn.setTextColor(Color.BLACK);
        refreshBtn.setTypeface(null, Typeface.BOLD);
        refreshBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                refreshModsList();
            }
        });

        actionRow.addView(enableAllBtn);
        actionRow.addView(disableAllBtn);
        actionRow.addView(refreshBtn);

        searchCard.addView(title);
        searchCard.addView(searchEditText);
        searchCard.addView(modCountText);
        searchCard.addView(actionRow);
        layout.addView(searchCard);

        modsListContainer = new LinearLayout(this);
        modsListContainer.setOrientation(LinearLayout.VERTICAL);
        layout.addView(modsListContainer);

        scrollView.addView(layout);
        return scrollView;
    }

    private void batchToggleMods(boolean enable) {
        for (ModInfo mod : currentModsList) {
            GimiNativeBridge.toggleMod(mod.getPath(), enable);
        }
        refreshModsList();
    }

    private void refreshModsList() {
        currentModsList = GimiNativeBridge.scanMods("/sdcard/GIMI/Mods");
        filterModsList(searchEditText != null ? searchEditText.getText().toString() : "");
    }

    private void filterModsList(String query) {
        if (modsListContainer == null) return;
        modsListContainer.removeAllViews();

        List<ModInfo> filtered = new ArrayList<>();
        if (query == null || query.trim().isEmpty()) {
            filtered.addAll(currentModsList);
        } else {
            String q = query.toLowerCase();
            for (ModInfo mod : currentModsList) {
                if (mod.getName() != null && mod.getName().toLowerCase().contains(q)) {
                    filtered.add(mod);
                }
            }
        }

        if (modCountText != null) {
            modCountText.setText("Total Mods: " + filtered.size());
        }

        if (filtered.isEmpty()) {
            TextView emptyText = new TextView(this);
            emptyText.setText("No mods found in /sdcard/GIMI/Mods/\n\nDrop your 3dmigoto mod folders inside /sdcard/GIMI/Mods/ to manage them here!");
            emptyText.setTextColor(Color.parseColor("#78909C"));
            emptyText.setTextSize(13f);
            emptyText.setGravity(Gravity.CENTER);
            emptyText.setPadding(0, 32, 0, 32);
            modsListContainer.addView(emptyText);
            return;
        }

        for (final ModInfo mod : filtered) {
            LinearLayout card = createCardLayout();
            LinearLayout row = new LinearLayout(this);
            row.setOrientation(LinearLayout.HORIZONTAL);
            row.setGravity(Gravity.CENTER_VERTICAL);

            LinearLayout infoLayout = new LinearLayout(this);
            infoLayout.setOrientation(LinearLayout.VERTICAL);
            infoLayout.setLayoutParams(new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f));

            TextView modName = new TextView(this);
            modName.setText(mod.getName());
            modName.setTextColor(Color.WHITE);
            modName.setTextSize(15f);
            modName.setTypeface(null, Typeface.BOLD);

            TextView modDetails = new TextView(this);
            modDetails.setText("Ini files: " + mod.getIniCount() + " | Path: " + mod.getPath());
            modDetails.setTextColor(Color.parseColor("#B0BEC5"));
            modDetails.setTextSize(11f);

            infoLayout.addView(modName);
            infoLayout.addView(modDetails);

            Switch toggleSwitch = new Switch(this);
            toggleSwitch.setChecked(mod.isEnabled());
            toggleSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    GimiNativeBridge.toggleMod(mod.getPath(), isChecked);
                }
            });

            row.addView(infoLayout);
            row.addView(toggleSwitch);
            card.addView(row);
            modsListContainer.addView(card);
        }
    }

    // ─── Settings Tab ───────────────────────────────────────────────────────────
    private ScrollView createSettingsView() {
        ScrollView scrollView = new ScrollView(this);
        scrollView.setLayoutParams(new ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
        ));

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(4, 4, 4, 16);

        LinearLayout settingsCard = createCardLayout();
        TextView title = new TextView(this);
        title.setText("Settings & Directory Paths");
        title.setTextColor(Color.WHITE);
        title.setTextSize(18f);
        title.setTypeface(null, Typeface.BOLD);

        TextView pathLabel = new TextView(this);
        pathLabel.setText("Mods Path Directory:");
        pathLabel.setTextColor(Color.parseColor("#B0BEC5"));
        pathLabel.setTextSize(12f);
        pathLabel.setPadding(0, 12, 0, 4);

        EditText pathEditText = new EditText(this);
        pathEditText.setText("/sdcard/GIMI/Mods");
        pathEditText.setTextColor(Color.WHITE);
        pathEditText.setTextSize(14f);
        pathEditText.setPadding(16, 16, 16, 16);
        pathEditText.setBackgroundColor(Color.parseColor("#263238"));

        settingsCard.addView(title);
        settingsCard.addView(pathLabel);
        settingsCard.addView(pathEditText);
        layout.addView(settingsCard);

        LinearLayout infoCard = createCardLayout();
        TextView infoTitle = new TextView(this);
        infoTitle.setText("App & System Info");
        infoTitle.setTextColor(Color.WHITE);
        infoTitle.setTextSize(16f);
        infoTitle.setTypeface(null, Typeface.BOLD);

        TextView infoDetails = new TextView(this);
        infoDetails.setText(
            "App Name: GIMI Launcher\n" +
            "Version: 1.0.0 (Release v1.0)\n" +
            "Architecture: ARM64 (arm64-v8a)\n" +
            "Graphics API: Vulkan 1.3 / OpenGL ES 3.2\n" +
            "Hook Engine: Vulkan Layer Spec + Dobby C++20\n" +
            "Android API Level: " + Build.VERSION.SDK_INT
        );
        infoDetails.setTextColor(Color.parseColor("#B0BEC5"));
        infoDetails.setTextSize(13f);
        infoDetails.setPadding(0, 8, 0, 0);

        infoCard.addView(infoTitle);
        infoCard.addView(infoDetails);
        layout.addView(infoCard);

        scrollView.addView(layout);
        return scrollView;
    }

    // ─── Helpers ────────────────────────────────────────────────────────────────
    private LinearLayout createCardLayout() {
        LinearLayout card = new LinearLayout(this);
        card.setOrientation(LinearLayout.VERTICAL);
        card.setPadding(20, 20, 20, 20);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        params.setMargins(0, 0, 0, 16);
        card.setLayoutParams(params);

        GradientDrawable drawable = new GradientDrawable();
        drawable.setColor(Color.parseColor("#1E1E1E"));
        drawable.setCornerRadius(16f);
        card.setBackground(drawable);
        return card;
    }

    private GradientDrawable createButtonDrawable(String colorHex) {
        GradientDrawable drawable = new GradientDrawable();
        drawable.setColor(Color.parseColor(colorHex));
        drawable.setCornerRadius(12f);
        return drawable;
    }

    private void copyToClipboard(String text, String toastMsg) {
        ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
        ClipData clip = ClipData.newPlainText("ADB Command", text);
        if (clipboard != null) {
            clipboard.setPrimaryClip(clip);
        }
        Toast.makeText(this, toastMsg, Toast.LENGTH_SHORT).show();
    }
}
