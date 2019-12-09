package com.example.ffmpegplayer;

import android.view.SurfaceView;

public interface IPlayer {
    void setDataSource(String dataSource);

    void setSurface(SurfaceView surfaceView);

    void prepare();

    void start();

    void stop();

    void release();

    int getDuration();

    interface OnPrepareListener {
        void onPrepare();
    }

    interface OnErrorListener {
        void onError(int code, String message);
    }

    interface OnProgressListener{
        void onProgress(int progress);
    }
}
