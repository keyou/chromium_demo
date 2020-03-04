// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "demo/demo_shell/android/demo_shell_manager.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/bind.h"
#include "base/lazy_instance.h"
#include "content/public/browser/web_contents.h"

#include "demo/demo_shell/browser/shell.h"
#include "demo/demo_shell/browser/demo_shell_browser_context.h"
#include "demo/demo_shell/browser/demo_shell_content_browser_client.h"
#include "demo/demo_shell/android/demo_shell_jni_headers/ShellManager_jni.h"

#include "url/gurl.h"

using base::android::JavaParamRef;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;

namespace {

struct GlobalState {
  GlobalState() {}
  base::android::ScopedJavaGlobalRef<jobject> j_shell_manager;
};

base::LazyInstance<GlobalState>::DestructorAtExit g_global_state =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace content {

ScopedJavaLocalRef<jobject> CreateShellView(DemoShell* shell) {
  DLOG(INFO) << "============== CreateShellView";

  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_ShellManager_createShell(env,
                                       g_global_state.Get().j_shell_manager,
                                       reinterpret_cast<intptr_t>(shell));
}

void RemoveShellView(const JavaRef<jobject>& shell_view) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ShellManager_removeShell(env, g_global_state.Get().j_shell_manager,
                                shell_view);
}

static void JNI_ShellManager_Init(JNIEnv* env,
                                  const JavaParamRef<jobject>& obj) {
  g_global_state.Get().j_shell_manager.Reset(obj);
  DLOG(INFO) << "============== JNI_ShellManager_Init";
}

void JNI_ShellManager_LaunchShell(JNIEnv* env,
                                  const JavaParamRef<jstring>& jurl) {
  DemoShellBrowserContext* browserContext =
      DemoShellContentBrowserClient::Get()->browser_context();
  GURL url(base::android::ConvertJavaStringToUTF8(env, jurl));
  DemoShell::CreateNewWindow(browserContext,
                         url,
                         nullptr,
                         gfx::Size());
  DLOG(INFO) << "============== JNI_ShellManager_LaunchShell";
  
}

void DestroyShellManager() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ShellManager_destroy(env, g_global_state.Get().j_shell_manager);
  DLOG(INFO) << "============== DestroyShellManager";

}

}  // namespace content
