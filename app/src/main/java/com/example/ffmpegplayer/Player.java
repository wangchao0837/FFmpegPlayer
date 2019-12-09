package com.example.ffmpegplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class Player implements IPlayer {

    static {
        System.loadLibrary("ffmpeg-native");
    }

    private OnPrepareListener onPrepareListener;
    private OnErrorListener onErrorListener;
    private String dataSource;
    private SurfaceView surfaceView;
    private OnProgressListener onProgressListener;

    @Override
    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    @Override
    public void setSurface(SurfaceView surfaceView) {
        this.surfaceView = surfaceView;
        surfaceView.getHolder().addCallback(callback);
    }

    @Override
    public void prepare() {
        native_prepare(dataSource);
    }

    @Override
    public void start() {
        native_start();
    }

    @Override
    public void stop() {
        native_stop();
    }

    @Override
    public void release() {
        native_release();
    }

    @Override
    public int getDuration() {
        return native_duration();
    }

    private void onError(int code) {
        if (onErrorListener != null) onErrorListener.onError(code, "");
    }

    public void seekTo(int seek) {
        native_seekTo(seek);
    }



    private void onProgressResult(int progress) {
        if (onProgressListener != null) onProgressListener.onProgress(progress);
    }

    private void onPrepare() {
        if (onPrepareListener != null) onPrepareListener.onPrepare();
    }


    public void setOnPrepareListener(OnPrepareListener onPrepareListener) {
        this.onPrepareListener = onPrepareListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }


    SurfaceHolder.Callback callback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceCreated(SurfaceHolder holder) {

        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            native_surface(holder.getSurface());
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
        }
    };

    public native void native_prepare(String dataSource);

    public native void native_start();

    public native void native_surface(Surface surface);

    public native void native_stop();

    public native void native_release();

    public native int native_duration();
    public native void native_seekTo(int seek);

}
