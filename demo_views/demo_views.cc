/**
 * NOTE: 更多关于 views 的 demo 建议参考官方 ui/views/examples.
 */

#include <memory>
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_type.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "build/build_config.h"
#include "components/viz/host/host_frame_sink_manager.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"
#include "components/viz/service/frame_sinks/frame_sink_manager_impl.h"
#include "mojo/core/embedder/embedder.h"
#include "ui/aura/client/window_parenting_client.h"
#include "ui/aura/env.h"
#include "ui/aura/test/test_screen.h"
#include "ui/aura/window.h"
#include "ui/aura/window_delegate.h"
#include "ui/aura/window_tree_host.h"
#include "ui/aura/window_tree_host_observer.h"
#include "ui/base/ime/init/input_method_initializer.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/compositor/paint_recorder.h"
#include "ui/compositor/test/in_process_context_factory.h"
#include "ui/display/screen.h"
#include "ui/events/event.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font_util.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gl/gpu_preference.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/background.h"
#include "ui/views/buildflags.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/examples/example_base.h"
#include "ui/views/examples/examples_window.h"
#include "ui/views/layout/layout_manager_base.h"
#include "ui/views/layout/layout_provider.h"
#include "ui/views/test/desktop_test_views_delegate.h"
#include "ui/views/view.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

#if defined(USE_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#endif

#if defined(USE_AURA)
#include "ui/aura/env.h"
#include "ui/wm/core/wm_state.h"
#endif

#if BUILDFLAG(ENABLE_DESKTOP_AURA)
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#endif

#if defined(USE_X11)
#include "ui/base/x/x11_util.h"
#include "ui/gfx/x/connection.h"  // nogncheck
#endif

// Maintain the UI controls and web view for content shell
class DemoViewsWidgetDelegateView : public views::WidgetDelegateView {
 public:
  explicit DemoViewsWidgetDelegateView(base::OnceClosure on_close)
      : on_close_(std::move(on_close)),
        layout_provider_(std::make_unique<views::LayoutProvider>()) {
    InitShellWindow();
  }

 private:
  void InitShellWindow() {
    DLOG(INFO) << "InitShellWindow";
    SetBackground(views::CreateSolidBackground(SK_ColorGRAY));
    // 设置布局管理器，为了简单这里不使用布局管理器
    // this->SetLayoutManager(std::make_unique<views::FillLayout>());
    // 添加一个使用 Material Design 的按钮
    button_ = std::make_unique<views::MdTextButton>(
        base::BindRepeating(&DemoViewsWidgetDelegateView::ButtonPressed,
                            base::Unretained(this)),
        u"MaterialButton");
    button_->SetBounds(0, 0, 100, 50);
    this->AddChildView(button_.release());
  }

  void ButtonPressed(const ui::Event& event) {
    DLOG(INFO) << "ButtonPressed()";
  }

  // Overridden from WidgetDelegateView
  bool CanMaximize() const override { return true; }
  bool CanMinimize() const override { return true; }
  std::u16string GetWindowTitle() const override {
    DLOG(INFO) << "GetWindowTitle()";
    return u"Demo Widget";
  }
  void WindowClosing() override {
    if (on_close_)
      std::move(on_close_).Run();
  }

  void OnPaint(gfx::Canvas* canvas) override {
    DLOG(INFO) << "OnPaint()";
    // 在views::OnPaint()中绘制背景色，它会覆盖这里的蓝色
    canvas->FillRect(gfx::Rect(0, 0, 300, 50), SK_ColorBLUE);
    views::WidgetDelegateView::OnPaint(canvas);
    canvas->FillRect(gfx::Rect(0, 0, 150, 100), SK_ColorRED);
  }

  std::unique_ptr<views::Button> button_;
  base::OnceClosure on_close_;
  std::unique_ptr<views::LayoutProvider> layout_provider_;
};

class SolidBackground : public views::Background {
 public:
  explicit SolidBackground(SkColor color) { SetNativeControlColor(color); }

  void Paint(gfx::Canvas* canvas, views::View* view) const override {
    // Fill the background. Note that we don't constrain to the bounds as
    // canvas is already clipped for us.
    canvas->DrawColor(get_color());
  }
};

class InkView : public views::View {
 public:
  InkView() {
    path_.moveTo(0, 0);
    paint_flags_.setColor(SK_ColorWHITE);
    paint_flags_.setStyle(cc::PaintFlags::Style::kStroke_Style);
    paint_flags_.setStrokeWidth(5);
  }
  void OnMouseMoved(const ui::MouseEvent& event) override {
    DLOG(INFO) << "OnMouseMoved()";
    path_.lineTo(event.x(), event.y());
    SchedulePaint();
  }
  void OnPaint(gfx::Canvas* canvas) override {
    DLOG(INFO) << "OnPaint()";
    // 在views::OnPaint()中绘制背景色，它会覆盖这里的蓝色
    // canvas->DrawColor(SK_ColorGREEN);
    views::View::OnPaint(canvas);
    canvas->DrawPath(path_, paint_flags_);
  }

  SkPath path_;
  cc::PaintFlags paint_flags_;
};

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true, true, true, false);

#if defined(OS_WIN)
  logging::LoggingSettings logging_setting;
  logging_setting.logging_dest = logging::LOG_TO_STDERR;
  logging::SetLogItems(true, true, false, false);
  logging::InitLogging(logging_setting);
#endif

  // 启动 Trace
  // auto trace_config = base::trace_event::TraceConfig("cc"/*,
  // "trace-to-console"*/);
  // base::trace_event::TraceLog::GetInstance()->SetEnabled(
  //     trace_config, base::trace_event::TraceLog::RECORDING_MODE);
  // 创建主消息循环，等价于 MessagLoop
  base::SingleThreadTaskExecutor main_task_executor(base::MessagePumpType::UI);
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("DemoViews");

  // 初始化mojo
  mojo::core::Init();

  {
    // Make Ozone run in single-process mode.
    ui::OzonePlatform::InitParams params;
    params.single_process = true;
    ui::OzonePlatform::InitializeForGPU(params);
  }
  // 加载相应平台的GL库及GL绑定
  gl::init::InitializeGLOneOff(gl::GpuPreference::kDefault);
  LOG(INFO) << "GetGLImplementation: " << gl::GetGLImplementation();

  // The ContextFactory must exist before any Compositors are created.
  viz::HostFrameSinkManager host_frame_sink_manager;
  viz::ServerSharedBitmapManager shared_bitmap_manager;
  viz::FrameSinkManagerImpl frame_sink_manager(
      (viz::FrameSinkManagerImpl::InitParams(&shared_bitmap_manager)));
  host_frame_sink_manager.SetLocalManager(&frame_sink_manager);
  frame_sink_manager.SetLocalClient(&host_frame_sink_manager);
  // 第三个参数需要设为 true 才能看到界面
  auto context_factory = std::make_unique<ui::InProcessContextFactory>(
      &host_frame_sink_manager, &frame_sink_manager, true);
  // context_factory->set_use_test_surface(false);

  // 初始化ICU(i18n),也就是icudtl.dat，views依赖ICU
  base::i18n::InitializeICU();

  // 初始化资源(l10n)，views依赖这些资源
  ui::RegisterPathProvider();
  base::FilePath pak_path;
  CHECK(base::PathService::Get(base::DIR_ASSETS, &pak_path));
  pak_path = pak_path.AppendASCII("demo_views.pak");
  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_path);

  // 初始化font，非必须
  gfx::InitializeFonts();

#if defined(USE_AURA)
  wm::WMState wm_state;
  std::unique_ptr<aura::Env> env = aura::Env::CreateInstance();
  aura::Env::GetInstance()->set_context_factory(context_factory.get());
#endif

#if BUILDFLAG(ENABLE_DESKTOP_AURA)
  views::DesktopTestViewsDelegate views_delegate;
  std::unique_ptr<display::Screen> desktop_screen =
      views::CreateDesktopScreen();
#endif
  // 初始化输入法相关接口
  ui::InitializeInputMethodForTesting();

  base::RunLoop run_loop;

  views::Widget* window_widget_ = new views::Widget;
  views::Widget::InitParams params;
  params.bounds = gfx::Rect(0, 0, 400, 300);
  params.delegate = new DemoViewsWidgetDelegateView(run_loop.QuitClosure());
  params.wm_class_class = "demo_views";
  params.wm_class_name = params.wm_class_class;
  window_widget_->Init(std::move(params));
  window_widget_->Show();

  views::Widget::InitParams child_params(
      views::Widget::InitParams::TYPE_CONTROL);
  child_params.parent = window_widget_->GetNativeView();
  child_params.ownership = views::Widget::InitParams::WIDGET_OWNS_NATIVE_WIDGET;
  child_params.opacity = views::Widget::InitParams::WindowOpacity::kTranslucent;
  auto view = std::make_unique<InkView>();
  view->SetBackground(std::make_unique<SolidBackground>(SK_ColorTRANSPARENT));
  views::Widget* child = new views::Widget;
  child->Init(std::move(child_params));
  child->SetBounds(gfx::Rect(50, 20, 120, 120));
  child->SetContentsView(view.get());
  child->Show();

  DLOG(INFO)
      << "MainWidget: "
      << window_widget_->GetNativeView()->GetHost()->GetAcceleratedWidget();
  DLOG(INFO) << "ChildWidget: "
             << child->GetNativeView()->GetHost()->GetAcceleratedWidget();

  LOG(INFO) << "running...";
  run_loop.Run();

  ui::ShutdownInputMethodForTesting();

#if defined(USE_AURA)
  env.reset();
#endif

  return 0;
}
