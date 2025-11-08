// 父进程作为子进程间的建联
// broker，负责转发*建联*的请求和回复，此后子进程间建立 pipe 并直接通讯
// (render 和 gpu 进程通讯机制的一种抽象) 

#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/process/launch.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/threading/thread.h"
#include "demo/demo_mojo/mojom/child_proc_direct_pipe.mojom.h"
#include "mojo/core/embedder/embedder.h"
#include "mojo/core/embedder/scoped_ipc_support.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "mojo/public/cpp/platform/platform_channel.h"
#include "mojo/public/cpp/system/invitation.h"
#include "mojo/public/cpp/system/message_pipe.h"

constexpr const char kCategory[] = "category";
constexpr const char kChildProcessClient[] = "client";
constexpr const char kChildProcessService[] = "service";

#pragma region helper

class ConnectionHelper : public demo::demo_mojo::mojom::Connection {
 public:
  explicit ConnectionHelper(
      mojo::PendingRemote<demo::demo_mojo::mojom::Connection> remote)
      : remote_(std::move(remote)) {
    LOG(INFO) << "ConnectionHelper ctor";

    remote_.set_disconnect_handler(base::BindOnce(
        [] { LOG(INFO) << "Connection disconnected from service"; }));
  }

  void AddClient(
      mojo::PendingReceiver<demo::demo_mojo::mojom::Connection> receiver) {
    auto actual =
        std::make_unique<mojo::Receiver<demo::demo_mojo::mojom::Connection>>(
            this, std::move(receiver));
    int id = ++client_index_;
    actual->set_disconnect_handler(base::BindOnce(
        [](base::WeakPtr<ConnectionHelper> helper, int id) {
          LOG(INFO) << "Connection disconnected from client";
          if (!helper) {
            return;
          }
          helper->receivers_.erase(id);
        },
        weak_factory_.GetWeakPtr(), id));
    receivers_.emplace(id, std::move(actual));
  }

  ~ConnectionHelper() override { LOG(INFO) << "ConnectionHelper dtor"; }

  // 只做转发
  void EstablishChannel(EstablishChannelCallback callback) override {
    LOG(INFO) << "ConnectionHelper: EstablishChannel";
    remote_->EstablishChannel(std::move(callback));
  }

 private:
  int client_index_ = 0;
  base::flat_map<
      int,
      std::unique_ptr<mojo::Receiver<demo::demo_mojo::mojom::Connection>>>
      receivers_;
  mojo::Remote<demo::demo_mojo::mojom::Connection> remote_;
  base::WeakPtrFactory<ConnectionHelper> weak_factory_{this};
};

#pragma endregion

#pragma region service

class MyServiceImpl : public demo::demo_mojo::mojom::MyService {
 public:
  explicit MyServiceImpl(
      mojo::PendingReceiver<demo::demo_mojo::mojom::MyService> receiver,
      base::OnceClosure on_disconnect_callback)
      : receiver_(this, std::move(receiver)) {
    LOG(INFO) << "MyService ctor";
    receiver_.set_disconnect_handler(std::move(on_disconnect_callback));
  }

  ~MyServiceImpl() override { LOG(INFO) << "MyService dtor"; }

  void Hi(const std::string& message) override {
    LOG(INFO) << "Hi from client: " << message;
  }

 private:
  mojo::Receiver<demo::demo_mojo::mojom::MyService> receiver_;
};

class ConnectionFactory : public demo::demo_mojo::mojom::Connection {
 public:
  explicit ConnectionFactory(
      mojo::PendingReceiver<demo::demo_mojo::mojom::Connection> receiver)
      : receiver_(this, std::move(receiver)) {
    LOG(INFO) << "ConnectionFactory ctor";
    receiver_.set_disconnect_handler(base::BindOnce(
        [] { LOG(INFO) << "Connection from main to service has closed"; }));
  }

  ~ConnectionFactory() override { LOG(INFO) << "ConnectionFactory dtor"; }

  void EstablishChannel(EstablishChannelCallback callback) override {
    LOG(INFO) << "ConnectionFactory: EstablishChannel";
    mojo::MessagePipe pipe;
    int32_t id = ++client_id;
    service_.emplace(
        id, std::make_unique<MyServiceImpl>(
                mojo::PendingReceiver<demo::demo_mojo::mojom::MyService>(
                    std::move(pipe.handle0)),
                base::BindOnce(
                    [](base::WeakPtr<ConnectionFactory> factory, int32_t id) {
                      LOG(INFO) << "Service with client id: " << id
                                << " has disconnect, destroy it";
                      if (!factory) {
                        return;
                      }
                      factory->service_.erase(id);
                    },
                    weak_factory_.GetWeakPtr(), id)));
    LOG(INFO) << "Created pipe to establish channel with client";
    // 调用回调函数，会把参数通过 browser 返送回 client 端
    std::move(callback).Run(id, std::move(pipe.handle1));
  }

 private:
  int32_t client_id = 0;
  mojo::Receiver<demo::demo_mojo::mojom::Connection> receiver_;
  base::flat_map<int32_t, std::unique_ptr<MyServiceImpl>> service_;
  base::WeakPtrFactory<ConnectionFactory> weak_factory_{this};
};
#pragma endregion

mojo::ScopedMessagePipeHandle LaunchProcess(const char* category) {
  base::CommandLine command_line(
      base::CommandLine::ForCurrentProcess()->GetProgram());
  base::LaunchOptions options;
  mojo::PlatformChannel channel;
  LOG(INFO) << "local: "
            << channel.local_endpoint().platform_handle().GetHandle().Get()
            << " remote: "
            << channel.remote_endpoint().platform_handle().GetHandle().Get();
  mojo::OutgoingInvitation invitation;
  mojo::ScopedMessagePipeHandle pipe =
      invitation.AttachMessagePipe(kChildProcessClient);

  channel.PrepareToPassRemoteEndpoint(&options, &command_line);
  command_line.AppendSwitchASCII(kCategory, category);

  base::Process child_process = base::LaunchProcess(command_line, options);
  channel.RemoteProcessLaunchAttempted();
  mojo::OutgoingInvitation::Send(
      std::move(invitation), child_process.Handle(),
      channel.TakeLocalEndpoint(),
      base::BindRepeating(
          [](const std::string& error) { LOG(ERROR) << error; }));
  return pipe;
}

void MojoMain() {
  auto pipe = LaunchProcess(kChildProcessService);
  (new base::RepeatingTimer(
       FROM_HERE, base::Seconds(5),
       base::BindRepeating(
           [](ConnectionHelper* helper) {
             helper->AddClient(
                 mojo::PendingReceiver<demo::demo_mojo::mojom::Connection>(
                     LaunchProcess(kChildProcessClient)));
           },
           base::Unretained(new ConnectionHelper(
               mojo::PendingRemote<demo::demo_mojo::mojom::Connection>(
                   std::move(pipe), 0))))))
      ->Reset();
}

void MojoService() {
  mojo::IncomingInvitation invitation = mojo::IncomingInvitation::Accept(
      mojo::PlatformChannel::RecoverPassedEndpointFromCommandLine(
          *base::CommandLine::ForCurrentProcess()));
  auto pipe = invitation.ExtractMessagePipe(kChildProcessService);
  LOG(INFO) << "Service with pipe: " << pipe->value();
  new ConnectionFactory(
      mojo::PendingReceiver<demo::demo_mojo::mojom::Connection>(
          std::move(pipe)));
}

void MojoClient(base::RepeatingClosure quit_closure) {
  mojo::IncomingInvitation invitation = mojo::IncomingInvitation::Accept(
      mojo::PlatformChannel::RecoverPassedEndpointFromCommandLine(
          *base::CommandLine::ForCurrentProcess()));
  auto pipe = invitation.ExtractMessagePipe(kChildProcessClient);
  LOG(INFO) << "Client with pipe: " << pipe->value();

  auto* connection = new mojo::Remote<demo::demo_mojo::mojom::Connection>(
      mojo::PendingRemote<demo::demo_mojo::mojom::Connection>(std::move(pipe),
                                                              0));
  if (!connection->is_connected()) {
    LOG(ERROR) << "Connection has not been made!";
  }
  connection->set_disconnect_handler(base::BindOnce(
      [] { LOG(WARNING) << "Connection has disconnect from client to main"; }));
  connection->get()->EstablishChannel(base::BindOnce(
      [](base::RepeatingClosure quit_closure, int client_id,
         mojo::ScopedMessagePipeHandle handle) {
        LOG(INFO) << "Got channel with client_id: " << client_id
                  << ", handle: " << handle->value();
        mojo::Remote<demo::demo_mojo::mojom::MyService> service(
            mojo::PendingRemote<demo::demo_mojo::mojom::MyService>(
                std::move(handle), 0));
        // 设置一个断连处理器来追踪流程
        service.set_disconnect_handler(
            base::BindOnce([] { LOG(INFO) << "Disconnect from service"; }));

        // 调用服务
        service->Hi("World!");
        // 退出 client
        quit_closure.Run();
      },
      quit_closure));
}

int main(int argc, char** argv) {
  base::CommandLine::Init(argc, argv);
#if defined(OS_WIN)
  logging::LoggingSettings logging_setting;
  logging_setting.logging_dest = logging::LOG_TO_STDERR;
  logging::SetLogItems(true, true, false, false);
  logging::InitLogging(logging_setting);
  // 取消注释可以看到 ipcz 的连接和转发过程，验证 client 和 service 之间的直接连接并传输
  // logging::SetMinLogLevel(-10);
#endif
  LOG(INFO) << base::CommandLine::ForCurrentProcess()->GetCommandLineString();

  // 初始化 FeatureList，mojo::core::InitFeatures() 依赖它
  base::FeatureList::SetInstance(std::make_unique<base::FeatureList>());

  // 创建主线程消息循环
  base::SingleThreadTaskExecutor main_task_executor;
  base::RunLoop run_loop;

  base::Thread io_thread("io_thread");
  io_thread.StartWithOptions(
      base::Thread::Options(base::MessagePumpType::IO, 0));

  // 初始化 mojo 的 IO 线程，用来异步收发消息。
  mojo::core::ScopedIPCSupport ipc_support(
      io_thread.task_runner(),
      mojo::core::ScopedIPCSupport::ShutdownPolicy::CLEAN);

  mojo::core::InitFeatures();

  mojo::core::Configuration mojo_config;
  mojo_config.disable_ipcz = false;
  if (argc < 2) {
    // 如果启用了 ipcz，则必须将发起 invitation 的进程的 is_broker_process 设为
    // true。
    mojo_config.is_broker_process = true;
  }
  mojo::core::Init(mojo_config);

  if (argc < 2) {
    logging::SetLogPrefix("main");
    MojoMain();
  } else {
    auto category =
        base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kCategory);
    if (category == kChildProcessClient) {
      logging::SetLogPrefix(kChildProcessClient);
      MojoClient(run_loop.QuitClosure());
    } else if (category == kChildProcessService) {
      logging::SetLogPrefix(kChildProcessService);
      MojoService();
    } else {
      // Invalid category
      return -1;
    }
  }

  LOG(INFO) << "running...";
  run_loop.Run();
  LOG(INFO) << "exit";
  return 0;
}
