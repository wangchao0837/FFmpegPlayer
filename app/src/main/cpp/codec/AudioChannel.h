//
// Created by 7invensun on 2019-11-26.
//

#ifndef FFMPEGPLAYER_AUDIOCHANNEL_H
#define FFMPEGPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswresample/swresample.h>
};

class BaseChannel;

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int i, AVCodecContext *avCodecContext, AVRational time_base,JavaCallHelper *javaCallHelper,int duration);

    void _playVoice();

    void p_render() {

    };

    void stop();

    void p_volume();

    int getPcm();




    ~AudioChannel();

public:
    AVCodecContext *avCodecContext;
    uint8_t *data = 0;
    int out_channels;
    int out_samplesize;
    int out_sample_rate;
    double audio_clock;
private:
    /**
    * OpenSL ES
    */
    // 引擎与引擎接口
    SLObjectItf engineObject = 0;
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerInterface = 0;

    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueueInterface = 0;
    JavaCallHelper *javaCallHelper;
    int duration = 0;
    //重采样
    SwrContext *swrContext = 0;


};


#endif //FFMPEGPLAYER_AUDIOCHANNEL_H
