// ─────────────────────────────────────────────────────────────────────────────
// gimi_arm64 — Android Launcher: Native JNI Bridge Implementation
// ─────────────────────────────────────────────────────────────────────────────

#include <jni.h>
#include "launcher/mod_manager_service.h"
#include "launcher/shizuku_layer_injector.h"
#include "utils/logger.h"

static jobjectArray scan_mods_internal(JNIEnv* env, jstring path, const char* className) {
    std::string mods_path = "/sdcard/GIMI/Mods";
    if (path != nullptr) {
        const char* path_str = env->GetStringUTFChars(path, nullptr);
        if (path_str != nullptr) {
            mods_path = path_str;
            env->ReleaseStringUTFChars(path, path_str);
        }
    }

    auto mods = gimi::ModManagerService::instance().scan_mods(mods_path);

    jclass modInfoClass = env->FindClass(className);
    if (!modInfoClass) {
        LOGE("JNI: Could not find class %s", className);
        return nullptr;
    }

    jmethodID modInfoInit = env->GetMethodID(
        modInfoClass,
        "<init>",
        "(Ljava/lang/String;Ljava/lang/String;ZII)V"
    );
    if (!modInfoInit) {
        LOGE("JNI: Could not find ModInfo constructor for %s", className);
        return nullptr;
    }

    jobjectArray result = env->NewObjectArray(
        static_cast<jsize>(mods.size()),
        modInfoClass,
        nullptr
    );

    for (size_t i = 0; i < mods.size(); ++i) {
        const auto& mod = mods[i];
        jstring jName = env->NewStringUTF(mod.name.c_str());
        jstring jPath = env->NewStringUTF(mod.path.c_str());
        jboolean jEnabled = mod.enabled ? JNI_TRUE : JNI_FALSE;
        jint jIniCount = static_cast<jint>(mod.resource_count);
        jint jTextureCount = 0;

        jobject modObj = env->NewObject(
            modInfoClass,
            modInfoInit,
            jName,
            jPath,
            jEnabled,
            jIniCount,
            jTextureCount
        );

        env->SetObjectArrayElement(result, static_cast<jsize>(i), modObj);

        env->DeleteLocalRef(jName);
        env->DeleteLocalRef(jPath);
        env->DeleteLocalRef(modObj);
    }

    return result;
}

static jboolean toggle_mod_internal(JNIEnv* env, jstring modPath, jboolean enable) {
    if (modPath == nullptr) return JNI_FALSE;
    const char* path_str = env->GetStringUTFChars(modPath, nullptr);
    if (path_str == nullptr) return JNI_FALSE;

    bool res = gimi::ModManagerService::instance().set_mod_enabled(path_str, enable == JNI_TRUE);
    env->ReleaseStringUTFChars(modPath, path_str);

    return res ? JNI_TRUE : JNI_FALSE;
}

static jint inject_layer_internal(JNIEnv* env, jstring packageName) {
    std::string pkg;
    if (packageName != nullptr) {
        const char* pkg_str = env->GetStringUTFChars(packageName, nullptr);
        if (pkg_str != nullptr) {
            pkg = pkg_str;
            env->ReleaseStringUTFChars(packageName, pkg_str);
        }
    }

    gimi::GameDistribution dist = gimi::GameDistribution::PlayStore;
    if (pkg == "com.miHoYo.GI.samsung") {
        dist = gimi::GameDistribution::GalaxyStore;
    } else if (pkg == "com.yuanshen.site") {
        dist = gimi::GameDistribution::ChinaServer;
    } else if (pkg == "com.miHoYo.ys.bilibili") {
        dist = gimi::GameDistribution::Bilibili;
    }

    bool success = gimi::ShizukuLayerInjector::instance().enable_layer(dist);
    return success ? 0 : -1;
}

static jint get_layer_status_internal() {
    bool available = gimi::ShizukuLayerInjector::instance().is_shizuku_available();
    return available ? 1 : 0;
}

extern "C" {

// ─── GimiNativeBridge bindings ────────────────────────────────────────────────
JNIEXPORT jobjectArray JNICALL
Java_com_gimi_launcher_jni_GimiNativeBridge_nativeScanMods(JNIEnv* env, jobject, jstring path) {
    return scan_mods_internal(env, path, "com/gimi/launcher/jni/ModInfo");
}

JNIEXPORT jboolean JNICALL
Java_com_gimi_launcher_jni_GimiNativeBridge_nativeToggleMod(JNIEnv* env, jobject, jstring modPath, jboolean enable) {
    return toggle_mod_internal(env, modPath, enable);
}

JNIEXPORT jint JNICALL
Java_com_gimi_launcher_jni_GimiNativeBridge_nativeInjectLayer(JNIEnv* env, jobject, jstring packageName) {
    return inject_layer_internal(env, packageName);
}

JNIEXPORT jint JNICALL
Java_com_gimi_launcher_jni_GimiNativeBridge_nativeGetLayerStatus(JNIEnv*, jobject) {
    return get_layer_status_internal();
}

// ─── GimiNativeBridgeJava bindings ────────────────────────────────────────────
JNIEXPORT jobjectArray JNICALL
Java_com_gimi_launcher_jni_GimiNativeBridgeJava_nativeScanMods(JNIEnv* env, jobject, jstring path) {
    return scan_mods_internal(env, path, "com/gimi/launcher/jni/ModInfoJava");
}

JNIEXPORT jboolean JNICALL
Java_com_gimi_launcher_jni_GimiNativeBridgeJava_nativeToggleMod(JNIEnv* env, jobject, jstring modPath, jboolean enable) {
    return toggle_mod_internal(env, modPath, enable);
}

JNIEXPORT jint JNICALL
Java_com_gimi_launcher_jni_GimiNativeBridgeJava_nativeInjectLayer(JNIEnv* env, jobject, jstring packageName) {
    return inject_layer_internal(env, packageName);
}

JNIEXPORT jint JNICALL
Java_com_gimi_launcher_jni_GimiNativeBridgeJava_nativeGetLayerStatus(JNIEnv*, jobject) {
    return get_layer_status_internal();
}

} // extern "C"
