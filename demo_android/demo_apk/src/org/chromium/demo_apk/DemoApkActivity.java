// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.demo_apk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import org.chromium.base.CommandLine;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;

import org.chromium.demo_jni.DemoFoo;

/**
 * Activity for managing the Demo shell.
 */
public class DemoApkActivity extends Activity {

    private static final String TAG = "DemoApk.DemoApkActivity";

    public static final String COMMAND_LINE_ARGS_KEY = "commandLineArgs";

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Initializing the command line must occur before loading the library.
        if (!CommandLine.isInitialized()) {
            ((DemoApkApplication) getApplication()).initCommandLine();
            String[] commandLineParams = getCommandLineParamsFromIntent(getIntent());
            if (commandLineParams != null) {
                CommandLine.getInstance().appendSwitchesAndArguments(commandLineParams);
            }
        }

        setContentView(R.layout.demo_apk_activity);
    }
    
    private static String[] getCommandLineParamsFromIntent(Intent intent) {
        return intent != null ? intent.getStringArrayExtra(COMMAND_LINE_ARGS_KEY) : null;
    }

    public void onClick(View view) {
        Log.i(TAG,"Call onClick");
        
        // 加载JNI库
        LibraryLoader.getInstance().ensureInitialized(LibraryProcessType.PROCESS_BROWSER);
        
        DemoFoo foo = new DemoFoo();
        foo.Hello("JNI");
    }
}
