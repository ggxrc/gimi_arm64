package com.gimi.launcher;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.LinearLayout;
import android.view.Gravity;

public class MainActivity extends Activity {
    static {
        try {
            System.loadLibrary("gimi_arm64");
        } catch (UnsatisfiedLinkError e) {
            // Library loaded by Vulkan layer or standalone app
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setGravity(Gravity.CENTER);

        TextView titleView = new TextView(this);
        titleView.setText("GIMI Launcher (ARM64)");
        titleView.setTextSize(24);
        titleView.setGravity(Gravity.CENTER);
        layout.addView(titleView);

        TextView statusView = new TextView(this);
        statusView.setText("Vulkan Layer Hook & Mod Manager Active");
        statusView.setTextSize(16);
        statusView.setGravity(Gravity.CENTER);
        layout.addView(statusView);

        setContentView(layout);
    }
}
