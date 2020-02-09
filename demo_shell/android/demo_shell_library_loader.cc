#include "base/android/jni_android.h"
#include "content/public/app/content_jni_onload.h"
#include "content/public/app/content_main.h"
#include "content/shell/app/shell_main_delegate.h"

#include "base/android/base_jni_onload.h"
#include "base/logging.h"

#include "demo/demo_shell/android/demo_view_jni_registration.h"
#include "demo/demo_shell/app/demo_shell_content_main_delegate.h"

JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::android::InitVM(vm);
  JNIEnv* env = base::android::AttachCurrentThread();
  if (!RegisterMainDexNatives(env) || !RegisterNonMainDexNatives(env)) {
    return -1;
  }
  if (!base::android::OnJNIOnLoadInit())
    return -1;
  content::SetContentMainDelegate(new content::DemoShellContentMainDelegate());
  DLOG(INFO) << "======JNI_OnLoad finished";
  return JNI_VERSION_1_4;
}
