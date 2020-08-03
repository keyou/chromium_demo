
#include "demo/demo_viz/demo_viz_layer_offscreen_client.h"

#include "base/at_exit.h"
#include "base/bind_helpers.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_type.h"
#include "base/path_service.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/rand_util.h"
#include "base/run_loop.h"
#include "base/synchronization/waitable_event.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/test/task_environment.h"
#include "base/test/test_discardable_memory_allocator.h"
#include "base/test/test_timeouts.h"
#include "base/threading/thread.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/trace_event/trace_buffer.h"
#include "build/build_config.h"
#include "build/buildflag.h"
#include "components/viz/service/display/skia_output_surface.h"
#include "components/viz/service/gl/gpu_service_impl.h"
#include "gpu/command_buffer/service/feature_info.h"
#include "gpu/command_buffer/service/gr_shader_cache.h"
#include "gpu/command_buffer/service/shared_context_state.h"
#include "gpu/config/gpu_driver_bug_workarounds.h"
#include "gpu/config/gpu_info_collector.h"
#include "gpu/config/gpu_preferences.h"
#include "gpu/config/gpu_util.h"
#include "gpu/ipc/common/gpu_client_ids.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/gl/GrGLTypes.h"
#include "third_party/skia/src/gpu/gl/GrGLDefines.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/init/gl_factory.h"

scoped_refptr<gl::GLSurface> g_gl_surface;
scoped_refptr<gl::GLContext> g_gl_context;
scoped_refptr<gpu::SharedContextState> g_context_state;
scoped_refptr<base::SingleThreadTaskRunner> g_gpu_task_runner;
viz::GpuServiceImpl* g_gpu_service;
gfx::Size g_size_;

void InitHostMain(gfx::AcceleratedWidget widget,
                  gfx::Size size,
                  viz::GpuServiceImpl* gpu_service) {
  TRACE_EVENT0("viz", "InitHostMain");
  g_size_ = size;
  g_gpu_service = gpu_service;
  g_gpu_task_runner = base::ThreadTaskRunnerHandle::Get();
  g_gl_surface = gl::init::CreateViewGLSurface(widget);

  auto context_state = gpu_service->GetContextState();
  auto share_group = context_state->share_group();
  g_gl_context = gl::init::CreateGLContext(share_group, g_gl_surface.get(),
                                           gl::GLContextAttribs());
  DCHECK(g_gl_context->MakeCurrent(g_gl_surface.get()));
  g_context_state = base::MakeRefCounted<gpu::SharedContextState>(
      share_group, g_gl_surface, g_gl_context, false, base::DoNothing(),
      gpu::GrContextType::kGL);
  gpu::GpuPreferences gpu_preferences;
  auto feature_info = gpu_service->GetContextState()->feature_info();
  g_context_state->InitializeGL(gpu_preferences, feature_info);
  g_context_state->InitializeGrContext(feature_info->workarounds(), nullptr);
  DCHECK(g_context_state->MakeCurrent(g_gl_surface.get(), true));
  DCHECK(g_context_state->gr_context());
  static unsigned int i = 0;
  glClearColor(1.0, (i++) % 10 / 10.f + 0.1f, 0, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  g_gl_surface->SwapBuffers(base::DoNothing());
}

void Redraw() {
  if (!g_gpu_task_runner->BelongsToCurrentThread()) {
    g_gpu_task_runner->PostTask(FROM_HERE, base::BindOnce(&Redraw));
    return;
  }

  TRACE_EVENT0("viz", "Redraw");
#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

  DCHECK(g_context_state->MakeCurrent(g_gl_surface.get(), true));

  // static unsigned int i = 0;
  // glClearColor(.0, (i++) % 10 / 10.f + 0.1f, 0, 1.f);
  // glClear(GL_COLOR_BUFFER_BIT);

  GrGLint buffer = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
  GrGLFramebufferInfo fb_info;
  fb_info.fFBOID = buffer;
  fb_info.fFormat = GL_RGBA8;

  auto gr_context = g_context_state->gr_context();
  DCHECK(gr_context);
  GrBackendRenderTarget backendRT(g_size_.width(), g_size_.height(), 0, 8,
                                  fb_info);
  SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
  auto skSurface = SkSurface::MakeFromBackendRenderTarget(
      gr_context, backendRT, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
      nullptr, &props);
  auto canvas = skSurface->getCanvas();
  static unsigned int i = 0;
  canvas->clear(SK_ColorYELLOW + i++);

#ifdef VIZ_GETOFFSCREENTEXTUREID
  auto texture_id = viz::GetOffscreenTextureId();
  if (texture_id) {
    GrGLTextureInfo textureInfo = {GR_GL_TEXTURE_2D, texture_id, GR_GL_RGBA8};
    GrBackendTexture backendTexture(g_size_.width(), g_size_.height(),
                                    GrMipMapped::kNo, textureInfo);
    sk_sp<SkImage> image = SkImage::MakeFromTexture(
        canvas->getGrContext(), backendTexture, kBottomLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    canvas->drawImage(image, 0, 0);
  }
#else
#pragma message "PATCH NOT APPLIED: demo/patches/0001-*.patch"
#endif // VIZ_GETOFFSCREENTEXTUREID

  skSurface->flush();
  g_gl_surface->SwapBuffers(base::DoNothing());
}