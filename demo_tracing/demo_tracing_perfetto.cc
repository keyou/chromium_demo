#include "base/at_exit.h"
#include "base/callback_forward.h"
#include "base/command_line.h"
#include "base/debug/stack_trace.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/process/process.h"
#include "base/task/post_task.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/test/task_environment.h"
#include "base/test/test_timeouts.h"
#include "base/threading/thread.h"
#include "base/timer/timer.h"
#include "base/trace_event/common/trace_event_common.h"
#include "base/trace_event/trace_event.h"
#include "components/tracing/common/trace_startup_config.h"
#include "components/tracing/common/trace_to_console.h"
#include "components/tracing/common/tracing_switches.h"
#include "mojo/core/embedder/embedder.h"
#include "mojo/core/embedder/scoped_ipc_support.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "mojo/public/cpp/system/data_pipe_drainer.h"
#include "services/tracing/perfetto/consumer_host.h"
#include "services/tracing/public/cpp/perfetto/perfetto_config.h"
#include "services/tracing/public/cpp/trace_startup.h"
#include "services/tracing/public/cpp/traced_process.h"
#include "services/tracing/public/cpp/tracing_features.h"
#include "services/tracing/public/mojom/perfetto_service.mojom.h"
#include "services/tracing/public/mojom/tracing_service.mojom.h"
#include "services/tracing/tracing_service.h"

namespace demo {

class TracingController : public mojo::DataPipeDrainer::Client,
                          public tracing::mojom::TracingSessionClient {
 public:
  static TracingController* GetInstance() {
    static TracingController controller;
    return &controller;
  }

  TracingController() {}

  bool StartTracing(base::trace_event::TraceConfig trace_config,
                    base::OnceClosure start_callback) {
    /********************************************************/
    // 初始化 tracing::TracingService 服务以及注册 Producer
    start_callback_ = std::move(start_callback);
    perfetto::TraceConfig perfetto_config = tracing::GetDefaultPerfettoConfig(
        trace_config, /*privacy_filtering_enabled=*/false);
    // Initialize the new service instance by pushing a pipe to each currently
    // registered client, including the browser process itself.
    std::vector<tracing::mojom::ClientInfoPtr> initial_clients;
    mojo::PendingRemote<tracing::mojom::TracedProcess> browser_remote;
    tracing::TracedProcess::ResetTracedProcessReceiver();
    tracing::TracedProcess::OnTracedProcessRequest(
        browser_remote.InitWithNewPipeAndPassReceiver());
    initial_clients.push_back(tracing::mojom::ClientInfo::New(
        base::GetCurrentProcId(), std::move(browser_remote)));
    service_.Initialize(std::move(initial_clients));

    /*******************************************************/
    // 注册 Consumer 并启动 Tracing
    // TracingSession 用来和 Consumer 通信，获取 Consumer 的结果，
    // 目前 Consumer 同时只支持一个 TracingSession，详见：
    // https://source.chromium.org/chromium/chromium/src/+/master:services/tracing/perfetto/perfetto_service.cc;l=145;drc=2f11470d7ad8963a9add116df64d2edd1b85d3a4?originalUrl=%2F
    // https://source.chromium.org/chromium/chromium/src/+/master:third_party/perfetto/src/tracing/core/tracing_service_impl.cc;l=474;drc=a088076eb26df502d265dd50498de99c05903fbd?originalUrl=%2F
    service_.BindConsumerHost(consumer_host_.BindNewPipeAndPassReceiver());
    consumer_host_->EnableTracing(
        tracing_session_host_.BindNewPipeAndPassReceiver(),
        receiver_.BindNewPipeAndPassRemote(), std::move(perfetto_config),
        tracing::mojom::TracingClientPriority::kUserInitiated);
    return true;
  }
  bool StopTracing(base::OnceClosure stop_callback) {
    tracing::TraceStartupConfig::GetInstance()->SetDisabled();

    mojo::ScopedDataPipeProducerHandle producer_handle;
    mojo::ScopedDataPipeConsumerHandle consumer_handle;
    MojoResult result =
        mojo::CreateDataPipe(nullptr, &producer_handle, &consumer_handle);
    if (result != MOJO_RESULT_OK) {
      return true;
    }

    drainer_.reset(new mojo::DataPipeDrainer(this, std::move(consumer_handle)));

    tracing_session_host_->DisableTracingAndEmitJson(
        "", std::move(producer_handle), false, std::move(stop_callback));
    return true;
  }

  // tracing::mojom::TracingSessionClient implementation:
  void OnTracingEnabled() override {
    if (start_callback_)
      std::move(start_callback_).Run();
  }
  void OnTracingDisabled() override {}

  // mojo::DataPipeDrainer::Client
  void OnDataAvailable(const void* data, size_t num_bytes) override {
    // 如果想要将 Trace 保存到文件需要在这里添加保存逻辑
    LOG(INFO) << "OnDataAvailable:\n"
              << std::string(static_cast<const char*>(data), num_bytes);
  }
  void OnDataComplete() override { LOG(INFO) << "OnDataComplete"; }

 private:
  // 这里为了简单直接创建 TracingService, 在产品中它也可以运行在独立进程
  tracing::TracingService service_;
  mojo::Remote<tracing::mojom::ConsumerHost> consumer_host_;
  mojo::Remote<tracing::mojom::TracingSessionHost> tracing_session_host_;
  mojo::Receiver<tracing::mojom::TracingSessionClient> receiver_{this};
  base::OnceClosure start_callback_;
  std::unique_ptr<mojo::DataPipeDrainer> drainer_;
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
  // Counter 类型的 Trace 是进程级别的，因此在 TraceViewer
  // 中它不会显示在某一个线程中
  TRACE_COUNTER1("test", "TraceCounter", times);
  usleep(10 * 1000);
}

int main(int argc, char** argv) {
  base::AtExitManager at_exit;
  base::CommandLine::Init(argc, argv);

  // 使用单进程的 Tracing
  base::FeatureList::InitializeInstance(features::kTracingServiceInProcess.name,
                                        "");

  base::MessageLoop message_loop;

  // 初始化线程池，会创建新的线程，在新的线程中会创建消息循环 MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("Demo");

  // Trace 的 perfetto 后端依赖 mojo
  mojo::core::Init();
  base::Thread ipc_thread("ipc!");
  ipc_thread.StartWithOptions(
      base::Thread::Options(base::MessagePumpType::IO, 0));
  // 初始化 mojo 的后台线程，用来异步收发消息存储到缓存
  mojo::core::ScopedIPCSupport ipc_support(
      ipc_thread.task_runner(),
      mojo::core::ScopedIPCSupport::ShutdownPolicy::CLEAN);

  // 在 Chromium 中 Startup 的 Trace 需要特殊处理，因为 TracingService
  // 启动的比较晚。使用这种方式启动 TraceLog 会检测程序启动参数，如果
  // 开启了 Startup, 则会创建共享内存来保存 Startup 的 Trace。否则
  // 在 TracingService 启动前的所有 Trace 都不能被记录。
  // 如果 TracingService 启动的足够早，则这里不是必须的
  tracing::EnableStartupTracingIfNeeded();

  // 如果在程序启动时没有指定 --trace-startup=... 参数，则下面这个 Trace
  // 不会被记录
  TRACE_EVENT0(
      "test",
      "This trace can not be record without '--trace-startup=...' parameter.");

  tracing::InitTracingPostThreadPoolStartAndFeatureList();

  // 对于我们自定义的 TracingController 并无实际作用
  auto tracing_controller_ = std::make_unique<demo::TracingController>();

  bool result = false;
  {
    TRACE_EVENT0("test", "StartTracing");
    LOG(INFO) << "StartTracing begin.";

    // 创建配置对象
    auto trace_config = base::trace_event::TraceConfig("startup,test");
    // 用于同步启动 TracingService，只有它完全启动后 Trace 才能被记录
    base::RunLoop start_tracing_run_loop;
    // 启动 TracingService
    result = demo::TracingController::GetInstance()->StartTracing(
        trace_config, base::BindOnce(
                          [](base::OnceClosure quit_closure) {
                            LOG(INFO) << "StartTracing complete.";
                            std::move(quit_closure).Run();
                          },
                          start_tracing_run_loop.QuitClosure()));
    DCHECK(result);
    start_tracing_run_loop.Run();
  }

  {
    // 必须等到 TracingService 完全启动后（或者开启了 Startup 的 Trace）,
    // 才能正确记录 Trace
    TRACE_EVENT0("test", "main");

    TraceMe();

    int i = 0;
    while (i < 10)
      TraceCount(i++);
  }

  {
    TRACE_EVENT0("test", "StopTracing");
    LOG(INFO) << "StopTracing begin.";

    base::RunLoop stop_tracing_run_loop;
    // 结束 Trace, 获取结果，这里可以替换为 string_endpoint 来输出字符串
    result = demo::TracingController::GetInstance()->StopTracing(
        stop_tracing_run_loop.QuitClosure());
    DCHECK(result);
    // 等待 StopTracing 返回
    stop_tracing_run_loop.Run();

    LOG(INFO) << "StopTracing complete.";
  }
  return 0;
}
