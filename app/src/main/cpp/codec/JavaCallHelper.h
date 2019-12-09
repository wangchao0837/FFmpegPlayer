//
// Created by 7invensun on 2019-11-25.
//

#ifndef FFMPEGPLAYER_JAVACALLHELPER_H
#define FFMPEGPLAYER_JAVACALLHELPER_H

#include <jni.h>
#include <sys/types.h>
#include "macro.h"

class JavaCallHelper {
public:

    JavaCallHelper(_JavaVM *vm, _JNIEnv *env, _jobject *jobject);

    ~JavaCallHelper();

    void onError(int thread, int code, const char *msg);

    void onPrepare(int thread);

    void onDuration(int thread, int duration);

private:
    _JavaVM *pVm = 0;
    _JNIEnv *pEnv = 0;
    _jobject *pInstance = 0;
    jmethodID onErrorId = 0;
    jmethodID onPrepareId = 0;
    jmethodID getDurationId = 0;
};


#endif //FFMPEGPLAYER_JAVACALLHELPER_H
