#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jni.h>
#include <android/log.h>
#include <string.h>
#include <pthread.h>
#include <sys/ptrace.h>
#include <sys/wait.h>


#define MODULE_NAME "YIBAN_SO"
#define LOGV(...) \
  __android_log_print(ANDROID_LOG_VERBOSE, MODULE_NAME, __VA_ARGS__)
#define LOGD(...) \
  __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGI(...) \
  __android_log_print(ANDROID_LOG_INFO, MODULE_NAME, __VA_ARGS__)
#define LOGW(...) \
  __android_log_print(ANDROID_LOG_WARN, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) \
  __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)
#define LOGF(...) \
  __android_log_print(ANDROID_LOG_FATAL, MODULE_NAME, __VA_ARGS__)


const char *jniClassPath = "com/yiban/app/jni/AntiDebug";

#define BUFF_LEN 1024


typedef struct jni_cache {
    jclass j_buildConfig;
    jfieldID j_debugField;
    jclass j_debug;
    jmethodID j_isDebugConnected;
    JavaVM *vm;
} JNICache;


JNICache mCache;

jint jniVersion = -1;


JNIEnv *getJNIEnv() {
    JavaVM *javaVM = mCache.vm;
    JNIEnv *env;
    jint res = (*javaVM)->GetEnv(javaVM, (void **) &env, JNI_VERSION_1_6);
    if (res != JNI_OK) {
        res = (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);
        if (JNI_OK != res) {
            LOGE("Failed to AttachCurrentThread, ErrorCode = %d", res);
        }
        LOGD("获取JNIEnv错误");
    }
    return env;
}

void trace_pid(char *path, JNIEnv *env, jobject thiz) {
    char buf[BUFF_LEN];
    FILE *fp;
    fp = fopen(path, "r");
    if (fp == NULL) {
        LOGE("status open failed:[error:%d, desc:%s]", errno, strerror(errno));
        return;
    }

    while (fgets(buf, BUFF_LEN, fp)) {
        if (strstr(buf, "TracerPid")) {
            //字符串分割
            strtok(buf, ":");
            const char *result = strtok(NULL, ":");
            //将字符串转化成整形
            int tracerPid = atoi(result);
            LOGD("%s, TracerPid:%d", path, tracerPid);
            if (tracerPid > 0) {
                //最后在判断一遍 是什么模式
                jboolean isDebug = (*env)->GetStaticBooleanField(env, mCache.j_buildConfig,
                                                                 mCache.j_debugField);
                if (isDebug) {
                    LOGD("程序目前处于debug模式，程序不退出");
                    exit(1);
                } else {
                    LOGD("程序目前处于release模式，程序将要退出");

                }
            }
            break;
        }
    }

    fclose(fp);
}


jboolean usbDebugConnected(JNIEnv *env, jobject thiz) {
    jboolean connected = (*env)->CallStaticBooleanMethod(env, mCache.j_debug,
                                                         mCache.j_isDebugConnected);
    return connected;
}


void checkTracerId(JNIEnv *env, jobject thiz) {
    int pid = getpid();
    char path[BUFF_LEN];
    sprintf(path, "/proc/%d/status", pid);
    trace_pid(path, env, thiz);

}

void tarce_pid_monitor(JNIEnv *env, jobject thiz) {

    jboolean isDebug = (*env)->GetStaticBooleanField(env, mCache.j_buildConfig,
                                                     mCache.j_debugField);
    if (isDebug) {
        LOGD("程序目前处于debug模式，程序将退出");
//        exit(1);
    } else {
        LOGD("程序目前处于release模式，程序运行正常");
    }
    //首先判断是否处于连接状态

    jboolean usbConnected = usbDebugConnected(env, thiz);
    if (usbConnected) {
        // 如果连接 ，在判断proc/self/status 中的TarcePid
        checkTracerId(env, thiz);
    }
}

const JNINativeMethod methods[] = {
        "startAntiDebug", "(Landroid/content/Context;)V", tarce_pid_monitor
};


JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    memset(&mCache, 0, sizeof(mCache));

    LOGD("JNI_OnLoad = %s", "执行 JNI_OnLoad");
    //通过Java虚拟机去创建JNIEnv

    if ((*vm)->GetEnv(vm, (void **) (&env), JNI_VERSION_1_6) == JNI_OK) {
        jniVersion = JNI_VERSION_1_6;
    }

    if (env == NULL) {
        if ((*vm)->GetEnv(vm, (void **) (&env), JNI_VERSION_1_4) == JNI_OK) {
            jniVersion = JNI_VERSION_1_4;
        }
    }

    if (env == NULL) {
        if ((*vm)->GetEnv(vm, (void **) (&env), JNI_VERSION_1_2) == JNI_OK) {
            jniVersion = JNI_VERSION_1_2;
        }
    }

    if (jniVersion == -1) {
        return jniVersion;
    }

    jclass j_debug = (*env)->FindClass(env, "android/os/Debug");
    mCache.j_debug = (*env)->NewGlobalRef(env, j_debug);

    mCache.j_isDebugConnected = (*env)->GetStaticMethodID(env, mCache.j_debug,
                                                          "isDebuggerConnected", "()Z");
    LOGE("status open failed:[error:%d, desc:%s]", errno, strerror(errno));

    jclass j_buildConfig = (*env)->FindClass(env, "com/yiban/ybantidebug/BuildConfig");
    jfieldID j_debugField = (*env)->GetStaticFieldID(env, j_buildConfig, "DEBUG", "Z");


    mCache.j_buildConfig = (*env)->NewGlobalRef(env, j_buildConfig);
    mCache.j_debugField = j_debugField;

    //找到动态注册的JNI类

    jclass jniClass = (*env)->FindClass(env, jniClassPath);

    (*env)->RegisterNatives(env, jniClass, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return jniVersion;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    JNIEnv *env = getJNIEnv();
    if (env) {
        (*env)->DeleteGlobalRef(env, mCache.j_debug);
        (*env)->DeleteGlobalRef(env, mCache.j_buildConfig);
    }
}












