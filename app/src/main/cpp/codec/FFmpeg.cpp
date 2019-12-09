//
// Created by 7invensun on 2019-11-25.
//

#include "FFmpeg.h"
#include <android/log.h>
#include <sys/types.h>
#include <pthread.h>
#include "FFmpeg.h"

FFmpeg::FFmpeg(JavaCallHelper *callHelper, const char *datasource) : callHelper(callHelper) {
    this->datasource = new char[strlen(datasource) + 1];
    stpcpy(this->datasource, datasource);
    pthread_mutex_init(&seekMutext, 0);
}

FFmpeg::~FFmpeg() {
    DELETE(datasource);
    DELETE(callHelper);
    if (formatContext) {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = 0;
    }
    pthread_mutex_destroy(&seekMutext);

    DELETE(videoChannel);
    DELETE(audioChannel);
}


void *task_prepare(void *args) {
    FFmpeg *fFmpeg = static_cast<FFmpeg *>(args);
    fFmpeg->p_prepare();
    return 0;
}


void *task_FFDecode(void *args) {
    FFmpeg *fFmpeg = static_cast<FFmpeg *>(args);
    fFmpeg->p_decode();
    return 0;
}

void *task_stop(void *args) {
    FFmpeg *fFmpeg = static_cast<FFmpeg *>(args);
    fFmpeg->p_stop();
    return 0;
}

void FFmpeg::prepare() {
    pthread_create(&pid_prepare, 0, task_prepare, this);
}

void FFmpeg::p_prepare() {
    isPlaying = 1;
    duration = 0;
    //初始化网络
    int ret = avformat_network_init();
    LOGE("初始化网络:%s", av_err2str(ret));
    formatContext = avformat_alloc_context();
    AVDictionary *opts = 0;
    //设置超时时间
    av_dict_set(&opts, "timeout", "3000000", 0);
    //datasource:传入的播放地址 可以为本地文件，也可以为网络地址
    //打开网络地址,赋值formatContext
    ret = avformat_open_input(&formatContext, datasource, 0, &opts);

    av_dict_free(&opts);
    opts = 0;
    if (ret) {
        LOGE("打开媒体失败:%s", av_err2str(ret));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL, av_err2str(ret));
        return;
    }
    //拿到总时长，对于本地文件和点播，有效。对于直播地址，为0
    duration = formatContext->duration / 1000000;
    LOGE("duration is :%d", duration);
    //获取流信息，赋值formatContext
    ret = avformat_find_stream_info(formatContext, 0);
    if (ret < 0) {
        LOGE("查找流失败:%s", av_err2str(ret));
        if (isPlaying) {
            callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS, av_err2str(ret));
            isPlaying = 0;
        }
        return;
    }

    //formatContext->nb_streams 表示有几条stream，一般有：audiostream 和 videostream
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];

        //stream->codecpar : 编码参数
        AVCodecParameters *codecPar = stream->codecpar;
        //查找解码器
        AVCodec *avCodec = avcodec_find_decoder(codecPar->codec_id);

        if (NULL == avCodec) {
            LOGE("查找解码器失败:%s", av_err2str(ret));
            if (isPlaying) {
                callHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL, av_err2str(ret));
                isPlaying = 0;
            }
            return;
        }
        //创建解码上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(avCodec);

        if (codecContext == NULL) {
            LOGE("创建解码上下文失败:%s", av_err2str(ret));
            if (isPlaying) {
                callHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL, av_err2str(ret));
                isPlaying = 0;
            }
            return;
        }
        //设置解码上下文参数
        ret = avcodec_parameters_to_context(codecContext, codecPar);
        if (ret < 0) {
            LOGE("设置解码上下文参数失败:%s", av_err2str(ret));
            if (isPlaying) {
                callHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL,
                                    av_err2str(ret));
                isPlaying = 0;
            }
            return;
        }
        //打开解码器
        ret = avcodec_open2(codecContext, avCodec, 0);
        if (ret != 0) {
            LOGE("打开解码器失败:%s", av_err2str(ret));
            if (isPlaying) {
                callHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL, av_err2str(ret));
                isPlaying = 0;
            }
            return;
        }

        if (codecPar->codec_type == AVMEDIA_TYPE_VIDEO) {

            AVRational frameRate = stream->avg_frame_rate;
            //获取视频帧率
            double fps = av_q2d(frameRate);
            videoChannel = new VideoChannel(i, codecContext, stream->time_base, fps);
        } else if (codecPar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(i, codecContext, stream->time_base, callHelper,
                                            duration);
        }

    }

    if (!audioChannel && !videoChannel) {
        LOGE("未找到音视频流");
        const char *msg = "未找到音视频流";
        if (isPlaying) {
            callHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA, msg);
            isPlaying = 0;
        }
        return;
    }
    if (isPlaying) {
        //通知java层，prepare成功，可以start解码播放了
        callHelper->onPrepare(THREAD_CHILD);
    }

}

void FFmpeg::start() {
    if (videoChannel) {
        videoChannel->packets.setWork(1);
        videoChannel->frames.setWork(1);
        videoChannel->play();
        if (audioChannel) {
            videoChannel->setAudioChannel(audioChannel);
        }
    }

    if (audioChannel) {
        audioChannel->packets.setWork(1);
        audioChannel->frames.setWork(1);
        audioChannel->play();
    }

    pthread_create(&pid_decode, 0, task_FFDecode, this);
}


void FFmpeg::stop() {

    pthread_create(&pid_stop, 0, task_stop, this);

}


void FFmpeg::p_stop() {
    isPlaying = 0;
    pthread_join(pid_prepare, 0);
    pthread_join(pid_decode, 0);

    videoChannel->stop();
    DELETE(videoChannel);
    audioChannel->stop();
    DELETE(audioChannel);

    FFmpeg *ffmepg = this;
    DELETE(ffmepg);
}

void FFmpeg::p_decode() {
    int ret;
    while (isPlaying) {
        if (audioChannel && audioChannel->frames.size() > 100) {
            av_usleep(100 * 1000);

        }

        if (videoChannel && videoChannel->frames.size() > 50) {
            av_usleep(100 * 1000);
        }

        if (!isPlaying) {
            break;
        }

        pthread_mutex_lock(&seekMutext);

        AVPacket *packet = av_packet_alloc();

        if (packet == 0) {
            continue;
        }

        ret = av_read_frame(formatContext, packet);
        if (ret == 0) {
            if (videoChannel && packet->stream_index == videoChannel->streamId) {
                //将视频Packet交给videochannel处理
               videoChannel->packets.push(packet);

            } else if (audioChannel && packet->stream_index == audioChannel->streamId) {
                //将音频Packet交给videochannel处理
                audioChannel->packets.push(packet);
            }
        }
        //ACERROR_EOF：读取结束，表示读取到文件末尾标志
        else if (ret == AVERROR_EOF) {
            while (isPlaying) {
                if (videoChannel->packets.empty() && audioChannel->packets.empty()) {
                    break;
                }
                //等待10毫秒再次检查
                av_usleep(10 * 1000);
            }
            DELETE(packet)
            pthread_mutex_unlock(&seekMutext);

            continue;
        } else {
            DELETE(packet)
            pthread_mutex_unlock(&seekMutext);
            break;
        }
        pthread_mutex_unlock(&seekMutext);


    }


}

void FFmpeg::setRenderFrameCallback(RenderFrameCallback callback) {
    videoChannel->setVideoRenderCallBack(callback);
}

void FFmpeg::seekToPosition(int i) {
    if (i < 0 || i >= duration) {
        return;
    }

    if (!audioChannel && !videoChannel) {
        return;
    }

    if (!formatContext) {
        return;
    }

    pthread_mutex_lock(&seekMutext);

    if (audioChannel) {
        audioChannel->stopWork();
        audioChannel->clear();
        audioChannel->startWork();
    }

    if (videoChannel) {
        videoChannel->stopWork();
        videoChannel->clear();
        videoChannel->startWork();
    }

    int64_t seek = i * AV_TIME_BASE;
    //seek ，参数-1表示音频和视频都执行seek。
    avformat_seek_file(formatContext, -1, INT64_MIN, seek, INT64_MAX, 0);

//    int64_t seektime = av_rescale_q(seek, av_get_time_base_q(), videoChannel->time_base);
//    int64_t a_seektime = av_rescale_q(seek, av_get_time_base_q(), audioChannel->time_base);
//
//    avformat_seek_file(formatContext, videoChannel->streamId, INT64_MIN, seektime, INT64_MAX, 0);
////
//    avformat_seek_file(formatContext, audioChannel->streamId, INT64_MIN, a_seektime, INT64_MAX, 0);
//


    avcodec_flush_buffers(videoChannel->avCodecContext);
    avcodec_flush_buffers(audioChannel->avCodecContext);

//    int ret = av_seek_frame(formatContext,-1,seek,AVSEEK_FLAG_BACKWARD);
//    av_seek_frame(formatContext, videoChannel->streamId, (int64_t) (i /av_q2d(videoChannel->time_base)), AVSEEK_FLAG_BACKWARD);
//    av_seek_frame(formatContext, audioChannel->streamId, (int64_t) (i /av_q2d(audioChannel->time_base)), AVSEEK_FLAG_BACKWARD);


    pthread_mutex_unlock(&seekMutext);
}





