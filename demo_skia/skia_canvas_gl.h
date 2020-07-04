#ifndef DEMO_DEMO_SKIA_SKIA_CANVAS_GL_H
#define DEMO_DEMO_SKIA_SKIA_CANVAS_GL_H

#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
// Chromium 在 gpu/GLES2/gl2chromium_autogen.h 中重定义了所有的 gles2 的方法
// 到 GLES2xxx，这样来对接 command buffer，我们不需要使用 command buffer,因此
// 使用绝对路径绕过这个逻辑
//#include <GLES2/gl2.h>
#include </usr/include/GLES2/gl2.h>

#include "demo/demo_skia/skia_canvas.h"

#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"
#include "third_party/skia/include/core/SkDeferredDisplayListRecorder.h"

namespace demo_jni {

class SkiaCanvasGL : public SkiaCanvas {
 public:
  SkiaCanvasGL(gfx::AcceleratedWidget widget,int width,int height);
  void Resize(int width, int height) override;

  ~SkiaCanvasGL() override; 

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

#endif // DEMO_DEMO_SKIA_SKIA_CANVAS_GL_H
