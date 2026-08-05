package com.gimi.launcher.jni;

public class ModInfo {
    private String name;
    private String path;
    private boolean enabled;
    private int iniCount;
    private int textureCount;

    public ModInfo(String name, String path, boolean enabled, int iniCount, int textureCount) {
        this.name = name;
        this.path = path;
        this.enabled = enabled;
        this.iniCount = iniCount;
        this.textureCount = textureCount;
    }

    public String getName() {
        return name;
    }

    public String getPath() {
        return path;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    public int getIniCount() {
        return iniCount;
    }

    public int getTextureCount() {
        return textureCount;
    }
}
