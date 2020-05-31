#ifndef DEMO_DEMO_ANDROID_SKIA_CANVAS_H
#define DEMO_DEMO_ANDROID_SKIA_CANVAS_H

#include <jni.h>

#include "android/native_window_jni.h"
#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace demo_jni {

class SkiaCanvas {
 public:
  virtual void OnTouch(JNIEnv* env, int action, jfloat x, jfloat y) = 0;

 protected:
  SkiaCanvas(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    const base::android::JavaParamRef<jobject>& surface);
  void ShowInfo(std::string info);

  const base::android::ScopedJavaGlobalRef<jobject> caller_;
  // SurfaceView 对应的 Window
  ANativeWindow* nativeWindow_;
  int width_;
  int height_;

  sk_sp<SkSurface> skSurface_ = nullptr;
  SkCanvas* skCanvas_ = nullptr;
  SkPaint pathPaint_;
  SkPaint circlePaint_;
  SkPath skPath_;
  SkScalar strokeWidth_ = 5.f;
 
};

}  // namespace demo_jni

#endif  // DEMO_DEMO_ANDROID_SKIA_CANVAS_H