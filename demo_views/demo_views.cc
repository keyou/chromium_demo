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
#include "build/build_config.h"
#include "components/viz/host/host_frame_sink_manager.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"
#include "components/viz/service/frame_sinks/frame_sink_manager_impl.h"
#include "mojo/core/embedder/embedder.h"
#include "ui/aura/client/default_capture_client.h"
#include "ui/aura/client/window_parenting_client.h"
#include "ui/aura/env.h"
#include "ui/aura/test/test_focus_client.h"
#include "ui/aura/test/test_screen.h"
#include "ui/aura/window.h"
#include "ui/aura/window_delegate.h"
#include "ui/aura/window_tree_host.h"
#include "ui/aura/window_tree_host_observer.h"
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
#include "ui/gfx/canvas.h"
#include "ui/gfx/font_util.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/skia_util.h"
#include "ui/gl/gl_switches.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/background.h"
#include "ui/views/buildflags.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/examples/example_base.h"
#include "ui/views/examples/examples_window.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/test/desktop_test_views_delegate.h"
#include "ui/views/view.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

#if defined(USE_AURA)
#include "ui/aura/env.h"
#include "ui/wm/core/wm_state.h"
#endif

#if BUILDFLAG(ENABLE_DESKTOP_AURA)
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#endif

#if defined(USE_X11)
#include "ui/gfx/x/x11_connection.h"  // nogncheck
#endif

// Maintain the UI controls and web view for content shell
class DemoViewsWidgetDelegateView : public views::WidgetDelegateView,
                                    public views::ButtonListener {
 public:
  DemoViewsWidgetDelegateView(base::OnceClosure on_close)
      : on_close_(std::move(on_close)) {
    InitShellWindow();
  }

 private:
  void InitShellWindow() {
    DLOG(INFO) << "InitShellWindow";
    this->SetBackground(views::CreateSolidBackground(SK_ColorGRAY));
    // 设置布局管理器，为了简单这里不使用布局管理器
    // this->SetLayoutManager(std::make_unique<views::FillLayout>());

    // 添加一个使用 Material Design 的按钮
    button_ =
        views::MdTextButton::Create(this, base::ASCIIToUTF16("MaterialButton"));
    button_->SetBounds(0, 0, 100, 50);
    this->AddChildView(button_.release());
  }

  // Overridden from ButtonListener
  void ButtonPressed(views::Button* sender, const ui::Event& event) override {
    DLOG(INFO) << "ButtonPressed()";
    // 使用无障碍接口来修改按钮文本
    sender->GetViewAccessibility().AnnounceText(base::ASCIIToUTF16("Pressed"));
  }

  // Overridden from WidgetDelegateView
  bool CanResize() const override { return true; }
  bool CanMaximize() const override { return true; }
  bool CanMinimize() const override { return true; }
  base::string16 GetWindowTitle() const override {
    DLOG(INFO) << "GetWindowTitle()";
    return base::ASCIIToUTF16("Demo Widget");
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

  DISALLOW_COPY_AND_ASSIGN(DemoViewsWidgetDelegateView);
};

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true, true, true, false);
  // 启动 Trace
  auto trace_config = base::trace_event::TraceConfig("cc", "trace-to-console");
  base::trace_event::TraceLog::GetInstance()->SetEnabled(
      trace_config, base::trace_event::TraceLog::RECORDING_MODE);
  // 创建主消息循环，等价于 MessagLoop
  base::SingleThreadTaskExecutor main_task_executor(base::MessagePumpType::UI);
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("DemoViews");
  
  // 初始化mojo
  mojo::core::Init();

  // 在Linux上，x11和aura都是默认开启的
#if defined(USE_X11)
  // This demo uses InProcessContextFactory which uses X on a separate Gpu
  // thread.
  gfx::InitializeThreadedX11();
#endif

  // 加载相应平台的GL库及GL绑定
  gl::init::InitializeGLOneOff();

  // The ContextFactory must exist before any Compositors are created.
  viz::HostFrameSinkManager host_frame_sink_manager;
  viz::ServerSharedBitmapManager shared_bitmap_manager;
  viz::FrameSinkManagerImpl frame_sink_manager(&shared_bitmap_manager);
  host_frame_sink_manager.SetLocalManager(&frame_sink_manager);
  frame_sink_manager.SetLocalClient(&host_frame_sink_manager);
  // 第三个参数需要设为false才能看到界面
  // 很多chromium中的demo都没有设置，导致无法看到界面
  auto context_factory = std::make_unique<ui::InProcessContextFactory>(
      &host_frame_sink_manager, &frame_sink_manager, false);
  context_factory->set_use_test_surface(false);

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
  aura::Env::GetInstance()->set_context_factory_private(context_factory.get());
#endif

#if BUILDFLAG(ENABLE_DESKTOP_AURA)
  views::DesktopTestViewsDelegate views_delegate;
  views::InstallDesktopScreenIfNecessary();
#endif

  // 初始化输入法相关接口
  ui::InitializeInputMethodForTesting();

  // 初始化 Material Design 风格，使用MD控件时才需要初始化
  ui::MaterialDesignController::Initialize();

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

  views::Widget* window_widget_ = new views::Widget;
  views::Widget::InitParams params;
  params.bounds = gfx::Rect(0, 0, 400, 300);
  params.delegate = new DemoViewsWidgetDelegateView(run_loop.QuitClosure());
  params.wm_class_class = "demo_views";
  params.wm_class_name = params.wm_class_class;
  window_widget_->Init(std::move(params));
  window_widget_->Show();

  LOG(INFO) << "running...";
  run_loop.Run();

  ui::ShutdownInputMethodForTesting();

#if defined(USE_AURA)
  env.reset();
#endif

  return 0;
}
