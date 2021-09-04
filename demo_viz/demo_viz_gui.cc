#include "base/at_exit.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/single_thread_task_runner.h"
#include "base/message_loop/message_pump_type.h"
#include "base/path_service.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/rand_util.h"
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
#include "components/viz/client/client_resource_provider.h"
#include "components/viz/common/quads/debug_border_draw_quad.h"
#include "components/viz/common/quads/picture_draw_quad.h"
#include "components/viz/common/quads/solid_color_draw_quad.h"
#include "components/viz/common/quads/surface_draw_quad.h"
#include "components/viz/common/quads/texture_draw_quad.h"
#include "components/viz/common/quads/tile_draw_quad.h"
#include "components/viz/common/quads/video_hole_draw_quad.h"
#include "components/viz/common/resources/bitmap_allocation.h"
#include "components/viz/common/resources/resource_format.h"
#include "components/viz/demo/host/demo_host.h"
#include "components/viz/demo/service/demo_service.h"
#include "components/viz/host/host_frame_sink_manager.h"
#include "components/viz/host/renderer_settings_creation.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"
#include "components/viz/service/frame_sinks/frame_sink_manager_impl.h"
#include "components/viz/service/main/viz_compositor_thread_runner_impl.h"
#include "demo/common/utils.h"
#include "mojo/core/embedder/embedder.h"
#include "mojo/core/embedder/scoped_ipc_support.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "ui/base/hit_test.h"
#include "ui/base/ime/init/input_method_initializer.h"
// #include "ui/base/material_design/material_design_controller.h"
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
// #include "ui/gfx/x/x11_connection.h"            // nogncheck
#include "ui/platform_window/x11/x11_window.h"  // nogncheck
#endif

#if defined(USE_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#endif

#if defined(OS_WIN)
// #include "ui/base/cursor/cursor_loader_win.h"
#include "ui/platform_window/win/win_window.h"
#endif

namespace demo {

constexpr SkColor colors[] = {SK_ColorRED, SK_ColorGREEN, SK_ColorYELLOW};

// Client 端
// 在 Chromium 中 cc::LayerTreeFrameSink 的作用就相当于 viz 中的 client.
class LayerTreeFrameSink : public viz::mojom::CompositorFrameSinkClient {
 public:
  LayerTreeFrameSink(const viz::FrameSinkId& frame_sink_id,
                     const viz::LocalSurfaceId& local_surface_id,
                     const gfx::Rect& bounds)
      : frame_sink_id_(frame_sink_id),
        local_surface_id_(local_surface_id),
        bounds_(bounds),
        thread_("Demo_" + frame_sink_id.ToString()) {
    CHECK(thread_.Start());
  }

  virtual ~LayerTreeFrameSink() {}

  // remote 和 associated_remote 只能一个有效.
  // remote 用于非 root 的 client, associated_remote 用于 root client.
  // 这表示 root client 提交 CF 的过程需要和 FSM 的调用进行同步,而其他 client
  // 可以在单独的通信通道中提交 CF.
  void Bind(
      mojo::PendingReceiver<viz::mojom::CompositorFrameSinkClient> receiver,
      mojo::PendingAssociatedRemote<viz::mojom::CompositorFrameSink>
          associated_remote) {
    thread_.task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&LayerTreeFrameSink::BindOnThread,
                       base::Unretained(this), std::move(receiver),
                       std::move(associated_remote), mojo::NullRemote()));
  }

  void Bind(
      mojo::PendingReceiver<viz::mojom::CompositorFrameSinkClient> receiver,
      mojo::PendingRemote<viz::mojom::CompositorFrameSink> remote) {
    thread_.task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&LayerTreeFrameSink::BindOnThread,
                       base::Unretained(this), std::move(receiver),
                       mojo::NullAssociatedRemote(), std::move(remote)));
  }

  viz::LocalSurfaceIdAllocation EmbedChild(
      const viz::FrameSinkId& child_frame_sink_id) {
    base::AutoLock lock(lock_);
    child_frame_sink_id_ = child_frame_sink_id;
    local_surface_id_allocator_.GenerateId();
    return local_surface_id_allocator_.GetCurrentLocalSurfaceIdAllocation();
  }

 private:
  void BindOnThread(
      mojo::PendingReceiver<viz::mojom::CompositorFrameSinkClient> receiver,
      mojo::PendingAssociatedRemote<viz::mojom::CompositorFrameSink>
          associated_remote,
      mojo::PendingRemote<viz::mojom::CompositorFrameSink> remote) {
    receiver_.Bind(std::move(receiver));
    if (associated_remote) {
      frame_sink_associated_remote_.Bind(std::move(associated_remote));
    } else {
      frame_sink_remote_.Bind(std::move(remote));
    }
    // 告诉 CompositorFrameSink 可以开始请求 CompositorFrame 了
    GetCompositorFrameSinkPtr()->SetNeedsBeginFrame(true);
    client_resource_provider_ =
        std::make_unique<viz::ClientResourceProvider>(false);
  }

  viz::CompositorFrame CreateFrame(const ::viz::BeginFrameArgs& args) {
    TRACE_EVENT0("viz", "LayerTreeFrameSink::CreateFrame")

    viz::CompositorFrame frame;
    frame.metadata.begin_frame_ack = viz::BeginFrameAck(args, true);
    frame.metadata.device_scale_factor = 1.f;
    frame.metadata.local_surface_id_allocation_time =
        local_surface_id_.allocation_time();
    frame.metadata.frame_token = ++frame_token_generator_;

    const int kRenderPassId = 1;
    const gfx::Rect& output_rect = bounds_;
    const gfx::Rect& damage_rect = output_rect;
    std::unique_ptr<viz::RenderPass> render_pass = viz::RenderPass::Create();
    render_pass->SetNew(kRenderPassId, output_rect, damage_rect,
                        gfx::Transform());

    AppendDebugBorderDrawQuad(frame, render_pass.get());

    if (child_frame_sink_id_.is_valid()) {
      AppendSurfaceDrawQuad(frame, render_pass.get());
    }

    AppendTileDrawQuad(frame, render_pass.get());
    AppendTextureDrawQuad(frame, render_pass.get());
    AppendPictureDrawQuad(frame, render_pass.get());
    AppendVideoHoleDrawQuad(frame, render_pass.get());
    AppendSolidColorDrawQuad(frame, render_pass.get());

    frame.render_pass_list.push_back(std::move(render_pass));

    return frame;
  }

  void AppendDebugBorderDrawQuad(viz::CompositorFrame& frame,
                                 viz::RenderPass* render_pass) {
    gfx::Rect output_rect = bounds_;

    auto* quad_state = render_pass->CreateAndAppendSharedQuadState();
    quad_state->SetAll(
        gfx::Transform(),
        /*quad_layer_rect=*/output_rect,
        /*visible_quad_layer_rect=*/output_rect,
        /*rounded_corner_bounds=*/gfx::RRectF(),
        /*clip_rect=*/output_rect,
        /*is_clipped=*/false, /*are_contents_opaque=*/false, /*opacity=*/1.f,
        /*blend_mode=*/SkBlendMode::kSrcOver, /*sorting_context_id=*/0);

    auto* debug_quad =
        render_pass->CreateAndAppendDrawQuad<viz::DebugBorderDrawQuad>();
    // 将 resource 添加到 tile_quad 中
    debug_quad->SetNew(quad_state, output_rect, output_rect, SK_ColorMAGENTA,
                       20);
  }

  // 演示 TileDrawQuad 的使用
  void AppendTileDrawQuad(viz::CompositorFrame& frame,
                          viz::RenderPass* render_pass) {
    // 创建要渲染的内容到 SkBitmap 中
    SkBitmap bitmap;
    bitmap.allocN32Pixels(200, 200);
    SkCanvas canvas(bitmap);
    canvas.clear(SK_ColorWHITE);
    canvas.drawCircle(
        30, 100, 150,
        SkPaint(SkColor4f::FromColor(
            colors[(*frame_token_generator_ / 60 + 1) % base::size(colors)])));
    canvas.drawCircle(
        10, 50, 60,
        SkPaint(SkColor4f::FromColor(
            colors[(*frame_token_generator_ / 60 + 2) % base::size(colors)])));
    canvas.drawCircle(
        180, 180, 50,
        SkPaint(SkColor4f::FromColor(
            colors[(*frame_token_generator_ / 60 + 3) % base::size(colors)])));

    gfx::Size tile_size(200, 200);
    // 将 SkBitmap 中的数据转换为资源
    viz::ResourceId resource =
        AllocateAndFillSoftwareResource(tile_size, bitmap);

    gfx::Rect output_rect{0, 0, 200, 200};
    gfx::Transform transform;
    transform.Translate(50, 50);

    auto* quad_state = render_pass->CreateAndAppendSharedQuadState();
    quad_state->SetAll(
        transform,
        /*quad_layer_rect=*/output_rect,
        /*visible_quad_layer_rect=*/output_rect,
        /*rounded_corner_bounds=*/gfx::RRectF(),
        /*clip_rect=*/output_rect,
        /*is_clipped=*/false, /*are_contents_opaque=*/false, /*opacity=*/1.f,
        /*blend_mode=*/SkBlendMode::kSrcOver, /*sorting_context_id=*/0);

    auto* tile_quad = render_pass->CreateAndAppendDrawQuad<viz::TileDrawQuad>();
    // 将 resource 添加到 tile_quad 中
    tile_quad->SetNew(quad_state, output_rect, output_rect, false, resource,
                      gfx::RectF(output_rect), output_rect.size(), true, true,
                      true);

    // 将 resource 对用的资源添加到 frame.resource_list
    // 中，在最简单的情况下可以直接使用 frame.resource_list.push_back(...)
    // 来添加
    client_resource_provider_->PrepareSendToParent(
        {resource}, &frame.resource_list, (viz::RasterContextProvider*)nullptr);
  }

  // 演示 TextureDrawQuad 的使用
  void AppendTextureDrawQuad(viz::CompositorFrame& frame,
                             viz::RenderPass* render_pass) {
    // 创建要渲染的内容到 SkBitmap 中
    SkBitmap bitmap;
    bitmap.allocN32Pixels(200, 200);
    SkCanvas canvas(bitmap);
    canvas.clear(SK_ColorWHITE);
    canvas.drawCircle(
        30, 100, 150,
        SkPaint(SkColor4f::FromColor(
            colors[(*frame_token_generator_ / 60 + 2) % base::size(colors)])));
    canvas.drawCircle(
        10, 50, 60,
        SkPaint(SkColor4f::FromColor(
            colors[(*frame_token_generator_ / 60 + 3) % base::size(colors)])));
    canvas.drawCircle(
        180, 180, 50,
        SkPaint(SkColor4f::FromColor(
            colors[(*frame_token_generator_ / 60 + 1) % base::size(colors)])));

    gfx::Size tile_size(200, 200);
    // 将 SkBitmap 中的数据转换为资源
    viz::ResourceId resource =
        AllocateAndFillSoftwareResource(tile_size, bitmap);

    gfx::Rect output_rect{0, 0, 200, 200};
    gfx::Transform transform;
    transform.Translate(350, 50);

    auto* quad_state = render_pass->CreateAndAppendSharedQuadState();
    quad_state->SetAll(
        transform,
        /*quad_layer_rect=*/output_rect,
        /*visible_quad_layer_rect=*/output_rect,
        /*rounded_corner_bounds=*/gfx::RRectF(),
        /*clip_rect=*/output_rect,
        /*is_clipped=*/false, /*are_contents_opaque=*/false, /*opacity=*/1.f,
        /*blend_mode=*/SkBlendMode::kSrcOver, /*sorting_context_id=*/0);

    auto* texture_quad =
        render_pass->CreateAndAppendDrawQuad<viz::TextureDrawQuad>();
    float vertex_opacity[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    // 将 resource 添加到 tile_quad 中
    texture_quad->SetNew(quad_state, output_rect, output_rect, false, resource,
                         true, gfx::PointF(0.f, 0.f), gfx::PointF(1.f, 1.f),
                         SK_ColorGRAY, vertex_opacity, false, false, false,
                         gfx::ProtectedVideoType::kClear);

    // 将 resource 对用的资源添加到 frame.resource_list
    // 中，在最简单的情况下可以直接使用 frame.resource_list.push_back(...)
    // 来添加
    client_resource_provider_->PrepareSendToParent(
        {resource}, &frame.resource_list, (viz::RasterContextProvider*)nullptr);
  }

  void AppendSurfaceDrawQuad(viz::CompositorFrame& frame,
                             viz::RenderPass* render_pass) {
    gfx::Rect output_rect{0, 0, 200, 200};
    gfx::Transform transform;
    transform.Translate(350, 350);

    auto* quad_state = render_pass->CreateAndAppendSharedQuadState();
    quad_state->SetAll(
        transform,
        /*quad_layer_rect=*/output_rect,
        /*visible_quad_layer_rect=*/output_rect,
        /*rounded_corner_bounds=*/gfx::RRectF(),
        /*clip_rect=*/output_rect,
        /*is_clipped=*/false, /*are_contents_opaque=*/false, /*opacity=*/1.f,
        /*blend_mode=*/SkBlendMode::kSrcOver, /*sorting_context_id=*/0);

    viz::SurfaceId child_surface_id(
        child_frame_sink_id_,
        local_surface_id_allocator_.GetCurrentLocalSurfaceIdAllocation()
            .local_surface_id());
    auto* surface_quad =
        render_pass->CreateAndAppendDrawQuad<viz::SurfaceDrawQuad>();
    surface_quad->SetNew(quad_state, output_rect, output_rect,
                         viz::SurfaceRange(base::nullopt, child_surface_id),
                         SK_ColorDKGRAY, true);
  }

  // 演示 TextureDrawQuad 的使用
  // VideoHoleDrawQuad 需要 viz overlay 机制支持，在Linux上还不支持，会显示为
  // SK_ColorMAGENTA 色块
  // TODO: 研究 viz overlay 机制
  void AppendVideoHoleDrawQuad(viz::CompositorFrame& frame,
                               viz::RenderPass* render_pass) {
    gfx::Rect output_rect{0, 0, 200, 200};
    gfx::Transform transform;
    transform.Translate(50, 350);

    auto* quad_state = render_pass->CreateAndAppendSharedQuadState();
    quad_state->SetAll(
        transform,
        /*quad_layer_rect=*/output_rect,
        /*visible_quad_layer_rect=*/output_rect,
        /*rounded_corner_bounds=*/gfx::RRectF(),
        /*clip_rect=*/output_rect,
        /*is_clipped=*/false, /*are_contents_opaque=*/false, /*opacity=*/1.f,
        /*blend_mode=*/SkBlendMode::kSrcOver, /*sorting_context_id=*/0);

    auto* video_hole_quad =
        render_pass->CreateAndAppendDrawQuad<viz::VideoHoleDrawQuad>();

    static const base::UnguessableToken overlay_plane_id =
        base::UnguessableToken::Create();
    video_hole_quad->SetNew(quad_state, output_rect, output_rect,
                            overlay_plane_id);
  }

  // 演示 PictureDrawQuad 的使用
  // PictureDrawQuad 当前不支持 mojo 的序列化，因此这里无法演示
  void AppendPictureDrawQuad(viz::CompositorFrame& frame,
                             viz::RenderPass* render_pass) {
    return;
    gfx::Rect output_rect = bounds_;
    output_rect.Inset(10, 10, 10, 10);

    auto display_list = base::MakeRefCounted<cc::DisplayItemList>();
    display_list->StartPaint();
    display_list->push<cc::DrawColorOp>(SK_ColorCYAN, SkBlendMode::kSrc);
    display_list->EndPaintOfUnpaired(output_rect);
    display_list->Finalize();

    viz::SharedQuadState* quad_state =
        render_pass->CreateAndAppendSharedQuadState();
    quad_state->SetAll(
        gfx::Transform(),
        /*quad_layer_rect=*/output_rect,
        /*visible_quad_layer_rect=*/output_rect,
        /*rounded_corner_bounds=*/gfx::RRectF(),
        /*clip_rect=*/gfx::Rect(),
        /*is_clipped=*/false, /*are_contents_opaque=*/false, /*opacity=*/1.f,
        /*blend_mode=*/SkBlendMode::kSrcOver, /*sorting_context_id=*/0);

    auto* picture_quad =
        render_pass->CreateAndAppendDrawQuad<viz::PictureDrawQuad>();
    picture_quad->SetNew(quad_state, output_rect, output_rect, true,
                         gfx::RectF(output_rect), output_rect.size(), false,
                         viz::RGBA_8888, output_rect, 1.f, {}, display_list);
  }

  // 演示 SolidColorDrawQuad 的使用
  void AppendSolidColorDrawQuad(viz::CompositorFrame& frame,
                                viz::RenderPass* render_pass) {
    gfx::Rect output_rect = bounds_;
    // Add a solid-color draw-quad for the big rectangle covering the entire
    // content-area of the client.
    viz::SharedQuadState* quad_state =
        render_pass->CreateAndAppendSharedQuadState();
    quad_state->SetAll(
        gfx::Transform(),
        /*quad_layer_rect=*/output_rect,
        /*visible_quad_layer_rect=*/output_rect,
        /*rounded_corner_bounds=*/gfx::RRectF(),
        /*clip_rect=*/gfx::Rect(),
        /*is_clipped=*/false, /*are_contents_opaque=*/false, /*opacity=*/1.f,
        /*blend_mode=*/SkBlendMode::kSrcOver, /*sorting_context_id=*/0);

    // 单一颜色
    viz::SolidColorDrawQuad* color_quad =
        render_pass->CreateAndAppendDrawQuad<viz::SolidColorDrawQuad>();
    color_quad->SetNew(
        quad_state, output_rect, output_rect,
        colors[(*frame_token_generator_ / 60) % base::size(colors)], false);
  }

  // 使用共享内存来传递资源到 viz
  viz::ResourceId AllocateAndFillSoftwareResource(const gfx::Size& size,
                                                  const SkBitmap& source) {
    viz::SharedBitmapId shared_bitmap_id = viz::SharedBitmap::GenerateId();
    // 创建共享内存
    base::MappedReadOnlyRegion shm =
        viz::bitmap_allocation::AllocateSharedBitmap(size, viz::RGBA_8888);
    base::WritableSharedMemoryMapping mapping = std::move(shm.mapping);

    SkImageInfo info = SkImageInfo::MakeN32Premul(size.width(), size.height());
    // 将 SkBitmap 中的像素数据拷贝到共享内存
    source.readPixels(info, mapping.memory(), info.minRowBytes(), 0, 0);

    // 将共享内存及与之对应的资源Id发送到 viz service 端
    GetCompositorFrameSinkPtr()->DidAllocateSharedBitmap(std::move(shm.region),
                                                         shared_bitmap_id);

    // 把资源存入 ClientResourceProvider 进行统一管理。
    // 后续会使用 ClientResourceProvider::PrepareSendToParent
    // 将已经存入的资源添加 到 CF 中。
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
    base::AutoLock lock(lock_);
    GetCompositorFrameSinkPtr()->SubmitCompositorFrame(
        local_surface_id_.local_surface_id(), CreateFrame(args),
        base::Optional<viz::HitTestRegionList>(),
        /*trace_time=*/0);
  }

  virtual void OnBeginFramePausedChanged(bool paused) override {}

  virtual void ReclaimResources(
      const std::vector<::viz::ReturnedResource>& resources) override {}

  viz::mojom::CompositorFrameSink* GetCompositorFrameSinkPtr() {
    if (frame_sink_associated_remote_.is_bound())
      return frame_sink_associated_remote_.get();
    return frame_sink_remote_.get();
  }

  mojo::Receiver<viz::mojom::CompositorFrameSinkClient> receiver_{this};
  mojo::AssociatedRemote<viz::mojom::CompositorFrameSink>
      frame_sink_associated_remote_;
  mojo::Remote<viz::mojom::CompositorFrameSink> frame_sink_remote_;
  viz::FrameSinkId frame_sink_id_;
  viz::LocalSurfaceIdAllocation local_surface_id_;
  gfx::Rect bounds_;
  viz::ParentLocalSurfaceIdAllocator local_surface_id_allocator_;
  viz::FrameSinkId child_frame_sink_id_;
  // 模拟每个 Client 都在独立的线程中生成 CF
  base::Thread thread_;
  viz::FrameTokenGenerator frame_token_generator_;
  base::Lock lock_;

  std::unique_ptr<viz::ClientResourceProvider> client_resource_provider_;
};

// Host 端
// 在 Chromium 中，Compositor 实现了 HostFrameSinkClient 接口，这里模拟 Chromium
// 中的命名。
class Compositor : public viz::HostFrameSinkClient {
 public:
  Compositor(gfx::AcceleratedWidget widget,
             gfx::Size size,
             mojo::PendingReceiver<viz::mojom::FrameSinkManagerClient> client,
             mojo::PendingRemote<viz::mojom::FrameSinkManager> manager)
      : widget_(widget), size_(size), compositor_thread_("CompositorThread") {
    CHECK(compositor_thread_.Start());
    compositor_thread_.task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&Compositor::InitializeOnThread, base::Unretained(this),
                       std::move(client), std::move(manager)));
  }

  void Resize(gfx::Size size) {
    // TODO:
  }

  // Called when a CompositorFrame with a new SurfaceId activates for the first
  // time.
  virtual void OnFirstSurfaceActivation(
      const viz::SurfaceInfo& surface_info) override {}

  // Called when a CompositorFrame with a new frame token is provided.
  virtual void OnFrameTokenChanged(uint32_t frame_token) override {}

 private:
  void InitializeOnThread(
      mojo::PendingReceiver<viz::mojom::FrameSinkManagerClient> client,
      mojo::PendingRemote<viz::mojom::FrameSinkManager> manager) {
    host_frame_sink_manager_.BindAndSetManager(std::move(client), nullptr,
                                               std::move(manager));
    display_client_ = std::make_unique<viz::HostDisplayClient>(widget_);

    // 创建 root client 的 FrameSinkId
    viz::FrameSinkId root_frame_sink_id =
        frame_sink_id_allocator_.NextFrameSinkId();

    // 注册 root client 的 FrameSinkId
    host_frame_sink_manager_.RegisterFrameSinkId(
        root_frame_sink_id, this, viz::ReportFirstSurfaceActivation::kNo);

    mojo::PendingAssociatedRemote<viz::mojom::CompositorFrameSink>
        frame_sink_remote;
    auto frame_sink_receiver =
        frame_sink_remote.InitWithNewEndpointAndPassReceiver();

    mojo::PendingRemote<viz::mojom::CompositorFrameSinkClient>
        root_client_remote;
    mojo::PendingReceiver<viz::mojom::CompositorFrameSinkClient>
        root_client_receiver =
            root_client_remote.InitWithNewPipeAndPassReceiver();

    auto params = viz::mojom::RootCompositorFrameSinkParams::New();
    params->widget = widget_;
    params->compositor_frame_sink = std::move(frame_sink_receiver);
    params->compositor_frame_sink_client = std::move(root_client_remote);
    params->frame_sink_id = root_frame_sink_id;
    params->disable_frame_rate_limit = false;
    params->gpu_compositing = false;
    // 只有Andorid平台能使用
    // params->refresh_rate = 1.0f;
    params->display_client = display_client_->GetBoundRemote(nullptr);
    params->display_private = display_private_.BindNewEndpointAndPassReceiver();
    // CreateRendererSettings 里面有很多和渲染相关的设置,有些对于调试非常方便
    params->renderer_settings = viz::CreateRendererSettings();
    host_frame_sink_manager_.CreateRootCompositorFrameSink(std::move(params));

    display_private_->Resize(size_);
    display_private_->SetDisplayVisible(true);

    local_surface_id_allocator_.GenerateId();
    root_client_ = std::make_unique<LayerTreeFrameSink>(
        root_frame_sink_id,
        local_surface_id_allocator_.GetCurrentLocalSurfaceIdAllocation(),
        gfx::Rect(size_));
    root_client_->Bind(std::move(root_client_receiver),
                       std::move(frame_sink_remote));
    EmbedChildClient(root_frame_sink_id);
  }

  void EmbedChildClient(viz::FrameSinkId parent_frame_sink_id) {
    // 创建 child 的 FrameSinkId
    viz::FrameSinkId frame_sink_id = frame_sink_id_allocator_.NextFrameSinkId();
    // uint64_t rand = base::RandUint64();
    // viz::FrameSinkId frame_sink_id(rand >> 32, rand & 0xffffffff);

    // 注册 root client 的 FrameSinkId
    host_frame_sink_manager_.RegisterFrameSinkId(
        frame_sink_id, this, viz::ReportFirstSurfaceActivation::kNo);
    host_frame_sink_manager_.RegisterFrameSinkHierarchy(parent_frame_sink_id,
                                                        frame_sink_id);

    mojo::PendingRemote<viz::mojom::CompositorFrameSink> frame_sink_remote;
    auto frame_sink_receiver =
        frame_sink_remote.InitWithNewPipeAndPassReceiver();

    mojo::PendingRemote<viz::mojom::CompositorFrameSinkClient> client_remote;
    mojo::PendingReceiver<viz::mojom::CompositorFrameSinkClient>
        client_receiver = client_remote.InitWithNewPipeAndPassReceiver();
    host_frame_sink_manager_.CreateCompositorFrameSink(
        frame_sink_id, std::move(frame_sink_receiver),
        std::move(client_remote));

    auto child_local_surface_id = root_client_->EmbedChild(frame_sink_id);
    child_client_ = std::make_unique<LayerTreeFrameSink>(
        frame_sink_id, child_local_surface_id, gfx::Rect(size_));
    child_client_->Bind(std::move(client_receiver),
                        std::move(frame_sink_remote));
  }

  gfx::AcceleratedWidget widget_;
  gfx::Size size_;
  viz::HostFrameSinkManager host_frame_sink_manager_;
  base::Thread compositor_thread_;
  viz::FrameSinkIdAllocator frame_sink_id_allocator_{0};
  viz::ParentLocalSurfaceIdAllocator local_surface_id_allocator_;
  std::unique_ptr<viz::HostDisplayClient> display_client_;
  mojo::AssociatedRemote<viz::mojom::DisplayPrivate> display_private_;
  std::unique_ptr<LayerTreeFrameSink> root_client_;
  std::unique_ptr<LayerTreeFrameSink> child_client_;
};

// Service 端
// 在 Chromium 中它运行于 GPU 进程,因此这里取名 GpuService
class GpuService {
 public:
  GpuService(mojo::PendingReceiver<viz::mojom::FrameSinkManager> receiver,
             mojo::PendingRemote<viz::mojom::FrameSinkManagerClient> client) {
    auto params = viz::mojom::FrameSinkManagerParams::New();
    params->restart_id = viz::BeginFrameSource::kNotRestartableId;
    params->use_activation_deadline = false;
    params->activation_deadline_in_frames = 0u;
    params->frame_sink_manager = std::move(receiver);
    params->frame_sink_manager_client = std::move(client);
    runner_ = std::make_unique<viz::VizCompositorThreadRunnerImpl>();
    runner_->CreateFrameSinkManager(std::move(params));
  }

 private:
  std::unique_ptr<viz::VizCompositorThreadRunnerImpl> runner_;
};

// DemoWindow creates the native window for the demo app. The native window
// provides a gfx::AcceleratedWidget, which is needed for the display
// compositor.
class DemoVizWindow : public ui::PlatformWindowDelegate {
 public:
  DemoVizWindow(base::OnceClosure close_closure)
      : close_closure_(std::move(close_closure)) {}
  ~DemoVizWindow() override = default;

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
    // We finally have a valid gfx::AcceleratedWidget. We can now start the
    // actual process of setting up the viz host and the service.
    // First, set up the mojo message-pipes that the host and the service will
    // use to communicate with each other.
    mojo::PendingRemote<viz::mojom::FrameSinkManager> frame_sink_manager;
    mojo::PendingReceiver<viz::mojom::FrameSinkManager>
        frame_sink_manager_receiver =
            frame_sink_manager.InitWithNewPipeAndPassReceiver();
    mojo::PendingRemote<viz::mojom::FrameSinkManagerClient>
        frame_sink_manager_client;
    mojo::PendingReceiver<viz::mojom::FrameSinkManagerClient>
        frame_sink_manager_client_receiver =
            frame_sink_manager_client.InitWithNewPipeAndPassReceiver();

    // Next, create the host and the service, and pass them the right ends of
    // the message-pipes.
    host_ = std::make_unique<Compositor>(
        widget_, platform_window_->GetBounds().size(),
        std::move(frame_sink_manager_client_receiver),
        std::move(frame_sink_manager));

    service_ =
        std::make_unique<GpuService>(std::move(frame_sink_manager_receiver),
                                     std::move(frame_sink_manager_client));
  }

  // ui::PlatformWindowDelegate:
  void OnBoundsChanged(const gfx::Rect& new_bounds) override {
    host_->Resize(new_bounds.size());
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
    platform_window_->Close();
    // service_.reset();
    // host_.reset();
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

  std::unique_ptr<Compositor> host_;
  std::unique_ptr<GpuService> service_;

  std::unique_ptr<ui::PlatformWindow> platform_window_;
  gfx::AcceleratedWidget widget_;
  base::OnceClosure close_closure_;

  DISALLOW_COPY_AND_ASSIGN(DemoVizWindow);
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
  demo::InitTrace("./trace_demo_viz_gui.json");
  demo::StartTrace("*,disabled-by-default-*");
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
  // gl::init::InitializeGLOneOff();

  // 初始化ICU(i18n),也就是icudtl.dat，views依赖ICU
  base::i18n::InitializeICU();

  ui::RegisterPathProvider();

  // This app isn't a test and shouldn't timeout.
  base::RunLoop::ScopedDisableRunTimeoutForTest disable_timeout;

  base::RunLoop run_loop;

  demo::DemoVizWindow window(run_loop.QuitClosure());
  window.Create(gfx::Rect(800, 600));

  LOG(INFO) << "running...";
  run_loop.Run();

  {
    base::RunLoop run_loop;
    demo::FlushTrace(run_loop.QuitClosure());
    run_loop.Run();
  }

  return 0;
}
