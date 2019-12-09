//
// Created by 7invensun on 2019-11-26.
//

#include "VideoChannel.h"
#include "AudioChannel.h"

// RenderFrameCallback VideoChannel::callback = 0;
// AVCodecContext *VideoChannel::avCodecContext = 0;
// SwsContext *VideoChannel::swsContext = 0;
// uint8_t *VideoChannel::dst_data[4];
// int VideoChannel::dst_linesize[4];


void VideoChannel::onAVFrameResult(AVFrame *frame) {

//    sws_scale(swsContext, reinterpret_cast<const uint8_t *const *>(frame->data),
//              frame->linesize, 0, avCodecContext->height, dst_data,
//              dst_linesize);
//    callback(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
//
//    releaseAvFrame(&frame);
}


VideoChannel::VideoChannel(int i, AVCodecContext *avCodecContext, AVRational time_base, double fps)
        : BaseChannel(i, avCodecContext, time_base) {
    this->fps = fps;
    frame_delay = 1.0 / fps;
    //目标： RGBA
    //关键帧率间隔
    LOGE("av gop_size %d",avCodecContext->gop_size);
    //初始化SwsContext
    swsContext = sws_getContext(
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
            avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, 0, 0, 0);


    av_image_alloc(dst_data, dst_linesize,
                   avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA, 1);

}

VideoChannel::~VideoChannel() {
    if (swsContext) {
        sws_freeContext(swsContext);
        swsContext = 0;
    }


}

void VideoChannel::p_render() {

    AVFrame *frame = 0;


    while (isPlaying) {

        //队列中取出frame
        int ret = frames.pop(frame);

        if (!isPlaying) {
            if (ret != 0) {
                releaseAvFrame(&frame);
            }
            break;
        }

        if (ret == 0) {
            continue;
        }

        //拿到当前视频帧的时间
        double clock = frame->best_effort_timestamp * av_q2d(time_base);

        //ffmpeg规定，y默认延迟需要加上这个extra_dela
        double extra_delay = frame->repeat_pict / (2 * fps);

        if (!audioChannel) {
//            frame_delay = 1.0 / fps;
            //frame_delay,比如一秒30帧，默认需要延迟0.033秒
            av_usleep((frame_delay + extra_delay) * 1000000);

        } else {
            if (clock == 0) {
                av_usleep((frame_delay + extra_delay) * 1000000);
            } else {
                double audio_clock = audioChannel->audio_clock;

                double diff = audio_clock - clock;

                //音频快了
                if (diff > 0) {
                    if (diff >= 0.04) {
                        releaseAvFrame(&frame);
                        frames.sync();
                        continue;
                    }
                } else {
                    if (fabs(diff) > 0.1) diff = 0;
                    av_usleep((fabs(diff) + frame_delay) * 1000000);
                }
            }
        }


        sws_scale(swsContext, reinterpret_cast<const uint8_t *const *>(frame->data),
                  frame->linesize, 0, avCodecContext->height, dst_data,
                  dst_linesize);
        //callback是一个函数指针，数据交给外层，做预览
        callback(dst_data[0], dst_linesize[0], avCodecContext->width,
                 avCodecContext->height);
        releaseAvFrame(&frame);

    }
    releaseAvFrame(&frame);

}

void VideoChannel::stop() {
    BaseChannel::stop();
    pthread_join(pid_render, 0);

}

