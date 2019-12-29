#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/task/task_scheduler/task_scheduler.h"


#include <mojo/core/embedder/embedder.h>
#include <mojo/core/embedder/scoped_ipc_support.h>
#include "mojo/public/c/system/buffer.h"
#include "mojo/public/c/system/data_pipe.h"
#include "mojo/public/c/system/message_pipe.h"
#include "mojo/public/cpp/system/buffer.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "mojo/public/cpp/system/message_pipe.h"
#include "mojo/public/cpp/system/simple_watcher.h"
#include "mojo/public/cpp/system/wait.h"

#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "mojo/public/cpp/bindings/interface_ptr.h"

// For services
#include "services/service_manager/embedder/main.h"
#include "services/service_manager/embedder/main_delegate.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "services/service_manager/public/cpp/service.h"
#include "services/service_manager/public/cpp/service_context.h"
#include "services/service_manager/public/cpp/service_binding.h"
#include "services/service_manager/embedder/switches.h"

#include "demo/mojom/test_service.mojom.h"
#include "demo/test_service_catalog_source.h"

// 在新版本中这些类被重命名,这里模拟新版本
template <class T>
using Remote = mojo::InterfacePtr<T>;
template <class T>
using PendingRemote = mojo::InterfacePtrInfo<T>;
template <class T>
using Receiver = mojo::Binding<T>;
template <class T>
using PendingReceiver = mojo::InterfaceRequest<T>;
template <class T>
using ReceiverSet = mojo::BindingSet<T>;

using namespace demo::mojom;

class TestInterfaceImpl : public demo::mojom::TestInterface {
 public:
  TestInterfaceImpl(PendingReceiver<TestInterface> receiver)
      : receiver_(this, std::move(receiver)) {}

  void Hello(const std::string& who) {
    LOG(INFO) << "TestInterfaceImpl run: Hello " << who;
  }

 private:
  // 也可以用StrongBinding
  // 要想让当前接口的一个实例可以服务多个Remote,这里可以使用ReceiverSet
  Receiver<demo::mojom::TestInterface> receiver_;
};
// 这里故意把Service的实现和接口的实现分开,因为很多情况下一个Service会提供多个接口
class TestService : public service_manager::Service {
 public:
  TestService(service_manager::mojom::ServiceRequest request) {
    service_binding_ = std::make_unique<service_manager::ServiceBinding>(this, std::move(request));
  }
  TestService(){
    service_binding_ = std::make_unique<service_manager::ServiceBinding>(this);
  }
  void OnStart() override { LOG(INFO) << "TestService Start."; }
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
  std::unique_ptr<service_manager::ServiceBinding> service_binding_;
};

class RootInterfaceImpl : public demo::mojom::RootInterface {
public:
  RootInterfaceImpl(PendingReceiver<RootInterface> receiver)
      : receiver_(this, std::move(receiver)) {}
  void Run() override {
    LOG(INFO) << "Root run";
  }
private:
  Receiver<RootInterface> receiver_;
};

class ConsumerService : public service_manager::Service {
 public:
  // ConsumerService(service_manager::mojom::ServiceRequest request)
  //     : service_binding_(this, std::move(request)) {}
  void OnStart() override { 
    LOG(INFO) << "ConsumerService Start."; 
  }
  void Hello() {
    // Remote<TestInterface> test;
    // service_binding_.GetConnector()->BindInterface(TestInterface::Name_,
    //                                                mojo::MakeRequest(&test));
    // test->Hello("TestInterface");
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
  //std::unique_ptr<service_manager::ServiceBinding> service_binding_;
};
using namespace service_manager;
class ServiceProcessLauncherDelegateImpl
    : public service_manager::ServiceProcessLauncherDelegate {
 public:
  explicit ServiceProcessLauncherDelegateImpl(MainDelegate* main_delegate)
      : main_delegate_(main_delegate) {}
  ~ServiceProcessLauncherDelegateImpl() override {}

 private:
  // service_manager::ServiceProcessLauncherDelegate:
  void AdjustCommandLineArgumentsForTarget(
      const service_manager::Identity& target,
      base::CommandLine* command_line) override {
    if (main_delegate_->ShouldLaunchAsServiceProcess(target)) {
      command_line->AppendSwitchASCII(switches::kProcessType,
                                      switches::kProcessTypeService);
    }

    main_delegate_->AdjustServiceProcessCommandLine(target, command_line);
  }

  MainDelegate* const main_delegate_;

  DISALLOW_COPY_AND_ASSIGN(ServiceProcessLauncherDelegateImpl);
};

class DemoServerManagerMainDelegate : public service_manager::MainDelegate {
 public:
  DemoServerManagerMainDelegate() {}
  int Initialize(
      const service_manager::MainDelegate::InitializeParams& params) override {
    // 设置日志格式
    logging::SetLogItems(true, false, true, false);
    LOG(INFO) <<"Command Line: "<< base::CommandLine::ForCurrentProcess()->GetCommandLineString();
    return -1;
  }
  // service_manager
  // 在调用RunEmbedderProcess之前已经执行了很多必要的初始化动作,包括:
  // - 初始化CommanLine
  // - 初始化mojo
  int RunEmbedderProcess() override {
    LOG(INFO) << "RunEmbedderProcess";
    // 创建主消息循环
    base::MessageLoop message_loop;
    // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
    base::TaskScheduler::CreateAndStartWithDefaultParams("Demo");
    
    base::Thread ipc_thread("IPC thread");
    ipc_thread.StartWithOptions(
        base::Thread::Options(base::MessageLoop::TYPE_IO, 0));
    mojo::core::ScopedIPCSupport ipc_support(
        ipc_thread.task_runner(),
        mojo::core::ScopedIPCSupport::ShutdownPolicy::FAST);
    
    ServiceProcessLauncherDelegateImpl service_process_launcher_delegate(this);
    service_manager::BackgroundServiceManager service_manager(
        &service_process_launcher_delegate, this->CreateServiceCatalog());
    //service_manager.StartService(service_manager::Identity("test_service"));
    //service_manager.StartService(service_manager::Identity("consumer_service"));
    mojom::ServicePtr service;
    context_ = std::make_unique<ServiceContext>(std::make_unique<ConsumerService>(),mojo::MakeRequest(&service));
    service_manager.RegisterService(service_manager::Identity("consumer_service", mojom::kRootUserID),std::move(service),nullptr);
    
    demo::mojom::RootInterfacePtr root;
    context_->connector()->BindInterface("consumer_service", &root);
    root->Run();

    demo::mojom::TestInterfacePtr test;
    context_->connector()->BindInterface("test_service", &test);
    test->Hello("test_service");

    LOG(INFO) << "running...";
    base::RunLoop().Run();
    ipc_thread.Stop();
    base::TaskScheduler::GetInstance()->Shutdown();
    return 0;
  }
  std::unique_ptr<base::Value> CreateServiceCatalog() override{
    LOG(INFO) << "CreateServiceCatalog";
    auto value = demo::services::CreateTestServiceCatalog();
    return value;
  }
  service_manager::ProcessType OverrideProcessType() override {
    LOG(INFO) << "OverrideProcessType";
    return service_manager::ProcessType::kDefault;
  }
  void OnServiceManagerInitialized(
      const base::Closure& quit_closure,
      service_manager::BackgroundServiceManager* service_manager) override {
    LOG(INFO) << "OnServiceManagerInitialized";
  }
  bool ShouldLaunchAsServiceProcess(const Identity& identity) override {
    return true;
  }
  void AdjustServiceProcessCommandLine(const Identity& identity,base::CommandLine* command_line) override {
    if (identity.name() == "consumer_service") {
      //command_line->AppendSwitchASCII();
    }
  }
  // 在Service进程中被调用,用来创建Service实例
  std::unique_ptr<Service> CreateEmbeddedService(const std::string& service_name) override {
    LOG(INFO) << "CreateEmbeddedService: " << service_name;
    if(service_name == "test_service"){
      return std::make_unique<TestService>();
    }
  }
private:
  std::unique_ptr<ServiceContext> context_;
};

int main(int argc, const char** argv) {
  base::AtExitManager at_exit;
  DemoServerManagerMainDelegate delegate;
  service_manager::MainParams main_params(&delegate);
  main_params.argc = argc;
  main_params.argv = argv;
  return service_manager::Main(main_params);
}
