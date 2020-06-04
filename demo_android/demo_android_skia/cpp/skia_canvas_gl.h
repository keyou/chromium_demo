#ifndef DEMO_DEMO_ANDROID_SKIA_CANVAS_GL_H
#define DEMO_DEMO_ANDROID_SKIA_CANVAS_GL_H

#include <jni.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <GLES/gl.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2.h>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

#include "android/native_window_jni.h"

#include "demo/demo_android/demo_android_skia/cpp/skia_canvas.h"

#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"
#include "third_party/skia/include/core/SkDeferredDisplayListRecorder.h"

namespace demo_jni {

class SkiaCanvasGL : public SkiaCanvas {
 public:
  SkiaCanvasGL(JNIEnv* env,
               const base::android::JavaParamRef<jobject>& caller,
               const base::android::JavaParamRef<jobject>& surface);

 private:
  void InitializeOnRenderThread() override;
  SkCanvas* BeginPaint() override;
  void OnPaint(SkCanvas* canvas) override;
  void SwapBuffer() override;

  EGLDisplay display_;
  EGLContext context_;
  EGLSurface surface_;
  EGLint stencilBits_;
  EGLint sampleCount_;
  sk_sp<const GrGLInterface> grGLInterface_;
  sk_sp<GrContext> grContext_;
  bool use_ddl_ = false;
  std::unique_ptr<SkDeferredDisplayListRecorder> recorder_;
};

} // namespace demo_jni

#endif // DEMO_DEMO_ANDROID_SKIA_CANVAS_GL_H