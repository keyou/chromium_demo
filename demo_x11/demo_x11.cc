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
#include "ui/base/hit_test.h"
#include "ui/base/ime/init/input_method_initializer.h"
#include "ui/base/material_design/material_design_controller.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/x/x11_util.h"
#include "ui/display/screen.h"
#include "ui/events/event.h"
#include "ui/events/event_constants.h"
#include "ui/events/platform/platform_event_source.h"

#if defined(USE_AURA)
#include "ui/aura/env.h"
#include "ui/wm/core/wm_state.h"
#endif

#if defined(USE_X11)
#include "ui/gfx/x/x11_connection.h"            // nogncheck
#include "ui/platform_window/x11/x11_window.h"  // nogncheck
#include "ui/base/x/x11_util_internal.h"
#include "ui/gfx/x/x11_atom_cache.h"
#endif


int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true, true, true, false);
  // 启动 Trace
  auto trace_config =
      base::trace_event::TraceConfig("startup" /*, "trace-to-console"*/);
  base::trace_event::TraceLog::GetInstance()->SetEnabled(
      trace_config, base::trace_event::TraceLog::RECORDING_MODE);
  // 创建主消息循环，等价于 MessagLoop
  base::SingleThreadTaskExecutor main_task_executor(base::MessagePumpType::UI);
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("DemoX11");

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

  // This app isn't a test and shouldn't timeout.
  base::RunLoop::ScopedDisableRunTimeoutForTest disable_timeout;

  base::RunLoop run_loop;

  auto xdisplay = gfx::GetXDisplay();

  // 在 Chromium 中 ui::XVisualManager 类封装了以下关于 Visual 的逻辑
  int visuals_len = 0;
  XVisualInfo visual_template;
  visual_template.screen = DefaultScreen(xdisplay);
  gfx::XScopedPtr<XVisualInfo[]> visual_list(XGetVisualInfo(
      xdisplay, VisualScreenMask, &visual_template, &visuals_len));
  
  // 支持透明的 visual
  XVisualInfo transparent_visual_info;

  for (int i = 0; i < visuals_len; ++i) {
    const XVisualInfo& info = visual_list[i];
    if (info.depth == 32 && info.visual->red_mask == 0xff0000 &&
        info.visual->green_mask == 0x00ff00 &&
        info.visual->blue_mask == 0x0000ff) {
      transparent_visual_info = info;
      DLOG(INFO) << "TransparentVID: " << transparent_visual_info.visualid;
      break;
    }
  }

  Colormap colormap = XCreateColormap(xdisplay, DefaultRootWindow(xdisplay),
                                transparent_visual_info.visual, AllocNone);

  unsigned long attribute_mask = 0;
  XSetWindowAttributes swa;
  memset(&swa, 0, sizeof(swa));

  attribute_mask |= CWBackPixel;
  swa.background_pixel = 0;

  attribute_mask |= CWColormap;
  swa.colormap = colormap;

  attribute_mask |= CWBorderPixel;
  swa.border_pixel = 0;

  // 创建一个透明窗口
  auto xwindow_ =
      XCreateWindow(xdisplay, DefaultRootWindow(xdisplay), 
                    0, 0, 400, 300,
                    0, transparent_visual_info.depth, InputOutput, transparent_visual_info.visual, 
                    attribute_mask, &swa);
  // 控制窗口边框
  // ui::SetUseOSWindowFrame(xwindow_, false);
  XMapWindow(xdisplay, xwindow_);

  LOG(INFO) << "running...";
  run_loop.Run();

  return 0;
}
