#include "base/at_exit.h"
#include "base/base_paths.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/files/file.h"
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
#include "base/trace_event/trace_buffer.h"
#include "build/build_config.h"
#include "build/buildflag.h"
#include "cc/animation/animation_host.h"
#include "cc/layers/content_layer_client.h"
#include "cc/layers/layer.h"
#include "cc/layers/picture_layer.h"
#include "cc/test/test_task_graph_runner.h"
#include "cc/trees/layer_tree_frame_sink.h"
#include "cc/trees/layer_tree_frame_sink_client.h"
#include "cc/trees/layer_tree_host.h"
#include "components/viz/common/quads/solid_color_draw_quad.h"
#include "components/viz/demo/host/demo_host.h"
#include "components/viz/demo/service/demo_service.h"
#include "components/viz/host/host_frame_sink_manager.h"
#include "components/viz/host/renderer_settings_creation.h"
#include "components/viz/service/display/display.h"
#include "components/viz/service/display/software_output_device.h"
#include "components/viz/service/display_embedder/output_surface_provider.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"
#include "components/viz/service/display_embedder/software_output_device_x11.h"
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

// 负责绘制要显示的内容
class Layer : public cc::ContentLayerClient {
 public:
  Layer() {
    content_cc_layer_ = cc::PictureLayer::Create(this);

    content_cc_layer_->SetTransformOrigin(gfx::Point3F());
    content_cc_layer_->SetContentsOpaque(true);
    content_cc_layer_->SetSafeOpaqueBackgroundColor(SK_ColorWHITE);
    content_cc_layer_->SetIsDrawable(true);
    content_cc_layer_->SetHitTestable(true);
    content_cc_layer_->SetElementId(cc::ElementId(content_cc_layer_->id()));
    content_cc_layer_->SetBounds(bounds_.size());
  }

  // 设置root cc layer
  void SetCompositor(scoped_refptr<cc::Layer> root_cc_layer) {
    root_cc_layer->AddChild(content_cc_layer_);
  }

  void SetBounds(const gfx::Rect& bounds) {
    bounds_ = bounds;
    content_cc_layer_->SetPosition(gfx::PointF(bounds_.origin()));
    content_cc_layer_->SetBounds(bounds_.size());
  }

  // ContentLayerClient implementation.
  gfx::Rect PaintableRegion() override { return bounds_; }
  // 绘制要显示的内容
  scoped_refptr<cc::DisplayItemList> PaintContentsToDisplayList(
      ContentLayerClient::PaintingControlSetting painting_control) override {
    LOG(INFO) << "PaintableRegion: paint layer";
    constexpr SkColor colors[] = {SK_ColorRED, SK_ColorGREEN, SK_ColorYELLOW};
    static int i = 0;
    SkColor color = colors[i++ % base::size(colors)];
    auto display_list = base::MakeRefCounted<cc::DisplayItemList>();
    display_list->StartPaint();
    display_list->push<cc::DrawColorOp>(color, SkBlendMode::kSrc);
    display_list->EndPaintOfUnpaired(bounds_);
    display_list->Finalize();
    base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(
            [](scoped_refptr<cc::PictureLayer> layer, SkColor color, int i) {
              LOG(INFO) << "layer->SetNeedsCommit()";
              // 将整个layer标记为demaged，它内部会调用SetNeedsCommit()
              // 也可以使用 LayerTreeHost::SetNeedsAnimate() 强制更新
              layer->SetNeedsDisplay();
            },
            content_cc_layer_, color, i),
        base::TimeDelta::FromSeconds(1));
    return display_list;
  }
  bool FillsBoundsCompletely() const override { return true; }
  size_t GetApproximateUnsharedMemoryUsage() const override { return 1; }

 private:
  gfx::Rect bounds_;
  scoped_refptr<cc::PictureLayer> content_cc_layer_;
};

// 连接cc和viz，cc使用它向viz提交CompositorFrame
class DemoLayerTreeFrameSink : public cc::LayerTreeFrameSink,
                               public viz::mojom::CompositorFrameSinkClient,
                               public viz::DisplayClient,
                               public viz::ExternalBeginFrameSourceClient {
 public:
  DemoLayerTreeFrameSink(
      gfx::AcceleratedWidget widget,
      viz::FrameSinkId& frame_sink_id,
      viz::LocalSurfaceIdAllocation local_surface_id,
      viz::FrameSinkManagerImpl* frame_sink_manager,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner)
      : cc::LayerTreeFrameSink(nullptr,
                               nullptr,
                               std::move(task_runner),
                               nullptr),
        widget_(widget),
        root_frame_sink_id_(frame_sink_id),
        root_local_surface_id_(local_surface_id),
        frame_sink_manager_(frame_sink_manager) {
    Initialize();
  }

  ~DemoLayerTreeFrameSink() override {}

 private:
  void Initialize() {
    auto task_runner = base::ThreadTaskRunnerHandle::Get();

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
        std::make_unique<viz::SoftwareOutputDeviceX11>(widget_, nullptr));
    auto scheduler = std::make_unique<viz::DisplayScheduler>(
        begin_frame_source_.get(), task_runner.get(),
        output_surface->capabilities().max_frames_pending);
    viz::RendererSettings settings = viz::CreateRendererSettings();
    settings.use_skia_renderer = false;
    display_ = std::make_unique<viz::Display>(
        frame_sink_manager_->shared_bitmap_manager(), settings,
        root_frame_sink_id_, std::move(output_surface), std::move(scheduler),
        task_runner);
    display_->Initialize(this, frame_sink_manager_->surface_manager());
    frame_sink_manager_->RegisterFrameSinkId(root_frame_sink_id_, false);
    frame_sink_manager_->RegisterBeginFrameSource(begin_frame_source_.get(),
                                                  root_frame_sink_id_);
    display_->Resize(size_);
    display_->SetVisible(true);
  }

  // cc::LayerTreeFrameSink implementation.
  bool BindToClient(cc::LayerTreeFrameSinkClient* client) override {
    if (!cc::LayerTreeFrameSink::BindToClient(client))
      return false;
    // 用于将OnBeginFrame请求转发到 cc::Scheduler 进行调度
    external_begin_frame_source_ =
        std::make_unique<viz::ExternalBeginFrameSource>(this);
    // 主要目的是将cc::Scheduler加入到BFS的observer中
    client_->SetBeginFrameSource(external_begin_frame_source_.get());
    return true;
  }
  void DetachFromClient() override {
    client_->SetBeginFrameSource(nullptr);
    external_begin_frame_source_.reset();

    // Unregister the SurfaceFactoryClient here instead of the dtor so that only
    // one client is alive for this namespace at any given time.
    // support_.reset();

    cc::LayerTreeFrameSink::DetachFromClient();
  }
  // 接收由 cc 提交的 CF
  void SubmitCompositorFrame(viz::CompositorFrame frame,
                             bool hit_test_data_changed,
                             bool show_hit_test_borders) override {
    support_->SubmitCompositorFrame(root_local_surface_id_.local_surface_id(),
                                    std::move(frame),
                                    base::Optional<viz::HitTestRegionList>(),
                                    /*trace_time=*/0);
  }
  void DidNotProduceFrame(const viz::BeginFrameAck& ack) override {
    support_->DidNotProduceFrame(ack);
  }
  void DidAllocateSharedBitmap(base::ReadOnlySharedMemoryRegion region,
                               const viz::SharedBitmapId& id) override {}
  void DidDeleteSharedBitmap(const viz::SharedBitmapId& id) override {}

  // ExternalBeginFrameSourceClient implementation:
  void OnNeedsBeginFrames(bool needs_begin_frames) override {
    // 这里被cc中的scheduler调用，通知viz发起新的BeginFrame请求
    support_->SetNeedsBeginFrame(needs_begin_frames);
  }

  // viz::mojom::CompositorFrameSinkClient overrides.
  virtual void DidReceiveCompositorFrameAck(
      const std::vector<::viz::ReturnedResource>& resources) override {
    // Submitting a CompositorFrame can synchronously draw and dispatch a frame
    // ack. PostTask to ensure the client is notified on a new stack frame.
    compositor_task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(
            &DemoLayerTreeFrameSink::DidReceiveCompositorFrameAckInternal,
            weak_factory_.GetWeakPtr(), resources));
  }
  void DidReceiveCompositorFrameAckInternal(
      const std::vector<viz::ReturnedResource>& resources) {
    client_->ReclaimResources(resources);
    // 用于告诉cc::Scheduler上一帧已经处理完了
    client_->DidReceiveCompositorFrameAck();
  }
  virtual void OnBeginFrame(
      const ::viz::BeginFrameArgs& args,
      const base::flat_map<uint32_t, ::viz::FrameTimingDetails>& timing_details)
      override {
    TRACE_EVENT0("cc", "DemoLayerTreeFrameSink::OnBeginFrame");
    LOG(INFO) << "OnBeginFrame: submit a new frame";

    // 这里用于结束PipelineReporter的
    // SubmitCompositorFrameToPresentationCompositorFrame
    // PipelineReporter用于统计cc各阶段的耗时，可以在chrome://tracing中查看
    for (const auto& pair : timing_details)
      client_->DidPresentCompositorFrame(pair.first, pair.second);

    if (support_->last_activated_local_surface_id() !=
        root_local_surface_id_.local_surface_id()) {
      display_->SetLocalSurfaceId(root_local_surface_id_.local_surface_id(),
                                  1.0f);
    }
    // 将来自viz的 OnBeginFrame 转发到cc中的调度器 cc::Scheduler
    // 从这里可以看出渲染的驱动是自底向上的，由底层的viz发起新的Frame请求
    // 当cc需要更新画面时，cc会间接调用到 OnNeedsBeginFrames
    // 方法通知viz发起新Frame的请求 注意webview中渲染的驱动不是自底向上的
    // 这句代码是 cc:::Scheduler 的发动机之一
    external_begin_frame_source_->OnBeginFrame(args);
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

  // 方便调试使用1FPS
  double fps_ = 1.0;
  // 画面大小为 300x200
  gfx::Size size_{300, 200};
  gfx::AcceleratedWidget widget_;
  viz::FrameSinkId root_frame_sink_id_{0, 1};
  viz::ParentLocalSurfaceIdAllocator root_local_surface_id_allocator_;
  viz::LocalSurfaceIdAllocation root_local_surface_id_;
  viz::FrameTokenGenerator frame_token_generator_;
  std::unique_ptr<viz::FrameSinkManagerImpl> frame_sink_manager_;
  base::WeakPtrFactory<DemoLayerTreeFrameSink> weak_factory_{this};
  std::unique_ptr<viz::ExternalBeginFrameSource> external_begin_frame_source_;
  std::unique_ptr<viz::DelayBasedBeginFrameSource> begin_frame_source_;
  std::unique_ptr<viz::CompositorFrameSinkSupport> support_;
  std::unique_ptr<viz::Display> display_;
};

// 作用类似
// ui::Compositor,负责初始化cc和viz，它将上层ui层和底层的cc和viz对接起来
class Compositor
    : public cc::LayerTreeHostClient,
      // 这个接口用于不使用cc::Scheduler的模式下，该demo使用cc::Scheduelr
      public cc::LayerTreeHostSingleThreadClient,
      public viz::HostFrameSinkClient {
 public:
  Compositor(gfx::AcceleratedWidget widget) : widget_(widget) {
    auto task_runner = base::ThreadTaskRunnerHandle::Get();
    cc::LayerTreeSettings settings;
    settings.initial_debug_state.show_fps_counter = true;

    animation_host_ = cc::AnimationHost::CreateMainInstance();

    cc::LayerTreeHost::InitParams params;
    params.client = this;
    params.task_graph_runner = &task_graph_runner_;
    params.settings = &settings;
    params.main_task_runner = task_runner;
    params.mutator_host = animation_host_.get();
    host_ = cc::LayerTreeHost::CreateSingleThreaded(this, std::move(params));

    root_cc_layer_ = cc::Layer::Create();
    host_->SetRootLayer(root_cc_layer_);
    host_->SetVisible(true);

    root_cc_layer_->SetBounds(size_);
    root_ui_layer_.SetBounds(gfx::Rect(size_));
    root_ui_layer_.SetCompositor(root_cc_layer_);

    host_->SetNeedsRedrawRect(gfx::Rect(size_));
    host_->SetNeedsCommit();
  }

  virtual ~Compositor() {}

 private:
  // 画面大小为 300x200
  gfx::Size size_{300, 200};
  float scale_ = 1.0;
  gfx::AcceleratedWidget widget_;
  scoped_refptr<cc::Layer> root_cc_layer_;
  demo::Layer root_ui_layer_;
  std::unique_ptr<viz::ServerSharedBitmapManager> shared_bitmap_manager_;
  viz::FrameSinkId root_frame_sink_id_{0, 1};
  viz::ParentLocalSurfaceIdAllocator root_local_surface_id_allocator_;
  viz::LocalSurfaceIdAllocation root_local_surface_id_;
  std::unique_ptr<cc::LayerTreeHost> host_;

  std::unique_ptr<viz::FrameSinkManagerImpl> frame_sink_manager_;
  cc::TestTaskGraphRunner task_graph_runner_;
  std::unique_ptr<cc::AnimationHost> animation_host_;

  // LayerTreeHostClient implementation.
  void WillBeginMainFrame() override {}
  void DidBeginMainFrame() override {}
  void OnDeferMainFrameUpdatesChanged(bool) override {}
  void OnDeferCommitsChanged(bool) override {}
  void WillUpdateLayers() override {}
  void DidUpdateLayers() override {}
  void BeginMainFrame(const viz::BeginFrameArgs& args) override {}
  void BeginMainFrameNotExpectedSoon() override {}
  void BeginMainFrameNotExpectedUntil(base::TimeTicks time) override {}
  void UpdateLayerTreeHost() override {
    root_cc_layer_->SetNeedsDisplayRect(gfx::Rect(size_));
  }
  void ApplyViewportChanges(const cc::ApplyViewportChangesArgs& args) override {
  }
  void RecordManipulationTypeCounts(cc::ManipulationInfo info) override {}
  void SendOverscrollEventFromImplSide(
      const gfx::Vector2dF& overscroll_delta,
      cc::ElementId scroll_latched_element_id) override {}
  void SendScrollEndEventFromImplSide(
      cc::ElementId scroll_latched_element_id) override {}
  void RequestNewLayerTreeFrameSink() override {
    auto task_runner = base::ThreadTaskRunnerHandle::Get();

    shared_bitmap_manager_ = std::make_unique<viz::ServerSharedBitmapManager>();
    frame_sink_manager_ = std::make_unique<viz::FrameSinkManagerImpl>(
        shared_bitmap_manager_.get());

    // 生成 root client 的 LocalSurfaceId
    root_local_surface_id_allocator_.GenerateId();
    root_local_surface_id_ =
        root_local_surface_id_allocator_.GetCurrentLocalSurfaceIdAllocation();

    auto layer_tree_frame_sink = std::make_unique<DemoLayerTreeFrameSink>(
        widget_, root_frame_sink_id_, root_local_surface_id_,
        frame_sink_manager_.get(), task_runner);

    host_->SetViewportRectAndScale(gfx::Rect(size_), scale_,
                                   root_local_surface_id_);

    // 这会触发 OnBeginFrame 和 LayerTreeFrameSink::BindToClient()
    host_->SetLayerTreeFrameSink(std::move(layer_tree_frame_sink));
  }
  void DidInitializeLayerTreeFrameSink() override {}
  void DidFailToInitializeLayerTreeFrameSink() override {}
  void WillCommit() override {}
  void DidCommit() override {}
  void DidCommitAndDrawFrame() override {}
  void DidReceiveCompositorFrameAck() override {}
  void DidCompletePageScaleAnimation() override {}
  void DidPresentCompositorFrame(
      uint32_t frame_token,
      const gfx::PresentationFeedback& feedback) override {
    TRACE_EVENT_MARK_WITH_TIMESTAMP1("cc,benchmark", "FramePresented",
                                     feedback.timestamp, "environment",
                                     "browser");
  }
  void RecordStartOfFrameMetrics() override {}
  void RecordEndOfFrameMetrics(base::TimeTicks frame_begin_time) override {}
  std::unique_ptr<cc::BeginMainFrameMetrics> GetBeginMainFrameMetrics()
      override {
    return nullptr;
  }

  // cc::LayerTreeHostSingleThreadClient implementation.
  void DidSubmitCompositorFrame() override {}
  void DidLoseLayerTreeFrameSink() override {}
  void FrameIntervalUpdated(base::TimeDelta interval) override {}

  // viz::HostFrameSinkClient implementation.
  void OnFirstSurfaceActivation(const viz::SurfaceInfo& surface_info) override {
  }
  void OnFrameTokenChanged(uint32_t frame_token) override {}
};

// 窗口类
class DemoCcWindow : public ui::PlatformWindowDelegate {
 public:
  DemoCcWindow(base::OnceClosure close_closure)
      : close_closure_(std::move(close_closure)) {}
  ~DemoCcWindow() override = default;

  void Create(const gfx::Rect& bounds) {
    platform_window_ = CreatePlatformWindow(bounds);
    platform_window_->Show();
    if (widget_ != gfx::kNullAcceleratedWidget)
      InitializeDemo();
  }

 private:
  std::unique_ptr<ui::PlatformWindow> CreatePlatformWindow(
      const gfx::Rect& bounds) {
    ui::PlatformWindowInitProperties props(bounds);
#if defined(USE_OZONE)
    return ui::OzonePlatform::GetInstance()->CreatePlatformWindow(
        this, std::move(props));
#elif defined(OS_WIN)
    return std::make_unique<ui::WinWindow>(this, props.bounds);
#elif defined(USE_X11)
    auto x11_window = std::make_unique<ui::X11Window>(this);
    x11_window->Initialize(std::move(props));
    return x11_window;
#else
    NOTIMPLEMENTED();
    return nullptr;
#endif
  }

  void InitializeDemo() {
    DCHECK_NE(widget_, gfx::kNullAcceleratedWidget);
    compositor_ = std::make_unique<Compositor>(widget_);
  }

  // ui::PlatformWindowDelegate:
  void OnBoundsChanged(const gfx::Rect& new_bounds) override {
    // compositor_->Resize(new_bounds.size());
  }

  void OnAcceleratedWidgetAvailable(gfx::AcceleratedWidget widget) override {
    widget_ = widget;
    if (platform_window_)
      InitializeDemo();
  }

  void OnDamageRect(const gfx::Rect& damaged_region) override {}
  void DispatchEvent(ui::Event* event) override {}
  void OnCloseRequest() override {
    // TODO: Use a more robust exit method
    compositor_.reset();
    platform_window_->Close();
  }
  void OnClosed() override {
    if (close_closure_)
      std::move(close_closure_).Run();
  }
  void OnWindowStateChanged(ui::PlatformWindowState new_state) override {}
  void OnLostCapture() override {}
  void OnAcceleratedWidgetDestroyed() override {}
  void OnActivationChanged(bool active) override {}
  void OnMouseEnter() override {}

  std::unique_ptr<Compositor> compositor_;

  std::unique_ptr<ui::PlatformWindow> platform_window_;
  gfx::AcceleratedWidget widget_;
  base::OnceClosure close_closure_;

  DISALLOW_COPY_AND_ASSIGN(DemoCcWindow);
};

void FlushTrace() {
  DLOG(INFO) << "Flush trace start.";
  base::FilePath output_path;
  DCHECK(base::PathService::Get(base::DIR_EXE, &output_path));
  output_path = output_path.AppendASCII("trace_demo_cc_gui.json");
  static base::File trace_file(
      output_path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  DCHECK(trace_file.IsValid());
  trace_file.SetLength(0);
  static base::trace_event::TraceResultBuffer trace_buffer;
  trace_buffer.SetOutputCallback(
      base::BindRepeating([](const std::string& chunk) {
        trace_file.WriteAtCurrentPos(chunk.c_str(), chunk.size());
      }));
  trace_buffer.Start();
  // 停止接收新的 Trace
  base::trace_event::TraceLog::GetInstance()->SetDisabled();
  // 获取 Trace 的结果，必须要先停止接收 Trace 才能执行 Flush
  base::trace_event::TraceLog::GetInstance()->Flush(base::BindRepeating(
      [](const scoped_refptr<base::RefCountedString>& events_str,
         bool has_more_events) {
        trace_buffer.AddFragment(events_str->data());
        if (!has_more_events) {
          trace_buffer.Finish();
          trace_file.Flush();
          DLOG(INFO) << "Flush trace finished.";
        }
      }));
}

}  // namespace demo

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  base::CommandLine::ForCurrentProcess()->AppendSwitch(
      "enable-skia-benchmarking");
  // 设置日志格式
  logging::SetLogItems(true, true, true, false);
  // 创建主消息循环，等价于 MessagLoop
  base::SingleThreadTaskExecutor main_task_executor(base::MessagePumpType::UI);
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("DemoViews");

  // 手动创建TraceConfig
  auto trace_config =
      base::trace_event::TraceConfig("*,disabled-by-default-*", "");
  // 2. 启动Trace
  base::trace_event::TraceLog::GetInstance()->SetEnabled(
      trace_config, base::trace_event::TraceLog::RECORDING_MODE);

  // 初始化mojo
  mojo::core::Init();
  base::Thread mojo_thread("mojo");
  mojo_thread.StartWithOptions(
      base::Thread::Options(base::MessagePumpType::IO, 0));
  auto ipc_support = std::make_unique<mojo::core::ScopedIPCSupport>(
      mojo_thread.task_runner(),
      mojo::core::ScopedIPCSupport::ShutdownPolicy::CLEAN);

  // 在Linux上，x11和aura都是默认开启的
#if defined(USE_X11)
  // This demo uses InProcessContextFactory which uses X on a separate Gpu
  // thread.
  gfx::InitializeThreadedX11();

  // 设置X11的异常处理回调，如果不设置在很多设备上会频繁出现崩溃。
  // 比如 ui::XWindow::Close() 和~SGIVideoSyncProviderThreadShim 的析构中
  // 都调用了 XDestroyWindow() ，并且是在不同的线程中调用的，当这两个窗口有
  // 父子关系的时候，如果先调用了父窗口的销毁再调用子窗口的销毁则会导致BadWindow
  // 错误，默认的Xlib异常处理会打印错误日志然后强制结束程序。
  // 这些错误大多是并发导致的代码执行顺序问题，所以修改起来没有那么容易。
  ui::SetDefaultX11ErrorHandlers();
#endif

  auto event_source_ = ui::PlatformEventSource::CreateDefault();

  // 加载相应平台的GL库及GL绑定
  gl::init::InitializeGLOneOff();

  // 初始化ICU(i18n),也就是icudtl.dat，views依赖ICU
  base::i18n::InitializeICU();

  ui::RegisterPathProvider();

  // This app isn't a test and shouldn't timeout.
  base::RunLoop::ScopedDisableRunTimeoutForTest disable_timeout;

  base::RunLoop run_loop;

  demo::DemoCcWindow window(run_loop.QuitClosure());
  window.Create(gfx::Rect(800, 600));

  main_task_executor.task_runner()->PostDelayedTask(
      FROM_HERE, base::BindOnce(&demo::FlushTrace),
      base::TimeDelta::FromSeconds(12));

  LOG(INFO) << "running...";
  run_loop.Run();

  return 0;
}
