// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.demo_shell_webview;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.Toast;

import org.chromium.base.MemoryPressureListener;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;
import org.chromium.content_public.browser.BrowserStartupController;
import org.chromium.content_public.browser.DeviceUtils;
import org.chromium.content_public.browser.WebContents;
import org.chromium.demo_content_shell.Shell;
import org.chromium.demo_content_shell.ShellManager;
import org.chromium.ui.base.ActivityWindowAndroid;

/**
 * Activity for managing the Content Shell.
 */
public class DemoShellWebView extends Activity {

    private static final String TAG = "DemoShellWebView";

    private static final String ACTIVE_SHELL_URL_KEY = "activeUrl";
    public static final String COMMAND_LINE_ARGS_KEY = "commandLineArgs";

    private ShellManager mShellManager;
    private ActivityWindowAndroid mWindowAndroid;
    private Intent mLastSentIntent;
    private String mStartupUrl;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LibraryLoader.getInstance().ensureInitialized(LibraryProcessType.PROCESS_BROWSER);
        
        // System.loadLibrary("demo_shell_content_view");

        // setContentView(R.layout.demo_shell_webview);
        // mShellManager = findViewById(R.id.shell_container);
        
    }

    public void Initialize(ShellManager shellManager,final Bundle savedInstanceState){
        mShellManager = shellManager;
        
        final boolean listenToActivityState = true;
        mWindowAndroid = new ActivityWindowAndroid(this, listenToActivityState);
        mWindowAndroid.restoreInstanceState(savedInstanceState);
        mShellManager.setWindow(mWindowAndroid);
        // Set up the animation placeholder to be the SurfaceView. This disables the
        // SurfaceView's 'hole' clipping during animations that are notified to the window.
        mWindowAndroid.setAnimationPlaceholderView(
                mShellManager.getContentViewRenderView().getSurfaceView());

        BrowserStartupController.get(LibraryProcessType.PROCESS_BROWSER)
                .startBrowserProcessesAsync(
                        true, false, new BrowserStartupController.StartupCallback() {
                            @Override
                            public void onSuccess() {
                                finishInitialization(savedInstanceState);
                            }

                            @Override
                            public void onFailure() {
                                initializationFailed();
                            }
                        });
    }

    private void finishInitialization(Bundle savedInstanceState) {
        Log.e(TAG, "====================ContentView initialization finishInitialization.");
        String shellUrl;
        if (!TextUtils.isEmpty(mStartupUrl)) {
            shellUrl = mStartupUrl;
        } else {
            shellUrl = ShellManager.DEFAULT_SHELL_URL;
        }

        Log.e(TAG,"========"+shellUrl);

        if (savedInstanceState != null
                && savedInstanceState.containsKey(ACTIVE_SHELL_URL_KEY)) {
            shellUrl = savedInstanceState.getString(ACTIVE_SHELL_URL_KEY);
        }
        mShellManager.launchShell(shellUrl);
    }

    private void initializationFailed() {
        Log.e(TAG, "====================ContentView initialization failed.");
        finish();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        WebContents webContents = getActiveWebContents();
        if (webContents != null) {
            outState.putString(ACTIVE_SHELL_URL_KEY, webContents.getLastCommittedUrl());
        }

        mWindowAndroid.saveInstanceState(outState);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            WebContents webContents = getActiveWebContents();
            if (webContents != null && webContents.getNavigationController().canGoBack()) {
                webContents.getNavigationController().goBack();
                return true;
            }
        }

        return super.onKeyUp(keyCode, event);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        Log.e(TAG, "Ignoring command line params: can only be set when creating the activity.");
    }

    @Override
    protected void onStart() {
        super.onStart();

        WebContents webContents = getActiveWebContents();
        if (webContents != null) webContents.onShow();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        mWindowAndroid.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void startActivity(Intent i) {
        mLastSentIntent = i;
        super.startActivity(i);
    }

    @Override
    protected void onDestroy() {
        if (mShellManager != null) mShellManager.destroy();
        super.onDestroy();
    }

    public Intent getLastSentIntent() {
        return mLastSentIntent;
    }

    private static String getUrlFromIntent(Intent intent) {
        return intent != null ? intent.getDataString() : null;
    }

    /**
     * @return The {@link ShellManager} configured for the activity or null if it has not been
     *         created yet.
     */
    public ShellManager getShellManager() {
        return mShellManager;
    }

    /**
     * @return The currently visible {@link Shell} or null if one is not showing.
     */
    public Shell getActiveShell() {
        return mShellManager != null ? mShellManager.getActiveShell() : null;
    }

    /**
     * @return The {@link WebContents} owned by the currently visible {@link Shell} or null if
     *         one is not showing.
     */
    public WebContents getActiveWebContents() {
        Shell shell = getActiveShell();
        return shell != null ? shell.getWebContents() : null;
    }

}
