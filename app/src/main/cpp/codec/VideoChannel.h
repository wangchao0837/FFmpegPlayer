//
// Created by 7invensun on 2019-11-26.
//

#ifndef FFMPEGPLAYER_VIDEOCHANNEL_H
#define FFMPEGPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"

class BaseChannel;

class VideoChannel : public BaseChannel {

public:

    VideoChannel(int i, AVCodecContext *avCodecContext, AVRational time_base, double fps);

    ~VideoChannel();

    void p_render();

    void p_volume() {

    }

    void stop();

    void setAudioChannel(AudioChannel *audioChannel1) {
        audioChannel = audioChannel1;
    }

    static void onAVFrameResult(AVFrame *avFrame);


public:
    SwsContext *swsContext = 0;
    uint8_t *dst_data[4];
    int dst_linesize[4];
    double fps;
    double frame_delay;
    AudioChannel *audioChannel = 0;

//    RenderFrameCallback callback = 0;
//    AVCodecContext *avCodecContext = 0;
//    SwsContext *swsContext = 0;
//    uint8_t *dst_data[4];
//    int dst_linesize[4];

};


#endif //FFMPEGPLAYER_VIDEOCHANNEL_H
