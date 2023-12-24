#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_type.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/thread.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/x/x11_util.h"
#include "ui/display/screen.h"
#include "ui/events/event.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/platform_window/platform_window_init_properties.h"

#include "demo/demo_skia/skia_canvas_gl.h"
#include "demo/demo_skia/skia_canvas_software.h"

#if defined(USE_AURA)
#include "ui/aura/env.h"
#endif

#if defined(USE_X11)
#include "ui/gfx/x/x11_connection.h"            // nogncheck
#include "ui/platform_window/x11/x11_window.h"  // nogncheck
#include "ui/base/x/x11_util_internal.h"
#include "ui/gfx/x/x11_atom_cache.h"
#include "ui/gl/gl_visual_picker_glx.h"
#include "ui/gl/gl_surface_glx.h"
// #include "/usr/include/GL/glx.h"
// #include "/usr/include/GL/glx_mangle.h"
#endif

#if defined(USE_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#endif

#if defined(OS_WIN)
#include "ui/base/cursor/cursor_loader_win.h"
#include "ui/platform_window/win/win_window.h"
#endif

namespace demo {
// DemoWindow creates the native window for the demo app. The native window
// provides a gfx::AcceleratedWidget, which is needed for the display
// compositor.
class DemoWindowHost : public ui::PlatformWindowDelegate {
 public:
  explicit DemoWindowHost(base::OnceClosure close_closure)
      : close_closure_(std::move(close_closure)) {}
  ~DemoWindowHost() override = default;

  void Create(const gfx::Rect& bounds) {
    if (base::CommandLine::ForCurrentProcess()->HasSwitch("gl")) {
      is_software_ = false;
    }

    platform_window_ = CreatePlatformWindow(bounds);
    platform_window_->Show();

    if (widget_ != gfx::kNullAcceleratedWidget)
      InitializeDemo();
  }

 private:
  std::unique_ptr<ui::PlatformWindow> CreatePlatformWindow(
      const gfx::Rect& bounds) {
    ui::PlatformWindowInitProperties props(bounds);
    props.type = ui::PlatformWindowType::kWindow;
    props.opacity = ui::PlatformWindowOpacity::kTranslucentWindow;
#if defined(USE_OZONE)
    return ui::OzonePlatform::GetInstance()->CreatePlatformWindow(
        this, std::move(props));
#elif defined(OS_WIN)
    return std::make_unique<ui::WinWindow>(this, props.bounds);
#else
    NOTIMPLEMENTED();
    return nullptr;
#endif
  }

  void InitializeDemo() {
    DCHECK_NE(widget_, gfx::kNullAcceleratedWidget);
    auto bounds = platform_window_->GetBoundsInPixels();
    if(is_software_) {
      LOG(INFO) << "Create SkiaCanvas: Software";
      skia_canvas_ = std::make_unique<demo_jni::SkiaCanvasSoftware>(
          widget_, bounds.width(), bounds.height());
    } else {
      LOG(INFO) << "Create SkiaCanvas: GLES2";
      skia_canvas_ = std::make_unique<demo_jni::SkiaCanvasGL>(
          widget_, bounds.width(), bounds.height());
    }
  }
#if defined(USE_X11)
  VisualID GetTransparentVisualId() {
    auto display = gfx::GetXDisplay();

    XVisualInfo visualinfo;
    if(XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &visualinfo)) {
      DLOG(INFO) << "TransparentVID: " << visualinfo.visualid;
      return visualinfo.visualid;
    }

    // 或者使用下面这种方法
    int visuals_len = 0;
    XVisualInfo visual_template;
    visual_template.screen = DefaultScreen(display);
    gfx::XScopedPtr<XVisualInfo[]> visual_list(XGetVisualInfo(
        display, VisualScreenMask, &visual_template, &visuals_len));

    for (int i = 0; i < visuals_len; ++i) {
      const XVisualInfo& info = visual_list[i];
      if (info.depth == 32 && info.visual->red_mask == 0xff0000 &&
          info.visual->green_mask == 0x00ff00 &&
          info.visual->blue_mask == 0x0000ff) {
        DLOG(INFO) << "TransparentVID: " << info.visualid;
        return info.visualid;
      }
    }

    return 0;
  }
#endif

  // ui::PlatformWindowDelegate:
  void OnBoundsChanged(
      const ui::PlatformWindowDelegate::BoundsChange& bounds) override {
    if (skia_canvas_) {
      auto new_bounds = platform_window_->GetBoundsInPixels().size();
      skia_canvas_->Resize(new_bounds.width(), new_bounds.height());
      #if defined(USE_X11)
      ((ui::X11Window*)platform_window_.get())->NotifySwapAfterResize();
      #endif
    }
  }

  void OnAcceleratedWidgetAvailable(gfx::AcceleratedWidget widget) override {
    widget_ = widget;
    // 如果需要去除窗口边框，将true改为false
    // ui::SetUseOSWindowFrame(widget_, true);
    if (platform_window_)
      InitializeDemo();
  }
  void OnWillDestroyAcceleratedWidget() override {}

  void OnDamageRect(const gfx::Rect& damaged_region) override {}
  void DispatchEvent(ui::Event* event) override {
    if ((event->IsMouseEvent() && event->AsMouseEvent()->IsLeftMouseButton()) ||
        event->IsTouchEvent()) {
      int action = -1;
      if (event->type() == ui::ET_MOUSE_PRESSED ||
          event->type() == ui::ET_TOUCH_PRESSED)
        action = 0;
      else if (event->type() == ui::ET_MOUSE_RELEASED ||
               event->type() == ui::ET_TOUCH_RELEASED)
        action = 1;
      else if (event->type() == ui::ET_MOUSE_DRAGGED ||
               event->type() == ui::ET_TOUCH_MOVED)
        action = 2;
      else
        return;
      auto* located_event = event->AsLocatedEvent();
      auto location = located_event->location();
      if (action != 2)
        DLOG(INFO) << "action,x,y= " << action << "," << location.x() << ","
                   << location.y();
      skia_canvas_->OnTouch(action, location.x(), location.y());
    }
  }
  void OnCloseRequest() override {
    // TODO: Use a more robust exit method

    platform_window_->Close();
  }
  void OnClosed() override {
    if (close_closure_)
      std::move(close_closure_).Run();
  }
  void OnWindowStateChanged(ui::PlatformWindowState old_state,
                            ui::PlatformWindowState new_state) override {}
  void OnLostCapture() override {}
  void OnAcceleratedWidgetDestroyed() override {}
  void OnActivationChanged(bool active) override {}
  void OnMouseEnter() override {}

  std::unique_ptr<ui::PlatformWindow> platform_window_;
  gfx::AcceleratedWidget widget_;
  base::OnceClosure close_closure_;
  std::unique_ptr<demo_jni::SkiaCanvas> skia_canvas_;
  bool is_software_ = true;
};

}  // namespace demo

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true, true, true, false);

  // 创建主消息循环，等价于 MessagLoop
  base::SingleThreadTaskExecutor main_task_executor(base::MessagePumpType::UI);
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("DemoSkia");

  {
    // Make Ozone run in single-process mode.
    ui::OzonePlatform::InitParams params;
    params.single_process = true;
    ui::OzonePlatform::InitializeForUI(params);
    ui::OzonePlatform::InitializeForGPU(params);
  }

  auto event_source_ = ui::PlatformEventSource::CreateDefault();

  // 初始化ICU(i18n),也就是icudtl.dat，views依赖ICU
  base::i18n::InitializeICU();

  ui::RegisterPathProvider();

  // This app isn't a test and shouldn't timeout.
  // base::RunLoop::ScopedDisableRunTimeoutForTest disable_timeout;

  base::RunLoop run_loop;

  demo::DemoWindowHost window(run_loop.QuitClosure());
  window.Create(gfx::Rect(800, 600));

  LOG(INFO) << "running...";
  run_loop.Run();

  return 0;
}
