// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "demo/demo_shell/browser/shell.h"

#include <jni.h>

#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"

#include "demo/demo_shell/android/demo_shell_jni_headers/Shell_jni.h"
#include "demo/demo_shell/android/demo_shell_manager.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::JavaParamRef;
using base::android::ScopedJavaLocalRef;

namespace content {

void DemoShell::PlatformInitialize(const gfx::Size& default_window_size) {
  DLOG(INFO) << "=========PlatformInitialize";
}

void DemoShell::PlatformExit() {
  DLOG(INFO) << "=========PlatformExit";

  DestroyShellManager();
}

void DemoShell::PlatformCleanUp() {
  DLOG(INFO) << "=========PlatformCleanUp";

  JNIEnv* env = AttachCurrentThread();
  if (java_object_.is_null())
    return;
  Java_Shell_onNativeDestroyed(env, java_object_);
}

void DemoShell::PlatformCreateWindow(int width, int height) {
  DLOG(INFO) << "=========PlatformCreateWindow";

  java_object_.Reset(CreateShellView(this));
}

void DemoShell::PlatformSetContents() {
  DLOG(INFO) << "=========PlatformSetContents";

  JNIEnv* env = AttachCurrentThread();
  Java_Shell_initFromNativeTabContents(env, java_object_,
                                       web_contents()->GetJavaWebContents());
}

void DemoShell::SizeTo(const gfx::Size& content_size) {
  DLOG(INFO) << "=========SizeTo";

  JNIEnv* env = AttachCurrentThread();
  Java_Shell_sizeTo(env, java_object_, content_size.width(),
                    content_size.height()); 
}


// static
void JNI_Shell_CloseShell(JNIEnv* env,
                          jlong shellPtr) {
  DLOG(INFO) << "=========JNI_Shell_CloseShell";

  // DemoShell* shell = reinterpret_cast<DemoShell*>(shellPtr);

}

}  // namespace content
