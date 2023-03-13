#include "base/at_exit.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
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
#include "components/viz/service/display/overlay_processor_stub.h"
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
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkEncodedImageFormat.h"
#include "third_party/skia/include/core/SkImageEncoder.h"
#include "third_party/skia/include/core/SkStream.h"
#include "ui/base/hit_test.h"
#include "ui/base/ime/init/input_method_initializer.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
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

#if defined(USE_AURA)
#include "ui/aura/env.h"
#include "ui/wm/core/wm_state.h"
#endif

#if BUILDFLAG(IS_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#endif

#if BUILDFLAG(IS_WIN)
// #include "ui/base/cursor/cursor_loader_win.h"
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
  void OnSwapBuffers(SwapBuffersCallback swap_ack_callback,
                     gl::FrameData data) override {
    auto image = surface_->makeImageSnapshot();
    SkBitmap bitmap;
    DCHECK(image->asLegacyBitmap(&bitmap));
    // 保存渲染结果到图片文件
    constexpr char filename[] = "demo_viz.png";
    base::FilePath path;
    DCHECK(base::PathService::Get(base::BasePathKey::DIR_EXE, &path));
    path = path.AppendASCII(filename);
#if defined(OS_WIN)
    SkFILEWStream stream(path.AsUTF8Unsafe().c_str());
#else
    SkFILEWStream stream(path.value().c_str());
#endif
    DCHECK(
        SkEncodeImage(&stream, bitmap.pixmap(), SkEncodedImageFormat::kPNG, 0));
    DLOG(INFO) << "OnSwapBuffers: save the frame to: " << path;
    viz::SoftwareOutputDevice::OnSwapBuffers(std::move(swap_ack_callback),
                                             data);
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
    auto init_params = viz::FrameSinkManagerImpl::InitParams(
        shared_bitmap_manager_.get());
    frame_sink_manager_ = std::make_unique<viz::FrameSinkManagerImpl>(init_params);
    auto task_runner = base::ThreadTaskRunnerHandle::Get();

    // 生成 root client 的 LocalSurfaceId
    root_local_surface_id_allocator_.GenerateId();
    root_local_surface_id_ =
        root_local_surface_id_allocator_.GetCurrentLocalSurfaceId();

    // 创建 support 用于将提交的CF存入Surface
    constexpr bool is_root = true;
    // constexpr bool needs_sync_points = true;
    support_ = std::make_unique<viz::CompositorFrameSinkSupport>(
        this, frame_sink_manager_.get(), root_frame_sink_id_, is_root);

    // 表示一个timer，会根据设置定时触发回调
    auto time_source =
        std::make_unique<viz::DelayBasedTimeSource>(task_runner.get());

    time_source->SetTimebaseAndInterval(
        base::TimeTicks(), base::Microseconds(
                               base::Time::kMicrosecondsPerSecond / fps_));
    // 用于定时请求BeginFrame
    begin_frame_source_ = std::make_unique<viz::DelayBasedBeginFrameSource>(
        std::move(time_source), viz::BeginFrameSource::kNotRestartableId);

    auto output_surface = std::make_unique<viz::SoftwareOutputSurface>(
        std::make_unique<OffscreenSoftwareOutputDevice>());
    auto scheduler = std::make_unique<viz::DisplayScheduler>(
        begin_frame_source_.get(), task_runner.get(),
        output_surface->capabilities().pending_swap_params);
    viz::RendererSettings settings = viz::CreateRendererSettings();
    // settings.use_skia_renderer = false;
    auto overlay_processor = std::make_unique<viz::OverlayProcessorStub>();
    display_ = std::make_unique<viz::Display>(
        shared_bitmap_manager_.get(), settings, &debug_settings_, root_frame_sink_id_,
        nullptr, std::move(output_surface), std::move(overlay_processor),
        std::move(scheduler), task_runner);
    display_->Initialize(this, frame_sink_manager_->surface_manager());

    frame_sink_manager_->RegisterFrameSinkId(root_frame_sink_id_, false);
    frame_sink_manager_->RegisterBeginFrameSource(begin_frame_source_.get(),
                                                  root_frame_sink_id_);
    display_->Resize(size_);
    display_->SetVisible(true);
    support_->SetNeedsBeginFrame(true);
  }

  viz::CompositorFrame CreateFrame(const ::viz::BeginFrameArgs& args) {
    constexpr SkColor4f colors[] = {SkColors::kRed, SkColors::kGreen, SkColors::kYellow};
    viz::CompositorFrame frame;

    frame.metadata.begin_frame_ack = viz::BeginFrameAck(args, true);
    frame.metadata.device_scale_factor = 1.f;
    // frame.metadata.local_surface_id_allocation_time =
    //     root_local_surface_id_.allocation_time();
    frame.metadata.frame_token = ++frame_token_generator_;

    const int kRenderPassId = 1;
    const gfx::Rect& output_rect = gfx::Rect(size_);
    const gfx::Rect& damage_rect = output_rect;
    std::unique_ptr<viz::CompositorRenderPass> render_pass = viz::CompositorRenderPass::Create();
    render_pass->SetNew(viz::CompositorRenderPassId{kRenderPassId}, output_rect, damage_rect,
                        gfx::Transform());

    // Add a solid-color draw-quad for the big rectangle covering the entire
    // content-area of the client.
    viz::SharedQuadState* quad_state =
        render_pass->CreateAndAppendSharedQuadState();
    quad_state->SetAll(gfx::Transform(),
                       /*layer_rect=*/output_rect,
                       /*visible_layer_rect=*/output_rect,
                       /*filter_info=*/gfx::MaskFilterInfo(),
                       /*clip=*/absl::nullopt,
                       /*contents_opaque=*/false, /*opacity_f=*/1.f,
                       /*blend=*/SkBlendMode::kSrcOver,
                       /*sorting_context=*/0);

    viz::SolidColorDrawQuad* color_quad =
        render_pass->CreateAndAppendDrawQuad<viz::SolidColorDrawQuad>();
    color_quad->SetNew(quad_state, output_rect, output_rect,
                       colors[(*frame_token_generator_) % std::size(colors)],
                       false);

    frame.render_pass_list.push_back(std::move(render_pass));

    return frame;
  }

  void DidReceiveCompositorFrameAck(
      std::vector<viz::ReturnedResource> resources) override {}

  void OnBeginFrame(
      const ::viz::BeginFrameArgs& args,
      const base::flat_map<uint32_t, ::viz::FrameTimingDetails>& details)
      override {
    DLOG(INFO) << "OnBeginFrame: submit a new frame";
    if (support_->last_activated_local_surface_id() != root_local_surface_id_) {
      display_->SetLocalSurfaceId(root_local_surface_id_, 1.0f);
    }
    support_->SubmitCompositorFrame(root_local_surface_id_, CreateFrame(args),
                                    absl::optional<viz::HitTestRegionList>(),
                                    /*submit_time=*/0);
  }

  void OnBeginFramePausedChanged(bool paused) override {}
  void OnCompositorFrameTransitionDirectiveProcessed(uint32_t sequence_id) override {}
  void ReclaimResources(
      std::vector<::viz::ReturnedResource> resources) override {}

  // viz::DisplayClient overrides.
  void DisplayAddChildWindowToBrowser(gpu::SurfaceHandle child_window) override {}
  void DisplayOutputSurfaceLost() override {}
  void DisplayWillDrawAndSwap(
      bool will_draw_and_swap,
      viz::AggregatedRenderPassList* render_passes) override {}
  void DisplayDidDrawAndSwap() override {}
  void SetWideColorEnabled(bool enabled) override {}
  void DisplayDidReceiveCALayerParams(
      const gfx::CALayerParams& ca_layer_params) override {}
  void DisplayDidCompleteSwapWithSize(const gfx::Size& pixel_size) override {}
  void SetPreferredFrameInterval(base::TimeDelta interval) override {}
  base::TimeDelta GetPreferredFrameIntervalForFrameSinkId(
      const viz::FrameSinkId& id,
      viz::mojom::CompositorFrameSinkType* type) override {
    return frame_sink_manager_->GetPreferredFrameIntervalForFrameSinkId(id, type);
  }

  base::Thread thread_;
  std::unique_ptr<viz::ServerSharedBitmapManager> shared_bitmap_manager_;
  std::unique_ptr<viz::FrameSinkManagerImpl> frame_sink_manager_;
  std::unique_ptr<viz::CompositorFrameSinkSupport> support_;
  std::unique_ptr<viz::DelayBasedBeginFrameSource> begin_frame_source_;
  const viz::DebugRendererSettings debug_settings_;
  std::unique_ptr<viz::Display> display_;
  // 由于要将显示存储为图片，所以使用1FPS
  double fps_ = 1.0;
  // 画面大小为 300x200
  gfx::Size size_{300, 200};

  viz::FrameSinkId root_frame_sink_id_{0, 1};
  viz::ParentLocalSurfaceIdAllocator root_local_surface_id_allocator_;
  viz::LocalSurfaceId root_local_surface_id_;
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
  auto trace_config =
      base::trace_event::TraceConfig("viz" /*, "trace-to-console"*/);
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

  ui::RegisterPathProvider();

  base::RunLoop run_loop;

  // Make Ozone run in single-process mode.
  ui::OzonePlatform::InitParams params;
  params.single_process = true;
  ui::OzonePlatform::InitializeForUI(params);
  ui::OzonePlatform::InitializeForGPU(params);

  // 每秒生成一张图片保存到文件中
  // 可以使用这种原理将浏览器嵌入其他程序，当然这个demo演示的并不是最优方案，只是一种可行方案
  demo::OffscreenRenderer renderer;

  LOG(INFO) << "running...";
  run_loop.Run();

  return 0;
}
