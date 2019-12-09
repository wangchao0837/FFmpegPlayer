package com.example.ffmpegplayer;

import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import java.io.FileDescriptor;
import java.io.IOException;
import java.io.InputStream;

public class LaunchActivity extends AppCompatActivity {

    private String path;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_launch);
    }

    public void onJump(View view) {
        path = Environment.getExternalStorageDirectory().getAbsolutePath();
        path = path + "/mvmusic.mp4";

        Intent intent = new Intent(this, MainActivity.class);
        intent.putExtra("path", path);
        startActivity(intent);
    }

    public void onJumpRemote(View view) {
        path = "http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8";
        Intent intent = new Intent(this, MainActivity.class);
        intent.putExtra("path", path);
        startActivity(intent);
    }
}
