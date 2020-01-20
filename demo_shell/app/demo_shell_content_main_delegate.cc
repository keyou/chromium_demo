#include "demo/demo_shell/app/demo_shell_content_main_delegate.h"

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/cpu.h"
#include "content/public/common/content_switches.h"
#include "base/path_service.h"
#include "base/base_paths.h"
#include "ui/base/resource/resource_bundle.h"
#include "components/crash/core/common/crash_key.h"
#include "base/trace_event/trace_log.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/utility/content_utility_client.h"
#include "content/public/gpu/content_gpu_client.h"
#include "content/common/content_constants_internal.h"

#include "demo/demo_shell/common/demo_shell_content_client.h"
#include "demo/demo_shell/browser/demo_shell_content_browser_client.h"
#include "demo/demo_shell/renderer/demo_shell_content_renderer_client.h"

#if defined(OS_POSIX) && !defined(OS_MACOSX)
#include "components/crash/content/app/breakpad_linux.h"
#include "v8/include/v8.h"
#endif

#if defined(OS_ANDROID)
#include "base/android/apk_assets.h"
#include "base/posix/global_descriptors.h"
#include "content/public/browser/android/compositor.h"
#include "content/public/test/nested_message_pump_android.h"
#include "content/shell/android/shell_descriptors.h"
#endif

namespace {

//初始化日志库
void InitLogging(const base::CommandLine& command_line) {
  base::FilePath log_filename =
      command_line.GetSwitchValuePath(switches::kLogFile);
  if (log_filename.empty()) {
    base::PathService::Get(base::DIR_EXE, &log_filename);
    log_filename = log_filename.AppendASCII("demo_shell.log");
  }

  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_ALL;
  settings.log_file_path = log_filename.value().c_str();
  settings.delete_old = logging::DELETE_OLD_LOG_FILE;
  logging::InitLogging(settings);
  logging::SetLogItems(true /* Process ID */, true /* Thread ID */,
                       true /* Timestamp */, false /* Tick count */);
  // Disable VLOG and enable all LOG
  logging::SetMinLogLevel(0);
}
}

namespace content{

DemoShellContentMainDelegate::DemoShellContentMainDelegate(){
}

DemoShellContentMainDelegate::~DemoShellContentMainDelegate(){
}

//基本的初始化工作完成，此时可以安全创建某些单例和检查命令行工作
bool DemoShellContentMainDelegate::BasicStartupComplete(int* exit_code) {
    base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
    int dummy;
    if (!exit_code)
        exit_code = &dummy;
#if defined(OS_ANDROID)
    Compositor::Initialize();
#endif
    InitLogging(command_line);

    //创建一个DemoShellContentClient
    content_client_.reset(new DemoShellContentClient);
    SetContentClient(content_client_.get());
    DLOG(INFO) << "=====BasicStartupComplete";
    return false;
}

//Sandbox启动前的逻辑，主要用于初始化User Data目录和PDF模块
void DemoShellContentMainDelegate::PreSandboxStartup() {
    DLOG(INFO) << "=====PreSandboxStartup";
    crash_reporter::InitializeCrashKeys();
    InitializeResourceBundle();
}

#if defined(OS_LINUX)
void DemoShellContentMainDelegate::ZygoteForked() {
    if (base::CommandLine::ForCurrentProcess()->HasSwitch(
            switches::kEnableCrashReporter)) {
        std::string process_type =
            base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
                switches::kProcessType);
        breakpad::InitCrashReporter(process_type);
    }
    DLOG(INFO) << "=====ZygoteForked";
}
#endif

void DemoShellContentMainDelegate::InitializeResourceBundle(){

#if defined(OS_ANDROID)
  // On Android, the renderer runs with a different UID and can never access
  // the file system. Use the file descriptor passed in at launch time.
  auto* global_descriptors = base::GlobalDescriptors::GetInstance();
  int pak_fd = global_descriptors->MaybeGet(kShellPakDescriptor);
  base::MemoryMappedFile::Region pak_region;
  if (pak_fd >= 0) {
    pak_region = global_descriptors->GetRegion(kShellPakDescriptor);
  } else {
    pak_fd =
        base::android::OpenApkAsset("assets/demo_shell.pak", &pak_region);
    // Loaded from disk for browsertests.
    if (pak_fd < 0) {
      base::FilePath pak_file;
      bool r = base::PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_file);
      DCHECK(r);
      pak_file = pak_file.Append(FILE_PATH_LITERAL("paks"));
      pak_file = pak_file.Append(FILE_PATH_LITERAL("demo_shell.pak"));
      int flags = base::File::FLAG_OPEN | base::File::FLAG_READ;
      pak_fd = base::File(pak_file, flags).TakePlatformFile();
      pak_region = base::MemoryMappedFile::Region::kWholeFile;
    }
    global_descriptors->Set(kShellPakDescriptor, pak_fd, pak_region);
  }
  DCHECK_GE(pak_fd, 0);
  // This is clearly wrong. See crbug.com/330930
  ui::ResourceBundle::InitSharedInstanceWithPakFileRegion(base::File(pak_fd),
                                                          pak_region);
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromFileRegion(
      base::File(pak_fd), pak_region, ui::SCALE_FACTOR_100P);
  DLOG(INFO) << "====InitializeResourceBundle ANDROID";
    
#else
  base::FilePath pak_file;
  bool r = base::PathService::Get(base::DIR_ASSETS, &pak_file);
  DCHECK(r);
  pak_file = pak_file.Append(FILE_PATH_LITERAL("demo_shell.pak"));
  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
  DLOG(INFO) << "====InitializeResourceBundle" << pak_file;

#endif
}

// 用于请求embedder启动一个进程。默认返回-1
int DemoShellContentMainDelegate::RunProcess(
    const std::string& process_type,
    const MainFunctionParams& main_function_params) {
    DLOG(INFO) << "=====RunProcess " << process_type;
    base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
    DLOG(INFO) << "======" << command_line.GetArgumentsString();

#if defined(OS_ANDROID)
    if (!process_type.empty())
        return -1;
    std::unique_ptr<BrowserMainRunner> main_runner = BrowserMainRunner::Create();
    // In browser tests, the |main_function_params| contains a |ui_task| which
    // will execute the testing. The task will be executed synchronously inside
    // Initialize() so we don't depend on the BrowserMainRunner being Run().
    int initialize_exit_code = main_runner->Initialize(main_function_params);
    DCHECK_LT(initialize_exit_code, 0)
        << "BrowserMainRunner::Initialize failed in ShellMainDelegate";
    ignore_result(main_runner.release());
    DLOG(INFO) << "=====RunProcess android browser runner" << process_type;

    return 0;
#else
    return -1;
#endif
}

ContentBrowserClient* DemoShellContentMainDelegate::CreateContentBrowserClient(){
    DLOG(INFO) << "=====CreateContentBrowserClient ";
    browser_client_.reset(new DemoShellContentBrowserClient);
    return browser_client_.get();
}

ContentGpuClient* DemoShellContentMainDelegate::CreateContentGpuClient(){
    DLOG(INFO) << "=====CreateContentGpuClient ";
    gpu_client_.reset(new ContentGpuClient());
    return gpu_client_.get();
}

ContentRendererClient* DemoShellContentMainDelegate::CreateContentRendererClient(){
    DLOG(INFO) << "=====CreateContentRendererClient ";
    renderer_client_.reset(new DemoShellContentRendererClient());
    return renderer_client_.get();
}

ContentUtilityClient* DemoShellContentMainDelegate::CreateContentUtilityClient(){
    DLOG(INFO) << "=====CreateContentUtilityClient ";
    utility_client_.reset(new ContentUtilityClient());
    return utility_client_.get();
}



} //namespace content
