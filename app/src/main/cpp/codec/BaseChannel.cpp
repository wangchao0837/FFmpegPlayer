//
// Created by 7invensun on 2019-11-27.
//

#include "BaseChannel.h"
#include "VideoChannel.h"

void *task_decode(void *args) {
    BaseChannel *baseChannel = static_cast<BaseChannel *>(args);
    baseChannel->p_decode();
    return 0;
}

void *task_render(void *args) {
    BaseChannel *baseChannel = static_cast<BaseChannel *>(args);
    baseChannel->p_render();
    baseChannel->p_volume();

    return 0;
}


BaseChannel::BaseChannel(int i, AVCodecContext *avCodecContext, AVRational time_base) : streamId(i),
                                                                                        avCodecContext(
                                                                                                avCodecContext),
                                                                                        time_base(
                                                                                                time_base) {

    packets.setReleaseCallBack(releaseAvPackage);
    frames.setReleaseCallBack(releaseAvFrame);
    frames.setSyncHandle(syncAvFrame);
}

BaseChannel::~BaseChannel() {


    if (avCodecContext) {
        avcodec_close(avCodecContext);
        avCodecContext = 0;
    }
    DELETE(avCodecContext);
}


void BaseChannel::play() {
    isPlaying = 1;
    pthread_create(&pid_decode, 0, task_decode, this);
    pthread_create(&pid_render, 0, task_render, this);
}


void BaseChannel::stop() {
    isPlaying = 0;
    packets.setWork(0);
    frames.setWork(0);
    packets.clear();
    frames.clear();
    pthread_join(pid_decode, 0);

}

void BaseChannel::p_decode() {
    AVPacket *packet = 0;
    while (isPlaying) {

        int ret = packets.pop(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPackage(&packet);
        if (ret != 0) {
            break;
        }

        //送入编码器成功

        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == AVERROR(EAGAIN)) {
            DELETE(avFrame)
            continue;
        } else if (ret != 0) {
            break;
        }

        //解码完一帧 数据，放入frames 队列中
        frames.push(avFrame);
    }
    releaseAvPackage(&packet);
}


void BaseChannel::releaseAvPackage(AVPacket **pPacket) {
    if (pPacket) {
        av_packet_free(pPacket);
        *pPacket = 0;
    }
}


void BaseChannel::releaseAvFrame(AVFrame **frame) {
    if (frame) {
        av_frame_free(frame);
        //为什么用指针的指针？
        // 指针的指针能够修改传递进来的指针的指向
        *frame = 0;
    }
}

void BaseChannel::syncAvFrame(queue<AVFrame *> &q) {
    if (!q.empty()) {
        AVFrame *avFrame = q.front();
        releaseAvFrame(&avFrame);
        q.pop();
    }
}
