//
// Created by 7invensun on 2019-11-27.
//

#ifndef FFMPEGPLAYER_BASECHANNEL_H
#define FFMPEGPLAYER_BASECHANNEL_H

#include "SafeQueue.h"
#include <pthread.h>
#include "macro.h"
#include "JavaCallHelper.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>

};

class VideoChannel;

class AudioChannel;

typedef void (*RenderFrameCallback)(uint8_t *, int, int, int);

class BaseChannel {

public:

    BaseChannel(int i, AVCodecContext *avCodecContext, AVRational time_base);

    virtual ~BaseChannel();

    void play();

    virtual void stop() = 0;

    void p_decode();

    virtual void p_render() = 0;

    virtual void p_volume() = 0;


    void clear() {
        frames.clear();
        packets.clear();
    }

    void stopWork() {
        frames.setWork(0);
        packets.setWork(0);
    }

    void startWork() {
        frames.setWork(1);
        packets.setWork(1);
    }


    static void releaseAvPackage(AVPacket **pPacket);

    static void releaseAvFrame(AVFrame **frame);


    static void syncAvFrame(queue<AVFrame *> &q);

    void setVideoRenderCallBack(RenderFrameCallback callback) {
        this->callback = callback;
    };
//    void setAvFrameCallBack(FrameCallBack frameCallBack1) ;

public:
    RenderFrameCallback callback = 0;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    int streamId;
    AVCodecContext *avCodecContext = 0;
    bool isPlaying;
    pthread_t pid_render;
    AVRational time_base;



private:
    pthread_t pid_decode;
//    FrameCallBack frameCallBack = 0;
private:

};


#endif //FFMPEGPLAYER_BASECHANNEL_H
