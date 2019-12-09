//
// Created by 7invensun on 2019-11-25.
//

#ifndef FFMPEGPLAYER_FFMPEG_H
#define FFMPEGPLAYER_FFMPEG_H

#include <sys/types.h>
#include "macro.h"
#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"

extern "C" {


#include <libavformat/avformat.h>
};

class FFmpeg {

public:
    typedef void (*RenderFrameCallback)(uint8_t *, int, int, int);

    FFmpeg(JavaCallHelper *callHelper, const char *datasource);

    ~FFmpeg();

    void prepare();

    void p_prepare();

    void p_decode();

    void p_stop();

    void start();

    void stop();

    void release();

    void setRenderFrameCallback(RenderFrameCallback callback);


    int getDuration(){
       return duration;
    };

    void seekToPosition(int i);

private:
    pthread_t pid_prepare;
    pthread_t pid_decode;
    pthread_t pid_stop;
    AVFormatContext *formatContext;
    pthread_mutex_t seekMutext;
    char *datasource;
    JavaCallHelper *callHelper;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    bool isPlaying;
    int duration = 0;
};

#endif //FFMPEGPLAYER_FFMPEG_H
