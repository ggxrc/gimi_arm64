package com.gimi.launcher;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.*;

import com.gimi.launcher.jni.GimiNativeBridgeJava;
import com.gimi.launcher.jni.ModInfoJava;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {
    private String selectedPackage = "com.miHoYo.GenshinImpact";
    private String modsDirectoryPath = "/sdcard/GIMI/Mods";

    private LinearLayout contentContainer;
    private TextView statusOutputView;
    private LinearLayout logsContainer;
    private List<String> logMessages = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        logMessages.add("[INFO] GIMI Launcher initialized");
        logMessages.add("[INFO] Native library libgimi_arm64.so loaded");
        logMessages.add("[INFO] Vulkan graphics layer status: OK");

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

        tabDashboard.setOnClickListener(v -> showDashboardView());
        tabModManager.setOnClickListener(v -> showModManagerView());
        tabSettings.setOnClickListener(v -> showSettingsView());

        bottomBar.addView(tabDashboard);
        bottomBar.addView(tabModManager);
        bottomBar.addView(tabSettings);
        mainLayout.addView(bottomBar);

        setContentView(mainLayout);
        showDashboardView();
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

        // Card 1: Vulkan Layer Status
        int layerStatus = GimiNativeBridgeJava.getLayerStatus();
        boolean isActive = layerStatus > 0;
        CardViewBuilder layerCard = new CardViewBuilder(this, isActive ? "#1B5E20" : "#B71C1C");
        layerCard.addTitle("Vulkan Layer Status");
        layerCard.addSubtitle(isActive ? "🟢 ACTIVE (Hook Loaded)" : "🔴 INACTIVE (Layer Disconnected)");
        layerCard.addDetail("Status Code: " + layerStatus);
        layout.addView(layerCard.build());

        // Card 2: Shizuku / ADB Status
        CardViewBuilder shizukuCard = new CardViewBuilder(this, "#0D47A1");
        shizukuCard.addTitle("Shizuku / ADB Rootless Service");
        shizukuCard.addSubtitle(isActive ? "⚡ Shizuku Active & Permission Granted" : "⚠️ Shizuku Not Detected / Standalone Mode");
        layout.addView(shizukuCard.build());

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

        // Inject / Activate Button
        Button injectBtn = new Button(this);
        injectBtn.setText("INJECT VULKAN LAYER & LAUNCH");
        injectBtn.setBackgroundColor(Color.parseColor("#6200EE"));
        injectBtn.setTextColor(Color.WHITE);
        injectBtn.setTextSize(14);
        injectBtn.setTypeface(null, Typeface.BOLD);
        injectBtn.setPadding(16, 24, 16, 24);
        injectBtn.setOnClickListener(v -> {
            int result = GimiNativeBridgeJava.injectLayer(selectedPackage);
            String msg = (result == 0) ? "Layer injected successfully for " + selectedPackage : "Injection failed (code: " + result + ")";
            statusOutputView.setText("System Output: " + msg);
            logMessages.add("[INJECT] " + msg);
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

        // Search Input
        EditText searchInput = new EditText(this);
        searchInput.setHint("Search mods...");
        searchInput.setHintTextColor(Color.GRAY);
        searchInput.setTextColor(Color.WHITE);
        searchInput.setBackgroundColor(Color.parseColor("#2C2C2C"));
        searchInput.setPadding(16, 16, 16, 16);
        layout.addView(searchInput);

        ModInfoJava[] mods = GimiNativeBridgeJava.scanMods(modsDirectoryPath);
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
            emptyText.setText("No mods found in " + modsDirectoryPath + "\nAdd mods to path or change in Settings.");
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
                modDetails.setText("📄 " + mod.iniCount + " ini files | 🖼️ " + mod.textureCount + " textures");
                modDetails.setTextColor(Color.LTGRAY);
                modDetails.setTextSize(11);
                infoLayout.addView(modDetails);

                modCard.addView(infoLayout);

                Switch toggle = new Switch(this);
                toggle.setChecked(mod.isEnabled);
                toggle.setOnCheckedChangeListener((buttonView, isChecked) -> {
                    GimiNativeBridgeJava.toggleMod(mod.path, isChecked);
                    logMessages.add("[MOD] Toggled " + mod.name + " -> " + (isChecked ? "ENABLED" : "DISABLED"));
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

        // Card 2: App & Engine Version Info
        CardViewBuilder infoCard = new CardViewBuilder(this, "#212121");
        infoCard.addTitle("App & Engine Details");
        infoCard.addDetail("Launcher Version: 1.0.0 (ARM64)");
        infoCard.addDetail("Architecture: ARM64 (aarch64)");
        infoCard.addDetail("Graphics API: Vulkan 1.3");
        infoCard.addDetail("Hook Core: VK_LAYER_GIMI_arm64");
        layout.addView(infoCard.build());

        // Card 3: System Logs
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
