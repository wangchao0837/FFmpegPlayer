package com.example.ffmpegplayer;

import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements IPlayer.OnErrorListener, IPlayer.OnPrepareListener, SeekBar.OnSeekBarChangeListener, IPlayer.OnProgressListener {

    private Player player;
    private Handler handler = new Handler();
    private SurfaceView surfaceView;
    private String path;
    private SeekBar seekBar;
    private int duration;
    private boolean isTouch;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager
                .LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);

        surfaceView = findViewById(R.id.surfaceView);
        seekBar = findViewById(R.id.seekBar);
        player = new Player();
//        path = Environment.getExternalStorageDirectory().getAbsolutePath();
//        path = path + "/mvmusic.mp4";
        path = getIntent().getStringExtra("path");
        //https://www.apple.com/105/media/us/iphone-x/2017/01df5b43-28e4-4848-bf20-490c34a926a7/films/feature/iphone-x-feature-tpl-cc-us-20170912_1920x1080h.mp4
//       path = "https://www.apple.com/105/media/cn/mac/family/2018/46c4b917_abfd_45a3_9b51_4e3054191797/films/bruce/mac-bruce-tpl-cn-2018_1280x720h.mp4";
//       path =  "http://flv3.bn.netease.com/tvmrepo/2018/6/H/9/EDJTRBEH9/SD/EDJTRBEH9-mobile.mp4"
//        path = "http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8";
        player.setDataSource(path);
        player.setSurface(surfaceView);
        player.setOnErrorListener(this);
        player.setOnPrepareListener(this);
        player.setOnProgressListener(this);

    }


    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager
                    .LayoutParams.FLAG_FULLSCREEN);
        } else {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = findViewById(R.id.surfaceView);

//        player.setDataSource(path);
        player.setSurface(surfaceView);
    }


    @Override
    protected void onResume() {
        super.onResume();
        player.prepare();
    }


    @Override
    protected void onStop() {
        super.onStop();
        player.stop();
    }

    @Override
    public void onError(int code, final String message) {
        handler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this, "错误:" + message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    public void onPrepare() {
        duration = player.getDuration();
        Log.e("AAAA", "duration:" + duration);
        handler.post(new Runnable() {
            @Override
            public void run() {
                if (duration <= 0) {
                    seekBar.setVisibility(View.GONE);
                } else {
                    seekBar.setVisibility(View.VISIBLE);
                    seekBar.setOnSeekBarChangeListener(MainActivity.this);
                }
                Toast.makeText(MainActivity.this, "开始播放:", Toast.LENGTH_SHORT).show();
            }
        });
        player.start();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        int seek = seekBar.getProgress()* duration / 100 ;
        Log.e("AAAA", "seekto:" + seek);
        player.seekTo(seek);
        isTouch = false;
    }


    @Override
    public void onProgress(int progress) {
        if (duration > 0 && !isTouch) {
            Log.e("AAAA", "onProgress:" + progress);
            seekBar.setProgress(progress * 100 / duration);
        }
    }
}
