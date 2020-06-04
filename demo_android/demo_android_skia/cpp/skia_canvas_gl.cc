
#include "demo/demo_android/demo_android_skia/cpp/skia_canvas_gl.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/bind.h"
#include "base/lazy_instance.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkDeferredDisplayListRecorder.h"
#include "third_party/skia/include/core/SkExecutor.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkSurfaceCharacterization.h"
#include "third_party/skia/include/gpu/gl/GrGLAssembleInterface.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace demo_jni {

namespace {
class GLShaderErrorHandler : public GrContextOptions::ShaderErrorHandler {
 public:
  void compileError(const char* shader, const char* errors) {
    LOG(ERROR) << "Skia shader compilation error\n"
               << "------------------------\n"
               << shader << "\nErrors:\n"
               << errors;
  }
};

// 作为 skia 的 GL 后端，使用 EGL 来获取 GL functions.
// 由于 chromium 包装的 skia 库没有将 skia 对 EGL 的支持包装进去，
// 因此我们这里使用 skia 提供的接口来对接 EGL，
// 如果使用官方的 skia 库则不需要这个操作。
// 具体见 skia 仓库中的 include/gpu/gl/GrGLInterface.h 文件
// 这里的代码从 skia 仓库中的 GrGLMakeNativeInterface_egl.cpp 复制而来
static GrGLFuncPtr egl_get_gl_proc(void* ctx, const char name[]) {
  SkASSERT(nullptr == ctx);
// https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_get_all_proc_addresses.txt
// eglGetProcAddress() is not guaranteed to support the querying of
// non-extension EGL functions.
#define M(X)                   \
  if (0 == strcmp(#X, name)) { \
    return (GrGLFuncPtr)X;     \
  }
  M(eglGetCurrentDisplay)
  M(eglQueryString)
  M(glActiveTexture)
  M(glAttachShader)
  M(glBindAttribLocation)
  M(glBindBuffer)
  M(glBindFramebuffer)
  M(glBindRenderbuffer)
  M(glBindTexture)
  M(glBlendColor)
  M(glBlendEquation)
  M(glBlendFunc)
  M(glBufferData)
  M(glBufferSubData)
  M(glCheckFramebufferStatus)
  M(glClear)
  M(glClearColor)
  M(glClearStencil)
  M(glColorMask)
  M(glCompileShader)
  M(glCompressedTexImage2D)
  M(glCompressedTexSubImage2D)
  M(glCopyTexSubImage2D)
  M(glCreateProgram)
  M(glCreateShader)
  M(glCullFace)
  M(glDeleteBuffers)
  M(glDeleteFramebuffers)
  M(glDeleteProgram)
  M(glDeleteRenderbuffers)
  M(glDeleteShader)
  M(glDeleteTextures)
  M(glDepthMask)
  M(glDisable)
  M(glDisableVertexAttribArray)
  M(glDrawArrays)
  M(glDrawElements)
  M(glEnable)
  M(glEnableVertexAttribArray)
  M(glFinish)
  M(glFlush)
  M(glFramebufferRenderbuffer)
  M(glFramebufferTexture2D)
  M(glFrontFace)
  M(glGenBuffers)
  M(glGenFramebuffers)
  M(glGenRenderbuffers)
  M(glGenTextures)
  M(glGenerateMipmap)
  M(glGetBufferParameteriv)
  M(glGetError)
  M(glGetFramebufferAttachmentParameteriv)
  M(glGetIntegerv)
  M(glGetProgramInfoLog)
  M(glGetProgramiv)
  M(glGetRenderbufferParameteriv)
  M(glGetShaderInfoLog)
  M(glGetShaderPrecisionFormat)
  M(glGetShaderiv)
  M(glGetString)
  M(glGetUniformLocation)
  M(glIsTexture)
  M(glLineWidth)
  M(glLinkProgram)
  M(glPixelStorei)
  M(glReadPixels)
  M(glRenderbufferStorage)
  M(glScissor)
  M(glShaderSource)
  M(glStencilFunc)
  M(glStencilFuncSeparate)
  M(glStencilMask)
  M(glStencilMaskSeparate)
  M(glStencilOp)
  M(glStencilOpSeparate)
  M(glTexImage2D)
  M(glTexParameterf)
  M(glTexParameterfv)
  M(glTexParameteri)
  M(glTexParameteriv)
  M(glTexSubImage2D)
  M(glUniform1f)
  M(glUniform1fv)
  M(glUniform1i)
  M(glUniform1iv)
  M(glUniform2f)
  M(glUniform2fv)
  M(glUniform2i)
  M(glUniform2iv)
  M(glUniform3f)
  M(glUniform3fv)
  M(glUniform3i)
  M(glUniform3iv)
  M(glUniform4f)
  M(glUniform4fv)
  M(glUniform4i)
  M(glUniform4iv)
  M(glUniformMatrix2fv)
  M(glUniformMatrix3fv)
  M(glUniformMatrix4fv)
  M(glUseProgram)
  M(glVertexAttrib1f)
  M(glVertexAttrib2fv)
  M(glVertexAttrib3fv)
  M(glVertexAttrib4fv)
  M(glVertexAttribPointer)
  M(glViewport)
#undef M
  return eglGetProcAddress(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
  return GrGLMakeAssembledInterface(nullptr, egl_get_gl_proc);
}

GrContextOptions CreateGrContextOptions() {
  static std::unique_ptr<SkExecutor> gGpuExecutor =
      SkExecutor::MakeFIFOThreadPool(1);
  static std::unique_ptr<GLShaderErrorHandler> handler =
      std::make_unique<GLShaderErrorHandler>();

  GrContextOptions options;
  options.fExecutor = gGpuExecutor.get();
  options.fDisableCoverageCountingPaths = true;
  options.fAllowPathMaskCaching = true;
  options.fSuppressGeometryShaders = false;
  options.fGpuPathRenderers = GpuPathRenderers::kAll;
  options.fDisableDriverCorrectnessWorkarounds = false;
  options.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;

  options.fPersistentCache = nullptr;
  options.fShaderCacheStrategy =
      GrContextOptions::ShaderCacheStrategy::kBackendSource;
  options.fShaderErrorHandler = handler.get();
  options.fSuppressPrints = true;
  // options.fInternalMultisampleCount = 4;
  return options;
}

}  // namespace

SkiaCanvasGL::SkiaCanvasGL(JNIEnv* env,
                           const base::android::JavaParamRef<jobject>& caller,
                           const base::android::JavaParamRef<jobject>& surface)
    : SkiaCanvas(env, caller, surface) {
  background_ = 0xFF00FF00;
  tag_ = "SkiaCanvasGL";
}

void SkiaCanvasGL::InitializeOnRenderThread() {
  display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLint majorVersion;
  EGLint minorVersion;
  eglInitialize(display_, &majorVersion, &minorVersion);

  DCHECK(eglBindAPI(EGL_OPENGL_ES_API));

  EGLint numConfigs = 0;
  EGLint eglSampleCnt = 1;
  const EGLint configAttribs[] = {EGL_SURFACE_TYPE,
                                  EGL_PBUFFER_BIT,
                                  EGL_RENDERABLE_TYPE,
                                  EGL_OPENGL_ES2_BIT,
                                  EGL_RED_SIZE,
                                  8,
                                  EGL_GREEN_SIZE,
                                  8,
                                  EGL_BLUE_SIZE,
                                  8,
                                  EGL_ALPHA_SIZE,
                                  8,
                                  EGL_STENCIL_SIZE,
                                  8,
                                  EGL_SAMPLE_BUFFERS,
                                  eglSampleCnt ? 1 : 0,
                                  EGL_SAMPLES,
                                  eglSampleCnt,
                                  EGL_NONE};

  EGLConfig surfaceConfig;
  DCHECK(
      eglChooseConfig(display_, configAttribs, &surfaceConfig, 1, &numConfigs));
  DCHECK(numConfigs > 0);

  static const EGLint kEGLContextAttribsForOpenGLES[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  context_ = eglCreateContext(display_, surfaceConfig, nullptr,
                              kEGLContextAttribsForOpenGLES);
  DCHECK(EGL_NO_CONTEXT != context_);

  //    SkDebugf("EGL: %d.%d", majorVersion, minorVersion);
  //    SkDebugf("Vendor: %s", eglQueryString(display_, EGL_VENDOR));
  //    SkDebugf("Extensions: %s", eglQueryString(display_, EGL_EXTENSIONS));

  surface_ =
      eglCreateWindowSurface(display_, surfaceConfig, nativeWindow_, nullptr);
  DCHECK(EGL_NO_SURFACE != surface_);

  DCHECK(eglMakeCurrent(display_, surface_, surface_, context_));
  glClearStencil(0);
  // glClearColor(0, 0, 0, 0);
  // glStencilMask(0xffffffff);
  glClearColor(0x00, 0xFF, 0x00, 0xFF);  // 0xFF00DE96
  glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  eglGetConfigAttrib(display_, surfaceConfig, EGL_STENCIL_SIZE, &stencilBits_);
  eglGetConfigAttrib(display_, surfaceConfig, EGL_SAMPLES, &sampleCount_);
  sampleCount_ = SkTMax(sampleCount_, 1);

  // 开启 VSYNC，这会导致 SkSurface::flush 被GL调用阻塞，从而使每帧的耗时接近16.6ms,帧率最多60fps
  eglSwapInterval(display_, 0);
  eglSwapBuffers(display_, surface_);

  grGLInterface_ = GrGLMakeNativeInterface();
  DCHECK(grGLInterface_);

  grContext_ = GrContext::MakeGL(grGLInterface_, CreateGrContextOptions());
  DCHECK(grContext_);
  SkiaCanvas::InitializeOnRenderThread();
}

SkCanvas* SkiaCanvasGL::BeginPaint() {
  DCHECK(display_);
  DCHECK(surface_);
  GrGLint buffer = 0;
  grGLInterface_->fFunctions.fGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);

#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

  GrGLFramebufferInfo fb_info;
  fb_info.fFBOID = buffer;
  fb_info.fFormat = GL_RGBA8;

  GrBackendRenderTarget backendRT(width_, height_, sampleCount_, stencilBits_,
                                  fb_info);
  SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
  skSurface_ = SkSurface::MakeFromBackendRenderTarget(
      grContext_.get(), backendRT, kBottomLeft_GrSurfaceOrigin,
      kRGBA_8888_SkColorType, nullptr, &props);
  if (use_ddl_) {
    SkSurfaceCharacterization characterization;
    bool result = skSurface_->characterize(&characterization);
    DCHECK(result) << "Failed to characterize raster SkSurface.";
    // 使用 ddl 来记录绘制操作，并不执行真正的绘制
    recorder_ =
        std::make_unique<SkDeferredDisplayListRecorder>(characterization);

    return recorder_->getCanvas();
  }
  return skSurface_->getCanvas();
}

void SkiaCanvasGL::OnPaint(SkCanvas* canvas) {
  if (use_ddl_) {
    // 必须在list销毁前flush
    auto list = recorder_->detach();
    DCHECK(skSurface_->draw(list.get()));
    skSurface_->flush();
  }
  else skSurface_->flush();
}

void SkiaCanvasGL::SwapBuffer() {
  DCHECK(display_);
  DCHECK(surface_);

  eglSwapBuffers(display_, surface_);
}

}  // namespace demo_jni