#include "demo/demo_shell/browser/demo_shell_browser_main_parts.h"

#include "content/public/browser/browser_context.h"
#include "ui/base/material_design/material_design_controller.h"

#if defined(OS_ANDROID)
#include "components/crash/content/browser/child_exit_observer_android.h"
#include "components/crash/content/browser/child_process_crash_observer_android.h"
#include "net/android/network_change_notifier_factory_android.h"
#include "net/base/network_change_notifier.h"
#endif


#if !defined(OS_CHROMEOS) && defined(USE_AURA) && defined(OS_LINUX)
#include "ui/base/ime/init/input_method_initializer.h"
#endif
#if defined(USE_AURA) && defined(USE_X11)
#include "ui/events/devices/x11/touch_factory_x11.h"  // nogncheck
#endif

#include "demo/demo_shell/browser/shell.h"

namespace content{

namespace{
    //  GURL GetStartupURL(){
    //      return GURL("http://www.baidu.com");
    //  }
}

DemoShellBrowserMainParts::DemoShellBrowserMainParts(const MainFunctionParams& params)
    : main_function_params_(params)
    , run_message_loop_(true) {
}

DemoShellBrowserMainParts::~DemoShellBrowserMainParts() {

}

// BrowserMainParts s.
int DemoShellBrowserMainParts::PreEarlyInitialization() {
    DLOG(INFO) << "============ PreEarlyInitialization";

#if !defined(OS_CHROMEOS) && defined(USE_AURA) && defined(OS_LINUX)
  ui::InitializeInputMethodForTesting();
#endif
#if defined(OS_ANDROID)
  net::NetworkChangeNotifier::SetFactory(
      new net::NetworkChangeNotifierFactoryAndroid());
#endif
    return BrowserMainParts::PreEarlyInitialization();
}

int DemoShellBrowserMainParts::PreCreateThreads() {
    DLOG(INFO) << "============ PreCreateThreads";
// #if defined(OS_ANDROID)
//   const base::CommandLine* command_line =
//       base::CommandLine::ForCurrentProcess();
//   crash_reporter::ChildExitObserver::Create();
//   if (command_line->HasSwitch(switches::kEnableCrashReporter)) {
//     crash_reporter::ChildExitObserver::GetInstance()->RegisterClient(
//         std::make_unique<crash_reporter::ChildProcessCrashObserver>());
//   }
// #endif
    return 0;
}

void DemoShellBrowserMainParts::PreMainMessageLoopStart() {
    DLOG(INFO) << "============ PreMainMessageLoopStart";

#if defined(USE_AURA) && defined(USE_X11)
  ui::TouchFactory::SetTouchDeviceListFromCommandLine();
#endif
}

void DemoShellBrowserMainParts::PostMainMessageLoopStart() {
    DLOG(INFO) << "============ PostMainMessageLoopStart";

}

void DemoShellBrowserMainParts::InitializeBrowserContexts(){
    DLOG(INFO) << "============ InitializeBrowserContexts";

    browser_context_.reset(new DemoShellBrowserContext(false));
}

void DemoShellBrowserMainParts::InitializeMessageLoopContext(){
    DLOG(INFO) << "============ InitializeMessageLoopContext";

    ui::MaterialDesignController::Initialize();
    DemoShell::CreateNewWindow(browser_context_.get(), GURL("http://www.baidu.com"), nullptr,
                         gfx::Size());
}


void DemoShellBrowserMainParts::PreMainMessageLoopRun() {
    InitializeBrowserContexts();
    DemoShell::Initialize();
    InitializeMessageLoopContext();
    DLOG(INFO) << "============ PreMainMessageLoopRun1";
    if (main_function_params_.ui_task) {
        DLOG(INFO) << "============ PreMainMessageLoopRun2";
        std::move(*main_function_params_.ui_task).Run();
        delete main_function_params_.ui_task;
        run_message_loop_ = false;
    }
}

bool DemoShellBrowserMainParts::MainMessageLoopRun(int* result_code) {
    DLOG(INFO) << "============ MainMessageLoopRun";
    return !run_message_loop_;
}

void DemoShellBrowserMainParts::PreDefaultMainMessageLoopRun(base::OnceClosure quit_closure) {
    DLOG(INFO) << "============ PreDefaultMainMessageLoopRun";
    DemoShell::SetMainMessageLoopQuitClosure(std::move(quit_closure));
}

void DemoShellBrowserMainParts::PostMainMessageLoopRun() {
    DLOG(INFO) << "============ PostMainMessageLoopRun";
    browser_context_.reset();
}

void DemoShellBrowserMainParts::PostDestroyThreads() {
    DLOG(INFO) << "============ PostDestroyThreads";
}


}