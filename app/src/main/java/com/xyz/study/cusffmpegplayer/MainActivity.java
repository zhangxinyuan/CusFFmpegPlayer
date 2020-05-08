package com.xyz.study.cusffmpegplayer;

import android.os.Bundle;
import android.os.Environment;
import android.view.SurfaceView;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private FFPlayer ffPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = findViewById(R.id.surfaceview);
        ffPlayer = new FFPlayer();
        ffPlayer.setSurfaceView(surfaceView);
    }

    public void start(View view) {
        File file = new File(Environment.getExternalStorageDirectory(), "demo.mp4");
        ffPlayer.start(file.getAbsolutePath());
    }
}
