//
// Created by 7invensun on 2019-11-25.
//

#include "JavaCallHelper.h"
#include "SafeQueue.h"
JavaCallHelper::JavaCallHelper(_JavaVM *vm, _JNIEnv *env, _jobject *jobject) : pVm(vm), pEnv(env) {

    pInstance = env->NewGlobalRef(jobject);
    jclass classInstance = pEnv->GetObjectClass(pInstance);
    onErrorId = pEnv->GetMethodID(classInstance, "onError", "(I)V");
    onPrepareId = pEnv->GetMethodID(classInstance, "onPrepare", "()V");
    getDurationId = pEnv->GetMethodID(classInstance, "onProgressResult", "(I)V");


}

JavaCallHelper::~JavaCallHelper() {
    pEnv->DeleteGlobalRef(pInstance);
}

void JavaCallHelper::onError(int thread, int code, const char *msg) {
    if (thread == THREAD_MAIN) {
        pEnv->CallVoidMethod(pInstance, onErrorId, code);
    } else {
        JNIEnv *env;
        jint ret = pVm->AttachCurrentThread(&env, 0);

        if (ret != JNI_OK) {
            return;
        }
        env->CallVoidMethod(pInstance, onErrorId, code );

        pVm->DetachCurrentThread();
    }
}


void JavaCallHelper::onPrepare(int thread) {
    if (thread == THREAD_MAIN) {
        pEnv->CallVoidMethod(pInstance, onPrepareId);
    } else {
        JNIEnv *env;
        jint ret = pVm->AttachCurrentThread(&env, 0);
        if (ret != JNI_OK) {
            return;
        }
        env->CallVoidMethod(pInstance, onPrepareId);
        pVm->DetachCurrentThread();

    }
}


void JavaCallHelper::onDuration(int thread,int duration){
    if (thread == THREAD_MAIN) {
        pEnv->CallVoidMethod(pInstance, getDurationId,duration);
    } else {
        JNIEnv *env;
        jint ret = pVm->AttachCurrentThread(&env, 0);
        if (ret != JNI_OK) {
            return;
        }
        env->CallVoidMethod(pInstance, getDurationId,duration);
        pVm->DetachCurrentThread();

    }
}

