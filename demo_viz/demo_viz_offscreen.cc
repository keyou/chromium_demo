#include "base/at_exit.h"
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
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/test/task_environment.h"
#include "base/test/test_discardable_memory_allocator.h"
#include "base/test/test_timeouts.h"
#include "base/threading/thread.h"
#include "build/build_config.h"
#include "build/buildflag.h"
#include "components/viz/common/quads/solid_color_draw_quad.h"
#include "components/viz/demo/host/demo_host.h"
#include "components/viz/demo/service/demo_service.h"
#include "components/viz/host/host_frame_sink_manager.h"
#include "components/viz/host/renderer_settings_creation.h"
#include "components/viz/service/display/display.h"
#include "components/viz/service/display/software_output_device.h"
#include "components/viz/service/display_embedder/output_surface_provider.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"
#include "components/viz/service/display_embedder/software_output_surface.h"
#include "components/viz/service/frame_sinks/frame_sink_manager_impl.h"
#include "components/viz/service/main/viz_compositor_thread_runner_impl.h"
#include "mojo/core/embedder/embedder.h"
#include "mojo/core/embedder/scoped_ipc_support.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "services/viz/privileged/mojom/viz_main.mojom.h"
#include "third_party/skia/include/core/SkImageEncoder.h"
#include "third_party/skia/include/core/SkStream.h"
#include "ui/base/hit_test.h"
#include "ui/base/ime/init/input_method_initializer.h"
#include "ui/base/material_design/material_design_controller.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/x/x11_util.h"
#include "ui/compositor/paint_recorder.h"
#include "ui/compositor/test/in_process_context_factory.h"
#include "ui/display/screen.h"
#include "ui/events/event.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font_util.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/skia_util.h"
#include "ui/gl/gl_switches.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/platform_window/platform_window_init_properties.h"
#include "ui/platform_window/x11/x11_window.h"

#include "components/viz/common/quads/picture_draw_quad.h"
#include "components/viz/common/quads/tile_draw_quad.h"
#include "components/viz/common/resources/resource_format.h"
#include "components/viz/client/client_resource_provider.h"
#include "components/viz/common/resources/bitmap_allocation.h"

#if defined(USE_AURA)
#include "ui/aura/env.h"
#include "ui/wm/core/wm_state.h"
#endif

#if defined(USE_X11)
#include "ui/gfx/x/x11_connection.h"            // nogncheck
#include "ui/platform_window/x11/x11_window.h"  // nogncheck
#endif

#if defined(USE_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#endif

#if defined(OS_WIN)
#include "ui/base/cursor/cursor_loader_win.h"
#include "ui/platform_window/win/win_window.h"
#endif

namespace demo {

// 实现离屏画面的保存
class OffscreenSoftwareOutputDevice : public viz::SoftwareOutputDevice {
 public:
  SkCanvas* BeginPaint(const gfx::Rect& damage_rect) override {
    DLOG(INFO) << "BeginPaint: get a canvas for paint";
    return viz::SoftwareOutputDevice::BeginPaint(damage_rect);
  }
  virtual void OnSwapBuffers(SwapBuffersCallback swap_ack_callback) override {
    auto image = surface_->makeImageSnapshot();
    SkBitmap bitmap;
    DCHECK(image->asLegacyBitmap(&bitmap));
    // 保存渲染结果到图片文件
    constexpr char filename[] = "demo_viz.png";
    base::FilePath path;
    DCHECK(base::PathService::Get(base::BasePathKey::DIR_EXE, &path));
    path = path.AppendASCII(filename);

    SkFILEWStream stream(path.value().c_str());
    DCHECK(
        SkEncodeImage(&stream, bitmap.pixmap(), SkEncodedImageFormat::kPNG, 0));
    DLOG(INFO) << "OnSwapBuffers: save the frame to: " << path;
    viz::SoftwareOutputDevice::OnSwapBuffers(std::move(swap_ack_callback));
  }
};

// 离屏画面的生成，类似Renderer进程做的事情
class OffscreenRenderer : public viz::mojom::CompositorFrameSinkClient,
                          public viz::DisplayClient {
 public:
  OffscreenRenderer() : thread_("OffscreenRenderer") {
    CHECK(thread_.Start());
    thread_.task_runner()->PostTask(
        FROM_HERE, base::BindOnce(&OffscreenRenderer::InitializeOnThread,
                                  base::Unretained(this)));
  }

 private:
  void InitializeOnThread() {
    shared_bitmap_manager_ = std::make_unique<viz::ServerSharedBitmapManager>();
    client_resource_provider_ = std::make_unique<viz::ClientResourceProvider>(false);
    frame_sink_manager_ = std::make_unique<viz::FrameSinkManagerImpl>(
        shared_bitmap_manager_.get());
    auto task_runner = base::ThreadTaskRunnerHandle::Get();

    // 生成 root client 的 LocalSurfaceId
    root_local_surface_id_allocator_.GenerateId();
    root_local_surface_id_ =
        root_local_surface_id_allocator_.GetCurrentLocalSurfaceIdAllocation();

    // 创建 support 用于将提交的CF存入Surface
    constexpr bool is_root = true;
    constexpr bool needs_sync_points = true;
    support_ = std::make_unique<viz::CompositorFrameSinkSupport>(
        this, frame_sink_manager_.get(), root_frame_sink_id_, is_root,
        needs_sync_points);

    // 表示一个timer，会根据设置定时触发回调
    auto time_source =
        std::make_unique<viz::DelayBasedTimeSource>(task_runner.get());

    time_source->SetTimebaseAndInterval(
        base::TimeTicks(), base::TimeDelta::FromMicroseconds(
                               base::Time::kMicrosecondsPerSecond / fps_));
    // 用于定时请求BeginFrame
    begin_frame_source_ = std::make_unique<viz::DelayBasedBeginFrameSource>(
        std::move(time_source), viz::BeginFrameSource::kNotRestartableId);

    auto output_surface = std::make_unique<viz::SoftwareOutputSurface>(
        std::make_unique<OffscreenSoftwareOutputDevice>());
    auto scheduler = std::make_unique<viz::DisplayScheduler>(
        begin_frame_source_.get(), task_runner.get(),
        output_surface->capabilities().max_frames_pending);
    viz::RendererSettings settings = viz::CreateRendererSettings();
    settings.use_skia_renderer = false;
    display_ = std::make_unique<viz::Display>(
        shared_bitmap_manager_.get(), settings, root_frame_sink_id_,
        std::move(output_surface), std::move(scheduler), task_runner);
    display_->Initialize(this, frame_sink_manager_->surface_manager());

    frame_sink_manager_->RegisterFrameSinkId(root_frame_sink_id_, false);
    frame_sink_manager_->RegisterBeginFrameSource(begin_frame_source_.get(),
                                                  root_frame_sink_id_);
    display_->Resize(size_);
    display_->SetVisible(true);
    support_->SetNeedsBeginFrame(true);
  }

  viz::CompositorFrame CreateFrame(const ::viz::BeginFrameArgs& args) {
    constexpr SkColor colors[] = {SK_ColorRED, SK_ColorGREEN, SK_ColorYELLOW};
    viz::CompositorFrame frame;

    frame.metadata.begin_frame_ack = viz::BeginFrameAck(args, true);
    frame.metadata.device_scale_factor = 1.f;
    frame.metadata.local_surface_id_allocation_time =
        root_local_surface_id_.allocation_time();
    frame.metadata.frame_token = ++frame_token_generator_;

    const int kRenderPassId = 1;
    const gfx::Rect& output_rect = gfx::Rect(size_);
    const gfx::Rect& damage_rect = output_rect;
    std::unique_ptr<viz::RenderPass> render_pass = viz::RenderPass::Create();
    render_pass->SetNew(kRenderPassId, output_rect, damage_rect,
                        gfx::Transform());

    // // Add a solid-color draw-quad for the big rectangle covering the entire
    // // content-area of the client.
    // viz::SharedQuadState* quad_state =
    //     render_pass->CreateAndAppendSharedQuadState();
    // quad_state->SetAll(
    //     gfx::Transform(),
    //     /*quad_layer_rect=*/output_rect,
    //     /*visible_quad_layer_rect=*/output_rect,
    //     /*rounded_corner_bounds=*/gfx::RRectF(),
    //     /*clip_rect=*/gfx::Rect(),
    //     /*is_clipped=*/false, /*are_contents_opaque=*/false, /*opacity=*/1.f,
    //     /*blend_mode=*/SkBlendMode::kSrcOver, /*sorting_context_id=*/0);

    // viz::SolidColorDrawQuad* color_quad =
    //     render_pass->CreateAndAppendDrawQuad<viz::SolidColorDrawQuad>();
    // color_quad->SetNew(quad_state, output_rect, output_rect,
    //                    colors[(*frame_token_generator_) % base::size(colors)],
    //                    false);

    {
      //gfx::Rect output_rect {0,0,32,32};
      // gfx::Rect output_rect = bounds_;
      // output_rect.Inset(10, 10, 10, 10);
      SkBitmap bitmap;
      bitmap.allocN32Pixels(32, 32);
      SkCanvas canvas(bitmap);
      canvas.clear(SK_ColorGRAY+*frame_token_generator_);

      gfx::Size tile_size(32, 32);
      viz::ResourceId resource =
          AllocateAndFillSoftwareResource(tile_size, bitmap);

      auto* quad_state = render_pass->CreateAndAppendSharedQuadState();
      quad_state->SetAll(
        gfx::Transform(),
        /*quad_layer_rect=*/output_rect,
        /*visible_quad_layer_rect=*/output_rect,
        /*rounded_corner_bounds=*/gfx::RRectF(),
        /*clip_rect=*/output_rect,
        /*is_clipped=*/false, /*are_contents_opaque=*/false, /*opacity=*/1.f,
        /*blend_mode=*/SkBlendMode::kSrcOver, /*sorting_context_id=*/0);
      auto* tile_quad =
          render_pass->CreateAndAppendDrawQuad<viz::TileDrawQuad>();
      tile_quad->SetNew(quad_state, output_rect, output_rect, false, resource,
                        gfx::RectF(output_rect), output_rect.size(), true, true,
                        true);
      std::vector<viz::ResourceId> resources = {resource};
      client_resource_provider_->PrepareSendToParent(
          resources, &frame.resource_list, (viz::RasterContextProvider*)nullptr);
      // frame.resource_list.push_back()
    }

    frame.render_pass_list.push_back(std::move(render_pass));

    return frame;
  }


  viz::ResourceId AllocateAndFillSoftwareResource(
    const gfx::Size& size,
    const SkBitmap& source) {
    viz::SharedBitmapId shared_bitmap_id = viz::SharedBitmap::GenerateId();
    base::MappedReadOnlyRegion shm =
      viz::bitmap_allocation::AllocateSharedBitmap(size, viz::RGBA_8888);
    shared_bitmap_manager_->ChildAllocatedSharedBitmap(shm.region.Map(),
                                                           shared_bitmap_id);
    base::WritableSharedMemoryMapping mapping = std::move(shm.mapping);

    SkImageInfo info = SkImageInfo::MakeN32Premul(size.width(), size.height());
    source.readPixels(info, mapping.memory(), info.minRowBytes(), 0, 0);

    return client_resource_provider_->ImportResource(
        viz::TransferableResource::MakeSoftware(shared_bitmap_id, size,
                                                viz::RGBA_8888),
        viz::SingleReleaseCallback::Create(base::DoNothing()));
  }

  virtual void DidReceiveCompositorFrameAck(
      const std::vector<::viz::ReturnedResource>& resources) override {}

  virtual void OnBeginFrame(
      const ::viz::BeginFrameArgs& args,
      const base::flat_map<uint32_t, ::viz::FrameTimingDetails>& details)
      override {
    DLOG(INFO) << "OnBeginFrame: submit a new frame";
    if (support_->last_activated_local_surface_id() !=
        root_local_surface_id_.local_surface_id()) {
      display_->SetLocalSurfaceId(root_local_surface_id_.local_surface_id(),
                                  1.0f);
    }
    support_->SubmitCompositorFrame(root_local_surface_id_.local_surface_id(),
                                    CreateFrame(args),
                                    base::Optional<viz::HitTestRegionList>(),
                                    /*trace_time=*/0);
  }

  virtual void OnBeginFramePausedChanged(bool paused) override {}

  virtual void ReclaimResources(
      const std::vector<::viz::ReturnedResource>& resources) override {}

  // viz::DisplayClient overrides.
  void DisplayOutputSurfaceLost() override {}
  void DisplayWillDrawAndSwap(bool will_draw_and_swap,
                              viz::RenderPassList* render_passes) override {}
  void DisplayDidDrawAndSwap() override {}
  void DisplayDidReceiveCALayerParams(
      const gfx::CALayerParams& ca_layer_params) override {}
  void DisplayDidCompleteSwapWithSize(const gfx::Size& pixel_size) override {}
  void SetPreferredFrameInterval(base::TimeDelta interval) override {}
  base::TimeDelta GetPreferredFrameIntervalForFrameSinkId(
      const viz::FrameSinkId& id) override {
    return frame_sink_manager_->GetPreferredFrameIntervalForFrameSinkId(id);
  }

  base::Thread thread_;
  std::unique_ptr<viz::ServerSharedBitmapManager> shared_bitmap_manager_;
  std::unique_ptr<viz::ClientResourceProvider> client_resource_provider_;
  std::unique_ptr<viz::FrameSinkManagerImpl> frame_sink_manager_;
  std::unique_ptr<viz::CompositorFrameSinkSupport> support_;
  std::unique_ptr<viz::DelayBasedBeginFrameSource> begin_frame_source_;
  std::unique_ptr<viz::Display> display_;
  // 由于要将显示存储为图片，所以使用1FPS
  double fps_ = 1.0;
  // 画面大小为 300x200
  gfx::Size size_{300, 200};

  viz::FrameSinkId root_frame_sink_id_{0, 1};
  viz::ParentLocalSurfaceIdAllocator root_local_surface_id_allocator_;
  viz::LocalSurfaceIdAllocation root_local_surface_id_;
  viz::FrameTokenGenerator frame_token_generator_;
};

}  // namespace demo

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true, true, true, false);
  // 启动 Trace
  auto trace_config = base::trace_event::TraceConfig("viz"/*, "trace-to-console"*/);
  base::trace_event::TraceLog::GetInstance()->SetEnabled(
      trace_config, base::trace_event::TraceLog::RECORDING_MODE);
  // 创建主消息循环，等价于 MessagLoop
  base::SingleThreadTaskExecutor main_task_executor(base::MessagePumpType::UI);
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("DemoViews");

  // 初始化mojo
  mojo::core::Init();
  base::Thread mojo_thread("mojo");
  mojo_thread.StartWithOptions(
      base::Thread::Options(base::MessagePumpType::IO, 0));
  auto ipc_support = std::make_unique<mojo::core::ScopedIPCSupport>(
      mojo_thread.task_runner(),
      mojo::core::ScopedIPCSupport::ShutdownPolicy::CLEAN);

  // 加载相应平台的GL库及GL绑定
  // gl::init::InitializeGLOneOff();

  // 初始化ICU(i18n),也就是icudtl.dat，views依赖ICU
  base::i18n::InitializeICU();

  ui::RegisterPathProvider();

  // This app isn't a test and shouldn't timeout.
  base::RunLoop::ScopedDisableRunTimeoutForTest disable_timeout;

  base::RunLoop run_loop;

  // 设置X11的异常处理回调，如果不设置在很多设备上会频繁出现崩溃。
  // 比如 ui::XWindow::Close() 和~SGIVideoSyncProviderThreadShim 的析构中
  // 都调用了 XDestroyWindow() ，并且是在不同的线程中调用的，当这两个窗口有
  // 父子关系的时候，如果先调用了父窗口的销毁再调用子窗口的销毁则会导致BadWindow
  // 错误，默认的Xlib异常处理会打印错误日志然后强制结束程序。
  // 这些错误大多是并发导致的代码执行顺序问题，所以修改起来没有那么容易。
  ui::SetDefaultX11ErrorHandlers();

  // 每秒生成一张图片保存到文件中
  // 可以使用这种原理将浏览器嵌入其他程序，当然这个demo演示的并不是最优方案，只是一种可行方案
  demo::OffscreenRenderer renderer;

  LOG(INFO) << "running...";
  run_loop.Run();

  return 0;
}
