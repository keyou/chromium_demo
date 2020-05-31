#include "base/android/jni_android.h"
// #include "content/public/app/content_jni_onload.h"
// #include "content/public/app/content_main.h"

#include "base/android/base_jni_onload.h"
#include "base/logging.h"

//#include "demo/demo_shell/app/demo_shell_content_main_delegate.h"

JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  DLOG(INFO) << "[demo_android_skia] JNI_OnLoad start";
  // 这个初始化是必要的，因为LibraryLoader依赖它，大量的base::android::*都依赖它
  // 它其实就是将vm保存为了内部的全局变量
  base::android::InitVM(vm);
  // 下面这个初始化在这个程序中不是必须的，但是推荐加上，主要是初始化了ClassLoader
  if (!base::android::OnJNIOnLoadInit())
    return false;
  
  // 我们没有使用（crazy linker）动态注册JNI函数的方式，所以这里不需要以下代码
  // if (!RegisterMainDexNatives(env) || !RegisterNonMainDexNatives(env)) {
  //   return -1;
  // }

  DLOG(INFO) << "[demo_android_skia] JNI_OnLoad finished";
  return JNI_VERSION_1_4;
}
