
#include "demo/demo_skia/skia_canvas_gl.h"

#include "base/bind.h"
#include "base/lazy_instance.h"
#include "base/trace_event/trace_event.h"
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
#if defined(OS_ANDROID)
  options.fSuppressGeometryShaders = false;
  options.fGpuPathRenderers = GpuPathRenderers::kAll;
#endif  // OS_ANDROID
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

SkiaCanvasGL::SkiaCanvasGL(gfx::AcceleratedWidget widget, int width, int height)
    : SkiaCanvas(widget, width, height) {
  background_ = 0xFF00FF00;
  tag_ = "SkiaCanvasGL";
}

void SkiaCanvasGL::InitializeOnRenderThread() {
  TRACE_EVENT0("shell", "SkiaCanvasGL::InitializeOnRenderThread");
  display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLint majorVersion;
  EGLint minorVersion;
  eglInitialize(display_, &majorVersion, &minorVersion);

  DCHECK(eglBindAPI(EGL_OPENGL_ES_API));

  SkDebugf("EGL: %d.%d", majorVersion, minorVersion);
  SkDebugf("Vendor: %s", eglQueryString(display_, EGL_VENDOR));
  SkDebugf("Extensions: %s", eglQueryString(display_, EGL_EXTENSIONS));

  ///////////////////////////////////////////////////////////
  EGLint config_count = 0;
  // Get number of all configs, have gotten display from EGL
  DCHECK(eglGetConfigs(display_, NULL, 0, &config_count));
  DLOG(INFO) << "Configurations available count: " << config_count;

  // collect information about the configs
  EGLConfig* configs = new EGLConfig[config_count];

  if (EGL_FALSE == eglGetConfigs(display_, configs, config_count, &config_count)) {
    delete[] configs;
    return;
  }
  struct {
    EGLint _alpha_size;
    EGLint _bind_to_texture_rgb;
    EGLint _bind_to_texture_rgba;
    EGLint _blue_size;
    EGLint _buffer_size;
    EGLint _config_caveat;
    EGLint _config_id;
    EGLint _depth_size;
    EGLint _green_size;
    EGLint _red_size;
    EGLint _level;
    EGLint _max_pbuffer_width;
    EGLint _max_pbuffer_height;
    EGLint _max_pbuffer_pixels;
    EGLint _max_swap_interval;
    EGLint _min_swap_interval;
    EGLint _native_renderable;
    EGLint _native_visual_id;
    EGLint _alpha_mask_size;
    EGLint _color_buffer_type;
    EGLint _luminance_size;
    EGLint _renderable_type;
    EGLint _conformant;
  } newFormat = {0};
  for (GLint c = 0; c < config_count; ++c) {
    EGLConfig config = configs[c];
    eglGetConfigAttrib(display_, config, EGL_ALPHA_SIZE,
                       &(newFormat._alpha_size));
    eglGetConfigAttrib(display_, config, EGL_BIND_TO_TEXTURE_RGB,
                       &(newFormat._bind_to_texture_rgb));
    eglGetConfigAttrib(display_, config, EGL_BIND_TO_TEXTURE_RGBA,
                       &(newFormat._bind_to_texture_rgba));
    eglGetConfigAttrib(display_, config, EGL_BLUE_SIZE,
                       &(newFormat._blue_size));
    eglGetConfigAttrib(display_, config, EGL_BUFFER_SIZE,
                       &(newFormat._buffer_size));
    eglGetConfigAttrib(display_, config, EGL_CONFIG_CAVEAT,
                       &(newFormat._config_caveat));
    eglGetConfigAttrib(display_, config, EGL_CONFIG_ID,
                       &(newFormat._config_id));
    eglGetConfigAttrib(display_, config, EGL_DEPTH_SIZE,
                       &(newFormat._depth_size));
    eglGetConfigAttrib(display_, config, EGL_GREEN_SIZE,
                       &(newFormat._green_size));
    eglGetConfigAttrib(display_, config, EGL_RED_SIZE, &(newFormat._red_size));
    eglGetConfigAttrib(display_, config, EGL_LEVEL, &(newFormat._level));
    eglGetConfigAttrib(display_, config, EGL_MAX_PBUFFER_WIDTH,
                       &(newFormat._max_pbuffer_width));
    eglGetConfigAttrib(display_, config, EGL_MAX_PBUFFER_HEIGHT,
                       &(newFormat._max_pbuffer_height));
    eglGetConfigAttrib(display_, config, EGL_MAX_PBUFFER_PIXELS,
                       &(newFormat._max_pbuffer_pixels));
    eglGetConfigAttrib(display_, config, EGL_MAX_SWAP_INTERVAL,
                       &(newFormat._max_swap_interval));
    eglGetConfigAttrib(display_, config, EGL_MIN_SWAP_INTERVAL,
                       &(newFormat._min_swap_interval));
    eglGetConfigAttrib(display_, config, EGL_NATIVE_RENDERABLE,
                       &(newFormat._native_renderable));
    eglGetConfigAttrib(display_, config, EGL_NATIVE_VISUAL_ID,
                       &(newFormat._native_visual_id));
    /// etc etc etc for all those that you care about

    if (majorVersion >= 1 && minorVersion >= 2) {
      // 1.2
      eglGetConfigAttrib(display_, config, EGL_ALPHA_MASK_SIZE,
                         &(newFormat._alpha_mask_size));
      eglGetConfigAttrib(display_, config, EGL_COLOR_BUFFER_TYPE,
                         &(newFormat._color_buffer_type));
      eglGetConfigAttrib(display_, config, EGL_LUMINANCE_SIZE,
                         &(newFormat._luminance_size));
      eglGetConfigAttrib(display_, config, EGL_RENDERABLE_TYPE,
                         &(newFormat._renderable_type));
    }

    if (majorVersion >= 1 && minorVersion >= 3) {
      // 1.3
      eglGetConfigAttrib(display_, config, EGL_CONFORMANT,
                         &(newFormat._conformant));
    }
    DLOG(INFO) << "newFormat._alpha_size: " << newFormat._alpha_size;
    DLOG(INFO) << "newFormat._bind_to_texture_rgb: "
               << newFormat._bind_to_texture_rgb;
    DLOG(INFO) << "newFormat._bind_to_texture_rgba: "
               << newFormat._bind_to_texture_rgba;
    DLOG(INFO) << "newFormat._blue_size: " << newFormat._blue_size;
    DLOG(INFO) << "newFormat._buffer_size: " << newFormat._buffer_size;
    DLOG(INFO) << "newFormat._config_caveat: " << newFormat._config_caveat;
    DLOG(INFO) << "newFormat._config_id: " << newFormat._config_id;
    DLOG(INFO) << "newFormat._depth_size: " << newFormat._depth_size;
    DLOG(INFO) << "newFormat._green_size: " << newFormat._green_size;
    DLOG(INFO) << "newFormat._red_size: " << newFormat._red_size;
    DLOG(INFO) << "newFormat._level: " << newFormat._level;
    DLOG(INFO) << "newFormat._max_pbuffer_width: "
               << newFormat._max_pbuffer_width;
    DLOG(INFO) << "newFormat._max_pbuffer_height: "
               << newFormat._max_pbuffer_height;
    DLOG(INFO) << "newFormat._max_pbuffer_pixels: "
               << newFormat._max_pbuffer_pixels;
    DLOG(INFO) << "newFormat._max_swap_interval: "
               << newFormat._max_swap_interval;
    DLOG(INFO) << "newFormat._min_swap_interval: "
               << newFormat._min_swap_interval;
    DLOG(INFO) << "newFormat._native_renderable: "
               << newFormat._native_renderable;
    DLOG(INFO) << "newFormat._native_visual_id: "
               << newFormat._native_visual_id;
    DLOG(INFO) << "newFormat._alpha_mask_size: " << newFormat._alpha_mask_size;
    DLOG(INFO) << "newFormat._color_buffer_type: "
               << newFormat._color_buffer_type;
    DLOG(INFO) << "newFormat._luminance_size: " << newFormat._luminance_size;
    DLOG(INFO) << "newFormat._renderable_type: " << newFormat._renderable_type;
    DLOG(INFO) << "newFormat._conformant: " << newFormat._conformant;
    DLOG(INFO) << "=========================";
  }
  /////////////////////////////////////////////////////////////

  EGLint numConfigs = 0;
  EGLint eglSampleCnt = 0;
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
  DCHECK(numConfigs > 0) << "error: " << eglGetError();

  static const EGLint kEGLContextAttribsForOpenGLES[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  context_ = eglCreateContext(display_, surfaceConfig, nullptr,
                              kEGLContextAttribsForOpenGLES);
  DCHECK(EGL_NO_CONTEXT != context_);

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

  // 关闭 VSYNC ，否则会由于帧率的抖动导致平均帧率降低。
  // VSYNC 会阻塞 SkSurface::flush，从而使每帧的耗时接近16.6ms,帧率最多60fps
  eglSwapInterval(display_, 0);
  eglSwapBuffers(display_, surface_);

  grGLInterface_ = GrGLMakeNativeInterface();
  DCHECK(grGLInterface_);

  grContext_ = GrContext::MakeGL(grGLInterface_, CreateGrContextOptions());
  DCHECK(grContext_);
  SkiaCanvas::InitializeOnRenderThread();
}

void SkiaCanvasGL::Resize(int width, int height) {
  width_ = width;
  height_ = height;
}

SkiaCanvasGL::~SkiaCanvasGL() {
  if (display_ != EGL_NO_DISPLAY) {
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context_ != EGL_NO_CONTEXT) {
      eglDestroyContext(display_, context_);
      DLOG(INFO) << "eglDestroyContext";
    }
    if (surface_ != EGL_NO_SURFACE) {
      eglDestroySurface(display_, surface_);
      DLOG(INFO) << "eglDestroySurface";
    }
    eglTerminate(display_);
    DLOG(INFO) << "eglTerminate";
  }
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
  } else
    skSurface_->flush();
}

void SkiaCanvasGL::SwapBuffer() {
  DCHECK(display_);
  DCHECK(surface_);

  eglSwapBuffers(display_, surface_);
}

}  // namespace demo_jni
