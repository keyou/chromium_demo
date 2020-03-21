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
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/power_monitor/power_monitor.h"
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
#include "ui/aura/window_delegate.h"
#include "ui/aura/window_tree_host_observer.h"
#include "ui/aura/window_tree_host.h"
#include "ui/aura/window.h"
#include "ui/base/hit_test.h"
#include "ui/base/ime/init/input_method_initializer.h"
#include "ui/base/material_design/material_design_controller.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
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
#include "ui/views/background.h"
#include "ui/views/buildflags.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/examples/example_base.h"
#include "ui/views/examples/examples_window.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/test/desktop_test_views_delegate.h"
#include "ui/views/view.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget.h"

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
class DemoViewsWidgetDelegateView : public views::WidgetDelegateView {
 public:
  DemoViewsWidgetDelegateView() { InitShellWindow(); }

 private:
  void InitShellWindow() {
    DLOG(INFO) << "InitShellWindow";
    this->SetBackground(views::CreateSolidBackground(SK_ColorGRAY));
    // this->SetLayoutManager(std::make_unique<views::FillLayout>());
    auto label_button = std::make_unique<views::LabelButton>(
        nullptr, base::ASCIIToUTF16("LabelButton"));
    label_button->SetFocusForPlatform();
    label_button->set_request_focus_on_press(true);
    label_button->SetBounds(0, 0, 100, 50);
    label_button->SetTextColor(views::Button::ButtonState::STATE_NORMAL,
                               SK_ColorRED);
    label_button->SetTextColor(views::Button::ButtonState::STATE_HOVERED,
                               SK_ColorGREEN);
    label_button->SetBackground(views::CreateSolidBackground(SK_ColorYELLOW));
    this->AddChildView(label_button.release());
  }

  // Overridden from WidgetDelegateView
  bool CanResize() const override { return true; }
  bool CanMaximize() const override { return true; }
  bool CanMinimize() const override { return true; }
  base::string16 GetWindowTitle() const override {
    DLOG(INFO) << "GetWindowTitle()";
    return base::ASCIIToUTF16("Demo Widget");
  }
  void WindowClosing() override {}

  void OnPaint(gfx::Canvas* canvas) override {
    DLOG(INFO) << "OnPaint()";
    canvas->FillRect(gfx::Rect(0, 0, 250, 50), SK_ColorBLUE);
    views::WidgetDelegateView::OnPaint(canvas);
    canvas->FillRect(gfx::Rect(0, 0, 150, 100), SK_ColorRED);
  }

  DISALLOW_COPY_AND_ASSIGN(DemoViewsWidgetDelegateView);
};

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true, false, true, false);
  // 创建主消息循环，等价于 MessagLoop
  base::SingleThreadTaskExecutor main_task_executor(base::MessagePumpType::UI);
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("DemoWidget");

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

  ui::InitializeInputMethodForTesting();
  ui::MaterialDesignController::Initialize();

  // This app isn't a test and shouldn't timeout.
  base::RunLoop::ScopedDisableRunTimeoutForTest disable_timeout;

  views::Widget* window_widget_ = new views::Widget;
  views::Widget::InitParams params;
  params.bounds = gfx::Rect(0, 0, 400, 300);
  params.delegate = new DemoViewsWidgetDelegateView();
  params.wm_class_class = "demo_views";
  params.wm_class_name = params.wm_class_class;
  window_widget_->Init(std::move(params));
  window_widget_->Show();

  LOG(INFO) << "running...";
  base::RunLoop().Run();

  return 0;
}
