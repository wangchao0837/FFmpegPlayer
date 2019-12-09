#include <jni.h>
#include <string>
#include "codec/FFmpeg.h"
#include <android/native_window_jni.h>

JavaVM *javaVM = NULL;
FFmpeg *fFmpeg = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
ANativeWindow *window = 0;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    LOGE("jin_onLoad");
    return JNI_VERSION_1_6;
}

void render(uint8_t *data, int lineszie, int w, int h) {
    //互斥锁
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    //设置windowbuffer的大小和格式
    ANativeWindow_setBuffersGeometry(window, w, h, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;
    if (ANativeWindow_lock(window, &windowBuffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }

    uint8_t *dst_data = static_cast<uint8_t *>(windowBuffer.bits);

    int dst_linesize = windowBuffer.stride * 4;

    for (int i = 0; i < windowBuffer.height; ++i) {
        //一行一行拷贝到windowBuffer.bits 中
        memcpy(dst_data + i * dst_linesize, data + i * lineszie, dst_linesize);
    }

    ANativeWindow_unlockAndPost(window);

    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegplayer_Player_native_1prepare(JNIEnv *env, jobject thiz,
                                                     jstring data_source) {

    const char *dataSource = env->GetStringUTFChars(data_source, 0);
    JavaCallHelper *javaCallHelper = new JavaCallHelper(javaVM, env, thiz);
    fFmpeg = new FFmpeg(javaCallHelper, dataSource);
    fFmpeg->prepare();
    env->ReleaseStringUTFChars(data_source, dataSource);

}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegplayer_Player_native_1start(JNIEnv *env, jobject thiz) {
    fFmpeg->setRenderFrameCallback(render);
    fFmpeg->start();

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegplayer_Player_native_1surface(JNIEnv *env, jobject thiz, jobject surface) {
    pthread_mutex_lock(&mutex);

    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }


    window = ANativeWindow_fromSurface(env, surface);

    pthread_mutex_unlock(&mutex);

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegplayer_Player_native_1stop(JNIEnv *env, jobject thiz) {
    if (fFmpeg) {
        fFmpeg->stop();
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegplayer_Player_native_1release(JNIEnv *env, jobject thiz) {
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
}extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ffmpegplayer_Player_native_1duration(JNIEnv *env, jobject thiz) {
    int duration = 0;
    if(fFmpeg){
        duration = fFmpeg->getDuration();
    }
    return duration;
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegplayer_Player_native_1seekTo(JNIEnv *env, jobject thiz, jint seek) {
    LOGE("AAAA begin");

    if(fFmpeg){
        fFmpeg->seekToPosition(seek);
    }

    LOGE("AAAA end");
}