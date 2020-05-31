// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.demo.demo_android_skia;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;
import android.view.MotionEvent;

import org.chromium.base.CommandLine;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;

import org.demo.demo_jni.SkiaCanvas;

/**
 * Activity for managing the Demo shell.
 */
public class DemoAndroidSkiaActivity extends Activity implements SurfaceHolder.Callback,SkiaCanvas.Callback, View.OnTouchListener {

    private static final String TAG = "DemoAndroidSkia";

    public static final String COMMAND_LINE_ARGS_KEY = "commandLineArgs";
    private TextView mTextView;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_activity);

        // Initializing the command line must occur before loading the library.
        if (!CommandLine.isInitialized()) {
            ((DemoAndroidSkiaApplication) getApplication()).initCommandLine();
            String[] commandLineParams = getCommandLineParamsFromIntent(getIntent());
            if (commandLineParams != null) {
                CommandLine.getInstance().appendSwitchesAndArguments(commandLineParams);
            }
        }

        // 加载JNI库
        LibraryLoader.getInstance().ensureInitialized(LibraryProcessType.PROCESS_BROWSER);
        mTextView = (TextView) findViewById(R.id.text);

        SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceViewSoftware);
        surfaceView.getHolder().addCallback(this);
        surfaceView.setOnTouchListener(this);

        surfaceView = (SurfaceView) findViewById(R.id.surfaceViewGL);
        surfaceView.getHolder().addCallback(this);
        surfaceView.setOnTouchListener(this);
    }
    
    private static String[] getCommandLineParamsFromIntent(Intent intent) {
        return intent != null ? intent.getStringArrayExtra(COMMAND_LINE_ARGS_KEY) : null;
    }

    public void onClick(View view) {
      Log.i(TAG, "Call onClick");
    }

    SkiaCanvas mCanvasSoftware;
    SkiaCanvas mCanvasGL;

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
      Log.i(TAG, "surfaceCreated");
      if(mCanvasSoftware == null && ((SurfaceView)findViewById(R.id.surfaceViewSoftware)).getHolder() == holder)
        mCanvasSoftware = new SkiaCanvas(holder.getSurface(),this,false);
      else if(mCanvasGL == null && ((SurfaceView)findViewById(R.id.surfaceViewGL)).getHolder() == holder)
        mCanvasGL = new SkiaCanvas(holder.getSurface(),this,true);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
      Log.i(TAG, "surfaceChanged");
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
      Log.i(TAG, "surfaceDestroyed");
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        int count = event.getPointerCount();
        for (int i = 0; i < count; i++) {
            final float x = event.getX(i);
            final float y = event.getY(i);
            final int owner = event.getPointerId(i);
            int action = event.getAction() & MotionEvent.ACTION_MASK;
            if(v.getId() == R.id.surfaceViewSoftware)
              mCanvasSoftware.onTouch(action,x,y);
            else
              mCanvasGL.onTouch(action,x,y);
        }
        return true;
    }

    @Override
    public void showInfo(String info) {
      mTextView.setText(info);
    }
}
