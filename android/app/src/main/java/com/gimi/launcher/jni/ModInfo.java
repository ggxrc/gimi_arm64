package com.gimi.launcher.jni;

public class ModInfo {
    public String name;
    public String path;
    public boolean isEnabled;
    public int iniCount;
    public int textureCount;

    public ModInfo() {
        this.name = "";
        this.path = "";
        this.isEnabled = false;
        this.iniCount = 0;
        this.textureCount = 0;
    }

    public ModInfo(String name, String path, boolean isEnabled, int iniCount, int textureCount) {
        this.name = name;
        this.path = path;
        this.isEnabled = isEnabled;
        this.iniCount = iniCount;
        this.textureCount = textureCount;
    }

    public String getName() { return name; }
    public String getPath() { return path; }
    public boolean isEnabled() { return isEnabled; }
    public int getIniCount() { return iniCount; }
    public int getTextureCount() { return textureCount; }
}
