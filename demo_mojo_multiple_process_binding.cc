#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/task/post_task.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/process/launch.h"
#include "base/threading/thread.h"

#include "mojo/core/embedder/embedder.h"
#include "mojo/core/embedder/scoped_ipc_support.h"
#include "mojo/public/cpp/platform/platform_channel.h"
#include "mojo/public/cpp/system/invitation.h"

#include "mojo/public/c/system/buffer.h"
#include "mojo/public/c/system/data_pipe.h"
#include "mojo/public/c/system/message_pipe.h"
#include "mojo/public/cpp/system/buffer.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "mojo/public/cpp/system/message_pipe.h"
#include "mojo/public/cpp/system/simple_watcher.h"
#include "mojo/public/cpp/system/wait.h"

// For bindings API
#include "demo/mojom/test.mojom.h"
#include "demo/mojom/test2.mojom.h"
#include "demo/mojom/test3.mojom.h"
#include "demo/mojom/test4.mojom.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/bindings/interface_ptr.h"

// For associated bindings API
#include "mojo/public/cpp/bindings/associated_binding.h"
#include "mojo/public/cpp/bindings/associated_interface_ptr.h"
#include "mojo/public/cpp/bindings/scoped_interface_endpoint_handle.h"

#include <iostream>
#include <vector>

// 在新版本中这些类被重命名,这里模拟新版本
template<class T> using Remote = mojo::InterfacePtr<T>;
template<class T> using PendingRemote = mojo::InterfacePtrInfo<T>;
template<class T> using Receiver = mojo::Binding<T>;
template<class T> using PendingReceiver = mojo::InterfaceRequest<T>;

// 以下定义用于模拟新版本的关联接口
template<class T> using AssociatedRemote = mojo::AssociatedInterfacePtr<T>;
template<class T> using PendingAssociatedRemote = mojo::AssociatedInterfacePtrInfo<T>;
template<class T> using AssociatedReceiver = mojo::AssociatedBinding<T>;
template<class T> using PendingAssociatedReceiver = mojo::AssociatedInterfaceRequest<T>;

using namespace demo::mojom;

#pragma region Test
class TestImpl : public demo::mojom::Test {
 public:
  void Hello(const std::string& who) override {
    who_ = who;
    LOG(INFO) << "Test1 run: Hello " << who_;
  }

  void Hi(HiCallback callback) override {
    LOG(INFO) << "Test1 run: Hi " << who_;
    std::move(callback).Run(who_);
  }

 private:
  std::string who_;
};
#pragma endregion

#pragma region Test2
class Test2Impl : public demo::mojom::Test2 {
public:
  explicit Test2Impl(PendingReceiver<demo::mojom::Test2> receiver)
    : receiver_(this,std::move(receiver)) {
  }
  void SendMessagePipeHandle(mojo::ScopedMessagePipeHandle pipe_handle) override {
    // mojo::WriteMessage(pipe_handle.get(),...)
    LOG(INFO) << "Test2 run: SendMessagePipeHandle "<< pipe_handle->value();
  }
private:
  Receiver<demo::mojom::Test2> receiver_;
};

class ApiImpl : public demo::mojom::Api {
public:
  explicit ApiImpl(PendingReceiver<Api> receiver)
    : receiver_(this,std::move(receiver)),associated_receiver_(nullptr) {}
  explicit ApiImpl(PendingAssociatedReceiver<Api> receiver)
    : receiver_(nullptr),associated_receiver_(this,std::move(receiver)){}

  void PrintApi(const std::string& data) override {
    LOG(INFO) << "Api run: PrintApi " << data;
  }
  
private:
  Receiver<Api> receiver_;
  AssociatedReceiver<Api> associated_receiver_;
};

class Api2Impl : public demo::mojom::Api2 {
public:
  explicit Api2Impl(PendingReceiver<Api2> receiver)
    : receiver_(this,std::move(receiver)),associated_receiver_(nullptr) {}
  explicit Api2Impl(PendingAssociatedReceiver<Api2> receiver)
    : receiver_(nullptr),associated_receiver_(this,std::move(receiver)){}
  
  void PrintApi2(const std::string& data) override {
    LOG(INFO) << "Api2 run: PrintApi2 " << data;
  }
private:
  Receiver<Api2> receiver_;
  AssociatedReceiver<Api2> associated_receiver_;
};
#pragma endregion

#pragma region Test3
class Test3Impl : public demo::mojom::Test3 {
 public:
  explicit Test3Impl(PendingReceiver<Test3> receiver)
    : receiver_(this,std::move(receiver)) {
  }
  void GetApi(PendingReceiver<Api> api) override {
    LOG(INFO) << "Test3 run: GetApi";
    api_ = std::make_unique<ApiImpl>(std::move(api));
  }
  void SetApi2(Remote<Api2> api2) override {
    LOG(INFO) <<"Test3 run: SetApi2";
    remote_api2_ = std::move(api2);
    remote_api2_->PrintApi2("api2");
    LOG(INFO) <<"Test3 call: Api2::PrintApi2";
  }

 private:
  std::unique_ptr<ApiImpl> api_;
  Remote<Api2> remote_api2_;
  Receiver<Test3> receiver_;
};

class Test32Impl : public demo::mojom::Test32 {
 public:
  explicit Test32Impl(PendingReceiver<Test32> receiver)
    : receiver_(this,std::move(receiver)) {
  }
  void GetApi(PendingAssociatedReceiver<Api> api) override {
    LOG(INFO) << "Test32 run: GetApi";
    api_ = std::make_unique<ApiImpl>(std::move(api));
  }
  void SetApi2(PendingAssociatedRemote<Api2> api2) override {
    LOG(INFO) <<"Test32 run: SetApi2";
    remote_api2_.Bind(std::move(api2));
    remote_api2_->PrintApi2("api2");
    LOG(INFO) <<"Test32 call: Api2::PrintApi2";
  }

 private:
  std::unique_ptr<ApiImpl> api_;
  AssociatedRemote<Api2> remote_api2_;
  Receiver<Test32> receiver_;
};
#pragma endregion

#pragma region Test4
class Interface1Impl : public Interface1 {
public:
    explicit Interface1Impl(PendingReceiver<Interface1> receiver)
    : receiver_(this,std::move(receiver)){}
    void Hello(const std::string& who) override {
      LOG(INFO) << "Interface1 run: Hello " << who;
    }
private:
  Receiver<Interface1> receiver_;
};

class Interface2Impl : public Interface2 {
public:
    explicit Interface2Impl(PendingReceiver<Interface2> receiver)
    : receiver_(this,std::move(receiver)){}

    void Hi(const std::string& who) override {
      LOG(INFO) << "Interface2 run: Hi " << who;
    }
private:
  Receiver<Interface2> receiver_;
};

// 在新版本中InterfaceProvider被改名为InterfaceBroker,这里只是说明它们两个的关系,没有实际作用
using InterfaceProvider = InterfaceBroker;
class InterfaceBrokerImpl : public InterfaceBroker {
public:
  explicit InterfaceBrokerImpl(PendingReceiver<InterfaceBroker> receiver)
    : receiver_(this,std::move(receiver)){}
  
  void GetInterface(const std::string& name, mojo::ScopedMessagePipeHandle pipe_handle) override {
    std::move(binders_[name]).Run(std::move(pipe_handle));
  }

  template<class InterfaceT,class InterfaceImplT>
  void AddMap(const std::string& name) {
    binders_.emplace(name,base::BindRepeating([](mojo::ScopedMessagePipeHandle handle){
      new InterfaceImplT(PendingReceiver<InterfaceT>(std::move(handle)));
    }));
  }

private:
  // 这里也可以使用service_manager::BinderRegistry
  using BinderMap =
      std::map<std::string,
               base::RepeatingCallback<void(mojo::ScopedMessagePipeHandle)>>;
  BinderMap binders_;
  Receiver<InterfaceBroker> receiver_;
};
using InterfaceProviderImpl = InterfaceBrokerImpl;
#pragma endregion

void MojoProducer() {
  // 创建一条系统级的IPC通信通道
  // 在linux上是 socket pair, Windows 是 named pipe，该通道用于支持ＭessagePipe
  mojo::PlatformChannel channel;
  LOG(INFO) << "local: "
            << channel.local_endpoint().platform_handle().GetFD().get()
            << " remote: "
            << channel.remote_endpoint().platform_handle().GetFD().get();

  mojo::OutgoingInvitation invitation;
  // 创建n个Ｍessage Pipe备用
  mojo::ScopedMessagePipeHandle pipe =
      invitation.AttachMessagePipe("my raw pipe");
  mojo::ScopedMessagePipeHandle pipe21 =
      invitation.AttachMessagePipe("my test21 pipe");
  mojo::ScopedMessagePipeHandle pipe22 =
      invitation.AttachMessagePipe("my test22 pipe");
  mojo::ScopedMessagePipeHandle pipe31 =
      invitation.AttachMessagePipe("my test31 pipe");
  mojo::ScopedMessagePipeHandle pipe32 =
      invitation.AttachMessagePipe("my test32 pipe");
  mojo::ScopedMessagePipeHandle pipe4 =
      invitation.AttachMessagePipe("my test4 pipe");
  LOG(INFO) << "pipe: " << pipe->value();

  base::LaunchOptions options;
  base::CommandLine command_line(
      base::CommandLine::ForCurrentProcess()->GetProgram());
  channel.PrepareToPassRemoteEndpoint(&options, &command_line);
  base::Process child_process = base::LaunchProcess(command_line, options);
  channel.RemoteProcessLaunchAttempted();

  mojo::OutgoingInvitation::Send(
      std::move(invitation), child_process.Handle(),
      channel.TakeLocalEndpoint(),
      base::BindRepeating(
          [](const std::string& error) { LOG(ERROR) << error; }));

  // C++ binding API test
  {
    // 这是调用端，注意以下代码不依赖Test的任何实现细节。
    // 在2019年3月之后的代码中这两个类被改为了 Remote,PendingRemote
    using TestPtr = mojo::InterfacePtr<demo::mojom::Test>;
    using TestPtrInfo = mojo::InterfacePtrInfo<demo::mojom::Test>;
    auto test_ptr = new TestPtr(TestPtrInfo(std::move(pipe), 0));
    auto& test = *test_ptr;
    test->Hello("World!");
    LOG(INFO) << "Test1 call: Hello";
    test->Hi(base::BindOnce(
        [](const std::string& who) { LOG(INFO) << "Test1 response: Hi " << who; }));
    LOG(INFO) << "Test1 call: Hi";
    // 新版本中的 Remote::BindNewPipeAndPassReceiver()
    // 只是一个语法糖，只在相同进程中才比较方便；
    // 它内部会创建新的MessagePipe，如果要跨进程需要取出它并传递出去；
    // 官方文档很多示例代码都是在单进程中使用的，这点需要注意；
  }
  // 使用mojo接口发送handle到对端,其他端就可以使用该handle进行通信
  {
    Remote<Test2> test2;
    // 演示使用Bind而不是Remote的构造函数,因为新版的示例大多都是这种方式,两种方式效果一样
    test2.Bind(PendingRemote<Test2>(std::move(pipe21),0));
    test2->SendMessagePipeHandle(std::move(pipe22));
    LOG(INFO) << "Test2 call: SendMessagePipeHandle";
  }
  // 使用mojo接口发送其他mojo接口
  {
    // 使用 pipe31 来调用Test3接口
    Remote<Test3> test3(PendingRemote<Test3>(std::move(pipe31),0));

    // 创建新的MessagePipe用于Api接口的调用
    mojo::MessagePipe api_pipe;
    Remote<Api> api(PendingRemote<Api>(std::move(api_pipe.handle0),0));
    test3->GetApi(PendingReceiver<Api>(std::move(api_pipe.handle1)));
    // 使用MakeRequest结果和上面一样，可以更简单,在更新的版本中Remote中添加了BindNew*方法，用来取代MakeRequest
    // Remote<Api> api;
    // test3->GetApi(mojo::MakeRequest(&api));
    LOG(INFO) << "Test3 call: GetApi";
    api->PrintApi("api");
    LOG(INFO) << "Api call: PrintApi";

    // 创建新的MessagePipe用于Api2接口的调用
    mojo::MessagePipe api2_pipe;
    Remote<Api2> api2(PendingRemote<Api2>(std::move(api2_pipe.handle0),0));
    new Api2Impl(PendingReceiver<Api2>(std::move(api2_pipe.handle1)));
    test3->SetApi2(std::move(api2));
    LOG(INFO) << "Test3 call: SetApi2";
  }

  // 关联接口(Associated Interfaces),关联接口只需要使用一个pipe即可,用来避免多个pipe导致的多接口间调用顺序无法保证的问题
  {
    // 使用 pipe32 来调用Test32接口
    // 为了避免pipe32被销毁后Api2Impl无法响应对方的调用,这里使用new
    auto test32_ptr = new Remote<Test32>(PendingRemote<Test32>(std::move(pipe32),0));
    auto& test32 = *test32_ptr;

    // 创建新的Endpoint用于Api接口的调用
    mojo::ScopedInterfaceEndpointHandle handle0;
    mojo::ScopedInterfaceEndpointHandle handle1;
    mojo::ScopedInterfaceEndpointHandle::CreatePairPendingAssociation(&handle0,&handle1);
    AssociatedRemote<Api> api(PendingAssociatedRemote<Api>(std::move(handle0),0));
    test32->GetApi(PendingAssociatedReceiver<Api>(std::move(handle1)));
    LOG(INFO) << "Test32 call: GetApi";
    api->PrintApi("api");
    LOG(INFO) << "Api call: PrintApi";

    // 创建新的Endpoint用于Api2接口的调用
    mojo::ScopedInterfaceEndpointHandle handle00;
    mojo::ScopedInterfaceEndpointHandle handle11;
    mojo::ScopedInterfaceEndpointHandle::CreatePairPendingAssociation(&handle00,&handle11);
    new Api2Impl(PendingAssociatedReceiver<Api2>(std::move(handle11)));
    test32->SetApi2(PendingAssociatedRemote<Api2>(std::move(handle00),0));
    LOG(INFO) << "Test32 call: SetApi2";
  }
  // 封装出通用的 InterfaceBroker 以便支撑任何新的接口
  {
    Remote<InterfaceBroker> broker(PendingRemote<InterfaceBroker>(std::move(pipe4),0));
    
    mojo::MessagePipe interface1_pipe;
    Remote<Interface1> interface1(PendingRemote<Interface1>(std::move(interface1_pipe.handle0),0));
    broker->GetInterface("Interface1",std::move(interface1_pipe.handle1));
    interface1->Hello("Interface1");
    LOG(INFO) << "Interface1 call: Hello";

    mojo::MessagePipe interface2_pipe;
    Remote<Interface2> interface2(PendingRemote<Interface2>(std::move(interface2_pipe.handle0),0));
    broker->GetInterface("Interface2",std::move(interface2_pipe.handle1));
    interface2->Hi("Interface2");
    LOG(INFO) << "Interface2 call: Hi";
  }
}

void MojoConsumer() {
  // Accept an invitation.
  mojo::IncomingInvitation invitation = mojo::IncomingInvitation::Accept(
      mojo::PlatformChannel::RecoverPassedEndpointFromCommandLine(
          *base::CommandLine::ForCurrentProcess()));
  mojo::ScopedMessagePipeHandle pipe =
      invitation.ExtractMessagePipe("my raw pipe");
  mojo::ScopedMessagePipeHandle pipe21 =
      invitation.ExtractMessagePipe("my test21 pipe");
  mojo::ScopedMessagePipeHandle pipe22 =
      invitation.ExtractMessagePipe("my test22 pipe");
  mojo::ScopedMessagePipeHandle pipe31 =
      invitation.ExtractMessagePipe("my test31 pipe");
  mojo::ScopedMessagePipeHandle pipe32 =
      invitation.ExtractMessagePipe("my test32 pipe");
  mojo::ScopedMessagePipeHandle pipe4 =
      invitation.ExtractMessagePipe("my test4 pipe");
  LOG(INFO) << "pipe: " << pipe->value();

  // C++ binding API test
  {
    // 这是执行端，需要将Test接口bind到具体的实现类上。
    // 在新的版本中，这两个类被改为了 Receiver,PendingReceiver
    using TestBinding = mojo::Binding<demo::mojom::Test>;
    using TestRequest = mojo::InterfaceRequest<demo::mojom::Test>;
    auto test = new TestImpl;
    // 为了测试，故意泄漏，避免pipe被close
    auto binding = new TestBinding(test, TestRequest(std::move(pipe)));
    ALLOW_UNUSED_LOCAL(binding);
  }
  {
    // 演示将Receiver放到实现类中
    new Test2Impl(PendingReceiver<Test2>(std::move(pipe21)));
  }
  {
    new Test3Impl(PendingReceiver<Test3>(std::move(pipe31)));
  }
  {
    new Test32Impl(PendingReceiver<Test32>(std::move(pipe32)));
  }
  {
    auto test4 = new InterfaceBrokerImpl(PendingReceiver<InterfaceBroker>(std::move(pipe4)));
    test4->AddMap<Interface1,Interface1Impl>("Interface1");
    test4->AddMap<Interface2,Interface2Impl>("Interface2");
  }
}

int main(int argc, char** argv) {
  base::CommandLine::Init(argc, argv);
  LOG(INFO) << base::CommandLine::ForCurrentProcess()->GetCommandLineString();
  // 创建主线程消息循环
  base::MessageLoop message_loop;
  base::RunLoop run_loop;

  // Init会创建一个sokcetpair和一条pipe，共4个fd
  mojo::core::Init();
  base::Thread ipc_thread("ipc!");
  ipc_thread.StartWithOptions(
      base::Thread::Options(base::MessagePumpType::IO, 0));

  // 初始化mojo的后台线程，用来异步收发消息存储到缓存
  mojo::core::ScopedIPCSupport ipc_support(
      ipc_thread.task_runner(),
      mojo::core::ScopedIPCSupport::ShutdownPolicy::CLEAN);

  if (argc < 2) {
    logging::SetLogPrefix("producer");
    MojoProducer();
  } else {
    logging::SetLogPrefix("consumer");
    MojoConsumer();
  }

  LOG(INFO) << "running...";
  run_loop.Run();
  return 0;
}
