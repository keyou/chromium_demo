#ifndef DEMO_DEMO_ANDROID_DEMO_FOO_H
#define DEMO_DEMO_ANDROID_DEMO_FOO_H

#include <jni.h>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

namespace demo_jni {

class DemoFoo {
public:
void HelloDemoFoo(JNIEnv* env, const base::android::JavaParamRef<jstring>& who);
};

    void Hi(std::string who);
    void HiStatic(std::string who);
}

#endif //DEMO_DEMO_ANDROID_DEMO_FOO_H