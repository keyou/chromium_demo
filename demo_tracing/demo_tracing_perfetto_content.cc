#include <base/task/post_task.h>
#include <base/threading/thread.h>
#include <mojo/core/embedder/embedder.h>
#include <mojo/core/embedder/scoped_ipc_support.h>

#include "base/at_exit.h"
#include "base/callback_forward.h"
#include "base/command_line.h"
#include "base/debug/stack_trace.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/task/task_traits.h"
#include "base/test/test_timeouts.h"
#include "base/timer/timer.h"
#include "base/trace_event/common/trace_event_common.h"
#include "base/trace_event/trace_event.h"
#include "components/tracing/common/trace_startup_config.h"
#include "components/tracing/common/trace_to_console.h"
#include "components/tracing/common/tracing_switches.h"
#include "content/browser/browser_thread_impl.h"
#include "content/browser/tracing/tracing_controller_impl.h"
#include "content/common/content_export.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/tracing_controller.h"
#include "content/public/common/content_client.h"
#include "content/public/test/browser_task_environment.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "mojo/public/cpp/system/data_pipe_drainer.h"
#include "services/tracing/public/cpp/trace_startup.h"
#include "services/tracing/public/cpp/tracing_features.h"
#include "services/tracing/public/mojom/perfetto_service.mojom.h"

namespace demo {

class TracingControllerImpl : public content::TracingController,
                              public mojo::DataPipeDrainer::Client,
                              public tracing::mojom::TracingSessionClient {
 public:
};

}  // namespace demo

void TraceMe() {
  TRACE_EVENT0("test", "TraceMe");
  TRACE_EVENT1("test", "TraceMe", "value", 1);
  TRACE_EVENT2("test", "TraceMe", "value", 1, "value2", 2);
  usleep(50 * 1000);
}

void TraceCount(int times) {
  TRACE_EVENT1("test", "TraceCount", "times", times);
  // Counter 类型的 Trace 是进程级别的，因此在 TraceViewer 中它不会显示在某一个线程中
  TRACE_COUNTER1("test", "TraceCounter", times);
  usleep(10 * 1000);
}

int main(int argc, char** argv) {
  base::AtExitManager at_exit;
  base::CommandLine::Init(argc, argv);

  // Trace 的 perfetto 后端依赖 mojo
  mojo::core::Init();
  base::Thread ipc_thread("ipc!");
  ipc_thread.StartWithOptions(
      base::Thread::Options(base::MessagePumpType::IO, 0));
  // 初始化mojo的后台线程，用来异步收发消息存储到缓存
  mojo::core::ScopedIPCSupport ipc_support(
      ipc_thread.task_runner(),
      mojo::core::ScopedIPCSupport::ShutdownPolicy::CLEAN);

  // 使用单进程的Tracing
  base::FeatureList::InitializeInstance(features::kTracingServiceInProcess.name,
                                        "");

  // 在 Chromium 中 Startup 的Trace需要特殊处理，因为 TracingService 启动的比较晚
  tracing::EnableStartupTracingIfNeeded();

  TRACE_EVENT0("toplevel,startup,test", "This trace can not be record.");

  // 测试环境的超时参数初始化
  TestTimeouts::Initialize();

  // 由于 content::TracingControllerImpl 依赖 content
  // 模块，所以这里初始化Browser的测试环境
  content::BrowserTaskEnvironment task_environment_;

  tracing::InitTracingPostThreadPoolStartAndFeatureList();

  content::ContentClient content_client;
  content::ContentBrowserClient browser_client;
  content::SetContentClient(&content_client);
  content::SetBrowserClientForTesting(&browser_client);

  auto tracing_controller_ = std::make_unique<content::TracingControllerImpl>();

  // 创建配置对象
  auto trace_config = base::trace_event::TraceConfig("startup,test");

  bool result = false;

  LOG(INFO) << "StartTracing begin.";

  // 用于同步启动TracingService，只有当Tracing完全启动后 Trace 才能被记录
  base::RunLoop start_tracing_run_loop;
  // 启动 TracingService
  result = content::TracingController::GetInstance()->StartTracing(
      trace_config, base::BindOnce(
                        [](base::OnceClosure quit_closure) {
                          LOG(INFO) << "StartTracing complete.";
                          std::move(quit_closure).Run();
                        },
                        start_tracing_run_loop.QuitClosure()));
  DCHECK(result);
  start_tracing_run_loop.Run();

  // 必须等到 TracingService 完全启动后才能正确记录 Trace，
  // 否则无法记录
  TRACE_EVENT0("test", "main");

  TraceMe();

  int i = 0;
  while (i < 10)
    TraceCount(i++);

  base::RunLoop stop_tracing_run_loop;
  LOG(INFO) << "StopTracing...";

  TRACE_EVENT0("test", "StopTracing");

  // 用于将Trace的结果保存为字符串
  auto string_endpoint =
      content::TracingController::CreateStringEndpoint(base::BindOnce(
          [](base::OnceClosure quit_closure, std::unique_ptr<std::string> str) {
            LOG(ERROR) << "tracing result2=" << *str.get();
            std::move(quit_closure).Run();
          },
          stop_tracing_run_loop.QuitClosure()));

  // 用于将Trace的结果保存到文件
  base::FilePath file_path("./demo_tracing_perfetto.json");
  auto file_endpoint = content::TracingController::CreateFileEndpoint(
      file_path,
      base::BindOnce(
          [](base::OnceClosure quit_closure) { std::move(quit_closure).Run(); },
          stop_tracing_run_loop.QuitClosure()));

  // 结束Trace,获取结果，这里可以替换为string_endpoint来输出字符串
  result = content::TracingController::GetInstance()->StopTracing(
      std::move(file_endpoint));
  DCHECK(result);

  // 因为我们使用了 content::BrowserTaskEnvironment, 它是用于单元测试的，
  // 因此默认有任务调度的超时时间，我们不是单元测试，所以禁用任务调度超时机制
  base::RunLoop::ScopedDisableRunTimeoutForTest disable_timeout;
  stop_tracing_run_loop.Run();
  LOG(INFO) << "save to tracing file: " << file_path;
  return 0;
}
