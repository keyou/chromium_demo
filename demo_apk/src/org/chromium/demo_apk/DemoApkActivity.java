// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.demo_apk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.Toast;

import org.chromium.base.CommandLine;
import org.chromium.base.MemoryPressureListener;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;
import org.chromium.ui.base.ActivityWindowAndroid;

/**
 * Activity for managing the Demo shell.
 */
public class DemoApkActivity extends Activity {

    private static final String TAG = "DemoApkActivity";

    private static final String ACTIVE_SHELL_URL_KEY = "activeUrl";
    public static final String COMMAND_LINE_ARGS_KEY = "commandLineArgs";

    // Native switch - shell_switches::kRunWebTests
    private static final String RUN_WEB_TESTS_SWITCH = "run-web-tests";

    //private ActivityWindowAndroid mWindowAndroid;

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

        // LibraryLoader.getInstance().ensureInitialized(LibraryProcessType.PROCESS_BROWSER);

        setContentView(R.layout.demo_apk_activity);
    }
    
    private static String[] getCommandLineParamsFromIntent(Intent intent) {
        return intent != null ? intent.getStringArrayExtra(COMMAND_LINE_ARGS_KEY) : null;
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        // WebContents webContents = getActiveWebContents();
        // if (webContents != null) {
        //     outState.putString(ACTIVE_SHELL_URL_KEY, webContents.getLastCommittedUrl());
        // }

        // mWindowAndroid.saveInstanceState(outState);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            // WebContents webContents = getActiveWebContents();
            // if (webContents != null && webContents.getNavigationController().canGoBack()) {
            //     webContents.getNavigationController().goBack();
            //     return true;
            // }
        }

        return super.onKeyUp(keyCode, event);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        // if (getCommandLineParamsFromIntent(intent) != null) {
        //     Log.i(TAG, "Ignoring command line params: can only be set when creating the activity.");
        // }

        // if (MemoryPressureListener.handleDebugIntent(this, intent.getAction())) return;

        // String url = getUrlFromIntent(intent);
        // if (!TextUtils.isEmpty(url)) {
        //     Shell activeView = getActiveShell();
        //     if (activeView != null) {
        //         activeView.loadUrl(url);
        //     }
        // }
    }

    @Override
    protected void onStart() {
        super.onStart();

        // WebContents webContents = getActiveWebContents();
        // if (webContents != null) webContents.onShow();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        //mWindowAndroid.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void startActivity(Intent i) {
        super.startActivity(i);
    }

    @Override
    protected void onDestroy() {
        //if (mShellManager != null) mShellManager.destroy();
        super.onDestroy();
    }

}
