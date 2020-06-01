#ifndef DEMO_DEMO_ANDROID_SKIA_CANVAS_GL_H
#define DEMO_DEMO_ANDROID_SKIA_CANVAS_GL_H

#include <jni.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <GLES/gl.h>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

#include "android/native_window_jni.h"

#include "demo/demo_android/demo_android_skia/cpp/skia_canvas.h"

#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace demo_jni {

class SkiaCanvasGL : public SkiaCanvas {
 public:
  SkiaCanvasGL(JNIEnv* env,
               const base::android::JavaParamRef<jobject>& caller,
               const base::android::JavaParamRef<jobject>& surface);

 private:
  SkSurface* BeginPaint() override;
  void SwapBuffer() override;

  EGLDisplay display_;
  EGLContext context_;
  EGLSurface surface_;
  EGLint stencilBits_;
  EGLint sampleCount_;
  sk_sp<const GrGLInterface> grGLInterface_;
  sk_sp<GrContext> grContext_;
};

} // namespace demo_jni

#endif // DEMO_DEMO_ANDROID_SKIA_CANVAS_GL_H