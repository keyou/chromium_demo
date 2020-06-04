// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.demo.demo_android_skia;

import android.app.Application;
import android.content.Context;
import android.view.Display;
import android.view.WindowManager;
import android.util.Log;
import android.view.Choreographer;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;

import org.chromium.base.ApplicationStatus;
import org.chromium.base.BuildConfig;
import org.chromium.base.CommandLine;
import org.chromium.base.ContextUtils;
import org.chromium.base.PathUtils;
import org.chromium.base.multidex.ChromiumMultiDexInstaller;

/**
 * Entry point for the demo apk application. Handles initialization of
 * information that needs to be shared across the main activity and the child
 * services created.
 */
public class DemoAndroidSkiaApplication extends Application {
  public static final String COMMAND_LINE_FILE = "/data/local/tmp/demo-apk-command-line";
  private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "demo_android_skia";
  private static final String TAG = "DemoAndroidSkiaApplication";

  public DemoAndroidSkiaApplication() {
    try {
      setFinalStatic(Choreographer.class.getDeclaredField("USE_VSYNC"), false);

    } catch (Exception e) {
      Log.e(TAG,e.toString());
    }
  }

  @Override
  protected void attachBaseContext(Context base) {
    super.attachBaseContext(base);
    // SystemProperties.getBoolean("debug.choreographer.vsync");
    boolean isBrowserProcess = !ContextUtils.getProcessName().contains(":");
    ContextUtils.initApplicationContext(this);
    if (isBrowserProcess) {
      if (BuildConfig.IS_MULTIDEX_ENABLED) {
        ChromiumMultiDexInstaller.install(this);
      }
      PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX);
      ApplicationStatus.initialize(this);
    }

    Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
    float refreshRating = display.getRefreshRate();
    Log.e(TAG, "Refresh Rating: " + refreshRating);
  }

  public void initCommandLine() {
    if (!CommandLine.isInitialized()) {
      CommandLine.initFromFile(COMMAND_LINE_FILE);
    }
  }

  static void setFinalStatic(Field field, Object newValue) throws Exception {
    field.setAccessible(true);

    Field modifiersField = Field.class.getDeclaredField("modifiers");
    modifiersField.setAccessible(true);
    modifiersField.setInt(field, field.getModifiers() & ~Modifier.FINAL);

    field.set(null, newValue);
  }
}
