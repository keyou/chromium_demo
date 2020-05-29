#include "demo/demo_android/demo_apk/cpp/demo_foo.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/bind.h"
#include "base/lazy_instance.h"

#include "demo/demo_android/demo_apk/demo_apk_jni_headers/DemoFoo_jni.h"

namespace demo_jni {

base::android::ScopedJavaGlobalRef<jobject> j_demo_foo;

void DemoFoo::HelloDemoFoo(JNIEnv* env, const base::android::JavaParamRef<jstring>& who) {
    LOG(INFO)<<"[demo_foo] DemoFoo::HelloDemoFoo: Hello," << base::android::ConvertJavaStringToUTF8(env, who);
    }

static jlong JNI_DemoFoo_Init(JNIEnv* env, const base::android::JavaParamRef<jobject>& caller) {
    DLOG(INFO) << "[demo_apk]================ JNI_DemoFoo_Init";
    j_demo_foo.Reset(caller);
    return reinterpret_cast<intptr_t>(new DemoFoo());
}

static void JNI_DemoFoo_Hello(JNIEnv* env, const base::android::JavaParamRef<jstring>& who) {
    LOG(INFO)<<"[demo_foo] JNI_DemoFoo_Hello: Hello,"<<base::android::ConvertJavaStringToUTF8(env, who);
    // 从C++调用Java
    Hi("Java");
    HiStatic("Java Static");
}

void Hi(std::string who) {
    LOG(INFO) << "[demo_foo] Hi: Hi,"<<who;
    JNIEnv* env = base::android::AttachCurrentThread();
    Java_DemoFoo_Hi(env,j_demo_foo,base::android::ConvertUTF8ToJavaString(env,who));
}

void HiStatic(std::string who) {
    LOG(INFO) << "[demo_foo] Hi Static: Hi,"<<who;
    JNIEnv* env = base::android::AttachCurrentThread();
    Java_DemoFoo_HiStatic(env,base::android::ConvertUTF8ToJavaString(env,who));
}
}