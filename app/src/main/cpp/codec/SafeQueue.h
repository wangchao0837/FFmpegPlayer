//
// Created by 7invensun on 2019-11-27.
//

#ifndef FFMPEGPLAYER_SAFEQUEUE_H
#define FFMPEGPLAYER_SAFEQUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

template<typename T>

class SafeQueue {

    typedef void (*ReleaseCallBack)(T *);

    typedef void (*SyncHandle)(queue<T> &);

public:
    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    int push(T new_value) {
        int ret = 0;
        if (work) {
            pthread_mutex_lock(&mutex);
            q.push(new_value);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            ret = 1;
        }else{
            releaseCallBack(&new_value);
        }
        return ret;
    }


    int pop(T &value) {
        int ret = 0;
        if(work){
            pthread_mutex_lock(&mutex);
            while (work && q.empty()) {
                pthread_cond_wait(&cond, &mutex);
            }
            if (!q.empty()) {
                value = q.front();
                q.pop();
                ret = 1;
            }
            pthread_mutex_unlock(&mutex);
        }

        return ret;
    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        int size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            releaseCallBack(&value);
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallBack(ReleaseCallBack releaseCallBack1) {
        this->releaseCallBack = releaseCallBack1;
    }

    void setSyncHandle(SyncHandle syncHandle1) {
        this->syncHandle = syncHandle1;
    }

    void sync(){
        pthread_mutex_lock(&mutex);
        syncHandle(q);
        pthread_mutex_unlock(&mutex);
    }

    SafeQueue() {
        pthread_mutex_init(&mutex, 0);
        pthread_cond_init(&cond, 0);
    }

    void setWork(int work) {
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

private:
    pthread_t pid;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    queue<T> q;
    bool work;
    ReleaseCallBack releaseCallBack;
    SyncHandle syncHandle;
};

#endif //FFMPEGPLAYER_SAFEQUEUE_H
