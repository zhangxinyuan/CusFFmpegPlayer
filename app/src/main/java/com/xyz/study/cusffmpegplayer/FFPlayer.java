package com.xyz.study.cusffmpegplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class FFPlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native_player");
    }

    private SurfaceHolder surfaceHolder;
    private Surface surface;

    public void setSurfaceView(SurfaceView surfaceView) {
        if (surfaceHolder == null) {
            surfaceHolder = surfaceView.getHolder();
        } else {
            surfaceHolder.removeCallback(this);
        }
        surfaceHolder.addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        this.surface = holder.getSurface();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void start(String path) {
        native_start(path, surface);
    }

    public native void native_start(String path, Surface surface);
}
