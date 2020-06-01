#ifndef DEMO_DEMO_ANDROID_SKIA_CANVAS_SOFTWARE_H
#define DEMO_DEMO_ANDROID_SKIA_CANVAS_SOFTWARE_H

#include <jni.h>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

#include "android/native_window_jni.h"

#include "demo/demo_android/demo_android_skia/cpp/skia_canvas.h"

namespace demo_jni {

class SkiaCanvasSoftware : public SkiaCanvas {
 public:
  SkiaCanvasSoftware(JNIEnv* env,
                     const base::android::JavaParamRef<jobject>& caller,
                     const base::android::JavaParamRef<jobject>& surface);
 
 private:
  SkCanvas* BeginPaint() override;
  void SwapBuffer() override;

  ARect dirtyRect_;
};

} // namespace demo_jni

#endif // DEMO_DEMO_ANDROID_SKIA_CANVAS_SOFTWARE_H