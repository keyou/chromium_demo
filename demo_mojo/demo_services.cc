/**
 * DEPRECATED: 这个 demo 已经过时！
 *
 * 升级改动
 * -------
 * 80->91:
 *  - 在 chromium 的中移除对 ServiceManager 相关类的依赖，让 Browser 直接管理
 * Services,详见 https://bugs.chromium.org/p/chromium/issues/detail?id=977637
 *  - 使用新的 Mojo API（新API名称）替换旧的 Mojo API，详见
 *    https://bugs.chromium.org/p/chromium/issues/detail?id=955171&q=955171&can=1
 *  - 使用轻量级的 mojo::ServiceFactory 进行服务的注册和运行，不再使用
 * ServiceManager；
 *  - 不再区分 Service 接口和普通的 mojo 接口，一个 Service 接口就是一个普通的
 * mojo 接口；
 *
 * NOTE: Chromium 产品中已经不再使用 ServiceManager，也不再这样使用
 * Service，因此这个 demo 已经不再重要。在新版中在学习完 mojo bindings
 * 接口之后，只需要再看一下 mojo::ServiceFactory 类即可，不需要再学习这个 demo,
 * 虽然它目前依然可以正常运行。
 */

#include "base/at_exit.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/task/post_task.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"

#include <mojo/core/embedder/embedder.h>
#include <mojo/core/embedder/scoped_ipc_support.h>
#include "mojo/public/c/system/buffer.h"
#include "mojo/public/c/system/data_pipe.h"
#include "mojo/public/c/system/message_pipe.h"
#include "mojo/public/cpp/bindings/interface_ptr.h"
#include "mojo/public/cpp/system/buffer.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "mojo/public/cpp/system/message_pipe.h"
#include "mojo/public/cpp/system/simple_watcher.h"
#include "mojo/public/cpp/system/wait.h"

// For services
#include "sandbox/policy/sandbox_type.h"
#include "sandbox/policy/switches.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "services/service_manager/public/cpp/identity.h"
#include "services/service_manager/public/cpp/manifest.h"
#include "services/service_manager/public/cpp/manifest_builder.h"
#include "services/service_manager/public/cpp/service.h"
#include "services/service_manager/public/cpp/service_executable/service_executable_environment.h"
#include "services/service_manager/public/cpp/service_executable/switches.h"
#include "services/service_manager/service_manager.h"
#include "services/service_manager/service_process_host.h"
#include "services/service_manager/service_process_launcher.h"
#include "services/service_manager/service_process_launcher_delegate.h"

#include "demo/demo_mojo/mojom/test_service.mojom.h"

using namespace demo::demo_mojo::mojom;
using namespace mojo;

class TestInterfaceImpl : public demo::demo_mojo::mojom::TestInterface {
 public:
  TestInterfaceImpl(PendingReceiver<TestInterface> receiver)
      : receiver_(this, std::move(receiver)) {}

  void Hello(const std::string& who) override {
    LOG(INFO) << "TestInterfaceImpl run: Hello " << who;
  }

 private:
  // 也可以用StrongBinding
  // 要想让当前接口的一个实例可以服务多个Remote,这里可以使用ReceiverSet
  Receiver<demo::demo_mojo::mojom::TestInterface> receiver_;
};
// 这里故意把Service的实现和接口的实现分开,因为很多情况下一个Service会提供多个接口
class TestService : public service_manager::Service {
 public:
  TestService(service_manager::mojom::ServiceRequest request) {
    service_receiver_ = std::make_unique<service_manager::ServiceReceiver>(
        this, std::move(request));
  }
  TestService() {
    service_receiver_ =
        std::make_unique<service_manager::ServiceReceiver>(this);
  }
  void OnStart() override {
    LOG(INFO) << "TestService Start.";
    // 演示从一个服务内请求其它服务提供的接口
    Remote<RootInterface> root;
    service_receiver_->GetConnector()->BindInterface(
        "consumer_service", root.BindNewPipeAndPassReceiver());
    root->Hi("TestService");
  }
  // 当其他服务调用connector->Connect()时会触发这里
  void OnBindInterface(const service_manager::BindSourceInfo& source,
                       const std::string& interface_name,
                       mojo::ScopedMessagePipeHandle interface_pipe) override {
    LOG(INFO) << "OnBindInterface: " << interface_name;
    // 如果当前服务提供多个接口,可以使用BinderRegistry消除这里大量的if/else
    if (interface_name == TestInterface::Name_) {
      // 这样写有个bug,只有最后一个发起服务请求的Remote有效,因为前面的pipe被销毁了
      // 要想保证一个TestInterfaceImpl实例可以服务多个Remote,可以在TestInterface的实现中使用ReceiverSet
      test_interface_.reset(new TestInterfaceImpl(
          PendingReceiver<TestInterface>(std::move(interface_pipe))));
    }
  }

 private:
  std::unique_ptr<TestInterfaceImpl> test_interface_;
  // 这里为了demo简单,不使用BinderRegistry
  // service_manager::BinderRegistry registry_;
  std::unique_ptr<service_manager::ServiceReceiver> service_receiver_;
};

const service_manager::Manifest& GetTestManifest() {
  static base::NoDestructor<service_manager::Manifest> manifest{
      service_manager::ManifestBuilder()
          .WithServiceName("test_service")
          .WithOptions(
              service_manager::ManifestOptionsBuilder()
                  .WithExecutionMode(service_manager::Manifest::ExecutionMode::
                                         kOutOfProcessBuiltin)
                  .WithSandboxType("none")
                  .Build())
          .ExposeCapability("test", service_manager::Manifest::InterfaceList<
                                        demo::demo_mojo::mojom::TestInterface>())
          .RequireCapability("consumer_service", "root")
          .Build()};
  return *manifest;
}

class RootInterfaceImpl : public demo::demo_mojo::mojom::RootInterface {
 public:
  RootInterfaceImpl(PendingReceiver<RootInterface> receiver)
      : receiver_(this, std::move(receiver)) {}
  void Hi(const std::string& who) override {
    LOG(INFO) << "RootInterfaceImpl run: Hi " << who;
  }

 private:
  Receiver<RootInterface> receiver_;
};

class ConsumerService : public service_manager::Service {
 public:
  ConsumerService() : service_receiver_(this) {}
  ConsumerService(service_manager::mojom::ServiceRequest request)
      : service_receiver_(this, std::move(request)) {}

  void OnStart() override {
    LOG(INFO) << "ConsumerService Start.";
    // 演示从一个服务内请求其它服务提供的接口
    Remote<TestInterface> test;
    service_receiver_.GetConnector()->BindInterface(
        "test_service", test.BindNewPipeAndPassReceiver());
    test->Hello("ConsumerService");
  }

  void OnBindInterface(const service_manager::BindSourceInfo& source,
                       const std::string& interface_name,
                       mojo::ScopedMessagePipeHandle interface_pipe) override {
    LOG(INFO) << "OnBindInterface: " << interface_name;
    // 如果当前服务提供多个接口,可以使用BinderRegistry消除这里大量的if/else
    if (interface_name == RootInterface::Name_) {
      // 这样写有个bug,只有最后一个发起服务请求的Remote有效,因为前面的pipe被销毁了
      // 要想保证一个TestInterfaceImpl实例可以服务多个Remote,可以在TestInterface的实现中使用ReceiverSet
      root_interface_.reset(new RootInterfaceImpl(
          PendingReceiver<RootInterface>(std::move(interface_pipe))));
    }
  }

 private:
  std::unique_ptr<RootInterfaceImpl> root_interface_;
  service_manager::ServiceReceiver service_receiver_;
};

const service_manager::Manifest& GetConsumerManifest() {
  static base::NoDestructor<service_manager::Manifest> manifest{
      service_manager::ManifestBuilder()
          .WithServiceName("consumer_service")
          .ExposeCapability("root", service_manager::Manifest::InterfaceList<
                                        demo::demo_mojo::mojom::RootInterface>())
          .RequireCapability("test_service", "test")
          .Build()};
  return *manifest;
}

const char kProcessTypeEmbedderService[] = "utility";

class ServiceProcessLauncherDelegateImpl
    : public service_manager::ServiceProcessLauncherDelegate {
 public:
  explicit ServiceProcessLauncherDelegateImpl() {}
  ~ServiceProcessLauncherDelegateImpl() override {}

 private:
  // service_manager::ServiceProcessLauncherDelegate:
  void AdjustCommandLineArgumentsForTarget(
      const service_manager::Identity& target,
      base::CommandLine* command_line) override {
    // 本来应该是 service_manager::switches::kProcessTypeService ,但是它有bug
    command_line->AppendSwitchASCII(sandbox::policy::switches::kProcessType,
                                    kProcessTypeEmbedderService);
  }

  DISALLOW_COPY_AND_ASSIGN(ServiceProcessLauncherDelegateImpl);
};

class DemoServiceProcessHost : public service_manager::ServiceProcessHost {
 public:
  DemoServiceProcessHost(ServiceProcessLauncherDelegateImpl* delegate_)
      : launcher_(delegate_,
                  base::CommandLine::ForCurrentProcess()->GetProgram()) {}
  mojo::PendingRemote<service_manager::mojom::Service> Launch(
      const service_manager::Identity& identity,
      sandbox::policy::SandboxType sandbox_type,
      const std::u16string& display_name,
      LaunchCallback callback) override {
    return launcher_.Start(identity, sandbox::policy::SandboxType::kService,
                           std::move(callback));
  }

 private:
  service_manager::ServiceProcessLauncher launcher_;
};

class DemoServiceManagerDelegate
    : public service_manager::ServiceManager::Delegate {
 public:
  DemoServiceManagerDelegate(ServiceProcessLauncherDelegateImpl* delegate)
      : delegate_(delegate) {}
  // 用于启动独立进程的服务
  std::unique_ptr<service_manager::ServiceProcessHost>
  CreateProcessHostForBuiltinServiceInstance(
      const service_manager::Identity& identity) override {
    return std::make_unique<DemoServiceProcessHost>(delegate_);
  }
  //用于启动运行在ServiceManager进程中的服务
  bool RunBuiltinServiceInstanceInCurrentProcess(
      const service_manager::Identity& identity,
      mojo::PendingReceiver<service_manager::mojom::Service> receiver)
      override {
    LOG(INFO) << "RunBuiltinServiceInstanceInCurrentProcess";
    // 在实际代码中需要考虑如何维护Service实例，而不是像这样泄露
    new ConsumerService(std::move(receiver));
    return true;
  }
  //用于启动运行在专门的服务进程中的服务
  std::unique_ptr<service_manager::ServiceProcessHost>
  CreateProcessHostForServiceExecutable(
      const base::FilePath& executable_path) override {
    LOG(INFO) << "CreateProcessHostForServiceExecutable";
    return nullptr;
  }

 private:
  ServiceProcessLauncherDelegateImpl* delegate_;
};

class DemoServerManagerMainDelegate {
 public:
  int Initialize() {
    // 设置日志格式
    logging::SetLogItems(true, false, true, false);
    LOG(INFO) << "Command Line: "
              << base::CommandLine::ForCurrentProcess()->GetCommandLineString();
    return -1;
  }
  // service_manager
  // 在调用RunEmbedderProcess之前已经执行了很多必要的初始化动作,包括:
  // - 初始化CommanLine
  // - 初始化mojo
  int RunEmbedderProcess() {
    LOG(INFO) << "RunEmbedderProcess";
    base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
    base::FeatureList::InitializeInstance(
        command_line->GetSwitchValueASCII(switches::kEnableFeatures),
        command_line->GetSwitchValueASCII(switches::kDisableFeatures));

    if (command_line->GetSwitchValueASCII(
            sandbox::policy::switches::kProcessType) ==
        kProcessTypeEmbedderService) {
      logging::SetLogPrefix("embedder");
      std::string service_name =
          base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
              service_manager::switches::kServiceName);
      if (service_name.empty()) {
        LOG(ERROR) << "Service process requires --service-name";
        return 1;
      }
      // 在它之前不能有MessageLoop或其等价类
      service_manager::ServiceExecutableEnvironment environment;
      // 用MessageLoop也可以
      base::SingleThreadTaskExecutor main_task_executor;
      auto service = this->CreateEmbeddedService(
          service_name, environment.TakeServiceRequestFromCommandLine());
      service->RunUntilTermination();
      base::ThreadPoolInstance::Get()->Shutdown();
      return 0;
    }

    logging::SetLogPrefix("host");
    // 创建主消息循环
    base::SingleThreadTaskExecutor single_thread_task_executor;
    // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
    base::ThreadPoolInstance::CreateAndStartWithDefaultParams("Demo");

    base::Thread ipc_thread("IPC thread");
    ipc_thread.StartWithOptions(
        base::Thread::Options(base::MessagePumpType::IO, 0));
    mojo::core::ScopedIPCSupport ipc_support(
        ipc_thread.task_runner(),
        mojo::core::ScopedIPCSupport::ShutdownPolicy::FAST);

    ServiceProcessLauncherDelegateImpl service_process_launcher_delegate;
    // service_manager::BackgroundServiceManager service_manager(
    //     &service_process_launcher_delegate, this->GetServiceManifests());
    auto service_manager = std::make_unique<service_manager::ServiceManager>(
        this->GetServiceManifests(),
        std::make_unique<DemoServiceManagerDelegate>(
            &service_process_launcher_delegate));
    // auto service_manager = std::make_unique<service_manager::ServiceManager>(
    //   this->GetServiceManifests(),service_manager::ServiceManager::ServiceExecutablePolicy::kSupported);

    // 可以使用以下方式手动启动一个Service
    service_manager->StartService("test_service");
    // service_manager.StartService(service_manager::Identity("consumer_service"));

    // 手动注册一个Service实例
    // mojom::ServicePtr service;
    // context_ =
    // std::make_unique<ServiceContext>(std::make_unique<ConsumerService>(),mojo::MakeRequest(&service));
    // service_manager.RegisterService(service_manager::Identity("consumer_service",
    // mojom::kRootUserID),std::move(service),nullptr);

    // 即使服务请求由自己提供的接口也需要权限
    // demo::mojom::RootInterfacePtr root;
    // context_->connector()->BindInterface("consumer_service", &root);
    // root->Hi("consumer_service");

    // 演示通过consumer_service的context请求由test_service服务提供的test接口
    // demo::mojom::TestInterfacePtr test;
    // context_->connector()->BindInterface("test_service", &test);
    // test->Hello("consumer_service");

    LOG(INFO) << "running...";
    base::RunLoop().Run();
    ipc_thread.Stop();
    base::ThreadPoolInstance::Get()->Shutdown();
    return 0;
  }
  std::vector<service_manager::Manifest> GetServiceManifests() {
    LOG(INFO) << "GetServiceManifests";
    return std::vector<service_manager::Manifest>(
        {GetTestManifest(), GetConsumerManifest()});
  }

  // 在Service进程中被调用,用来创建Service实例
  std::unique_ptr<service_manager::Service> CreateEmbeddedService(
      const std::string& service_name,
      service_manager::mojom::ServiceRequest request) {
    LOG(INFO) << "CreateEmbeddedService: " << service_name;
    if (service_name == "test_service") {
      return std::make_unique<TestService>(std::move(request));
    }
    if (service_name == "consumer_service") {
      return std::make_unique<ConsumerService>(std::move(request));
    }
    return nullptr;
  }
};

int main(int argc, const char** argv) {
  base::AtExitManager at_exit;
  base::CommandLine::Init(argc, argv);
  LOG(INFO) << base::CommandLine::ForCurrentProcess()->GetCommandLineString();

  mojo::core::Init();

  DemoServerManagerMainDelegate delegate;
  delegate.Initialize();
  delegate.RunEmbedderProcess();
  return 0;
}
