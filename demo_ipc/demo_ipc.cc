/**
 * 目前 chromium 中的 IPC 机制底层都使用mojo来实现。只有在NaCL中使用以前的IPC通道的机制。
 * 
 */

#include "base/at_exit.h"
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

// For IPC API
#include "ipc/ipc.mojom.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_channel_mojo.h"
#include "ipc/ipc_logging.h"
#include "ipc/ipc_message_macros.h"
// 不能使用这个类，因为它没有标记为导出，链接的时候会报错
//#include "ipc/message_filter_router.h"
#include "ipc/message_router.h"
#include "ipc/message_filter.h"

#include "demo_ipc_messages.h"

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

class ProducerListener : public IPC::Listener {
 public:
  ProducerListener(){}

  ~ProducerListener() override = default;

 private:
  
  void OnChannelConnected(int32_t peer_pid) override {
    LOG(INFO) << "Producer OnChannelConnected: peer_pid= "<<peer_pid;
  }

  bool OnMessageReceived(const IPC::Message& message) override {
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(ProducerListener, message)
      IPC_MESSAGE_HANDLER(IPCTestMsg_Hi,OnHi);
      IPC_MESSAGE_UNHANDLED(handled = false);
    IPC_END_MESSAGE_MAP()
    LOG(INFO) << "Producer OnMessageReceived: handled= " << handled;
    return handled;
  }
  void OnHi(const std::string& who) {
    LOG(INFO) << "ProducerListener run: Hi " << who;
  }
};

class ConsumerListener : public IPC::Listener {
 public:
  ConsumerListener(){}

  ~ConsumerListener() override = default;

 private:
  bool OnMessageReceived(const IPC::Message& message) override {
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(ConsumerListener, message)
      IPC_MESSAGE_HANDLER(IPCTestMsg_RoutedHello,OnRoutedHello);
      IPC_MESSAGE_UNHANDLED(handled = false);
    IPC_END_MESSAGE_MAP()
    LOG(INFO) << "Consumer OnMessageReceived: handled= " << handled;
    return handled;
  }
  void OnRoutedHello(const std::string& who) {
    LOG(INFO) << "ConsumerListener run: RoutedHello " << who;
  }
};

class ConsumerFilter : public IPC::MessageFilter {
public:

  bool AddRoute(int32_t routing_id, IPC::Listener* listener) {
    return router_.AddRoute(routing_id,listener);
  }

  bool OnMessageReceived(const IPC::Message& message) override {
    return router_.RouteMessage(message);
  }

private:
  IPC::MessageRouter router_;
};

// 这不是使用IPC::Channel所必需的，这里是模拟chromium中的使用，用来演示MessageFilter
class ConsumerChannel : public IPC::Listener, public IPC::Sender {
public:
  ConsumerChannel(scoped_refptr<base::SingleThreadTaskRunner> task_runner) 
    : task_runner_(task_runner){}

  bool Init(IPC::ChannelHandle handle) {
    channel_ = IPC::Channel::CreateClient(handle,this,task_runner_);
    LOG(INFO) << "Consumer Connect";
    bool result = channel_->Connect();
    if(result) {
      listener_ = std::make_unique<ConsumerListener>();
      filter_ = std::make_unique<ConsumerFilter>();
      // 注册一个 routing_id 为 1 的消息监听器，所有id为1的消息都会被该监听器接收
      // 在实际项目中要保证在一条通道上，该id要唯一
      filter_->AddRoute(1,listener_.get());
      AddFilter(filter_.get());
    }

    return result;
  }

  void AddFilter(IPC::MessageFilter* filter) {
    filters_.push_back(filter);
  }

  void OnChannelConnected(int32_t peer_pid) override {
    LOG(INFO) << "Consumer OnChannelConnected: peer_pid= "<<peer_pid;
  }

  bool OnMessageReceived(const IPC::Message& message) override {
    LOG(INFO) << "ConsumerChannel OnMessageReceived: message.routing_id()= " << message.routing_id();
    bool handled = false;
    if (message.routing_id() == MSG_ROUTING_CONTROL) {
      handled = OnControlMessageReceived(message);
    } else {
      for(auto filter : filters_)
      {
        if(filter->OnMessageReceived(message)) {
          break;
        }
      }
    }
    return handled;
  }

  bool OnControlMessageReceived(const IPC::Message& message) {
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(ConsumerChannel, message)
      IPC_MESSAGE_HANDLER(IPCTestMsg_Hello,OnHello);
      IPC_MESSAGE_UNHANDLED(handled = false);
    IPC_END_MESSAGE_MAP()
    LOG(INFO) << "ConsumerChannel OnControlMessageReceived: handled= " << handled;
    return handled;
  }

  void OnHello(const std::string& who) {
    LOG(INFO) << "ConsumerChannel run: Hello " << who;
  }

  bool Send(IPC::Message* message) override {
    return channel_->Send(message);
  }

  std::unique_ptr<ConsumerFilter> filter_;
  std::unique_ptr<ConsumerListener> listener_;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
  std::vector<IPC::MessageFilter*> filters_;
  std::unique_ptr<IPC::Channel> channel_;
};

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
  
  // 创建主线程消息循环
  base::MessageLoop message_loop;

  ProducerListener listener;
  std::unique_ptr<IPC::Channel> ipc_channel = IPC::Channel::CreateServer(pipe.release(),&listener,message_loop.task_runner());
  LOG(INFO) << "Producer Connect";
  bool result = ipc_channel->Connect();
  if(result) {
    LOG(INFO) << "Producer Send: IPCTestMsg_Hello Producer";
    ipc_channel->Send(new IPCTestMsg_Hello("Producer"));
    ipc_channel->Send(new IPCTestMsg_RoutedHello(1,"Producer"));
  }

  base::RunLoop run_loop;
  LOG(INFO) << "running...";
  run_loop.Run();
}

void MojoConsumer() {
  // Accept an invitation.
  mojo::IncomingInvitation invitation = mojo::IncomingInvitation::Accept(
      mojo::PlatformChannel::RecoverPassedEndpointFromCommandLine(
          *base::CommandLine::ForCurrentProcess()));
  mojo::ScopedMessagePipeHandle pipe =
      invitation.ExtractMessagePipe("my raw pipe");
  LOG(INFO) << "pipe: " << pipe->value();

  // 创建主线程消息循环
  base::MessageLoop message_loop;

  // ConsumerListener listener;
  // 这里也可以直接把listener传给Channel，但为了演示使用ConsumerChannel
  // std::unique_ptr<IPC::Channel> channel = IPC::Channel::CreateClient(pipe.release(),&listener,message_loop.task_runner());
  // LOG(INFO) << "Consumer Connect";
  // bool result = channel->Connect();

  ConsumerChannel channel(message_loop.task_runner());
  bool result = channel.Init(pipe.release());
  DCHECK(result);
  
  if(result) {
    LOG(INFO) << "Consumer Send: IPCTestMsg_Hi Consumer";
    channel.Send(new IPCTestMsg_Hi("Consumer"));
  }

  base::RunLoop run_loop;
  LOG(INFO) << "running...";
  run_loop.Run();
}

int main(int argc, char** argv) {
  base::AtExitManager at_exit;

  base::CommandLine::Init(argc, argv);
  LOG(INFO) <<"Command Line: "<< base::CommandLine::ForCurrentProcess()->GetCommandLineString();
  
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("Demo");
  
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

  ipc_thread.Stop();
  base::ThreadPoolInstance::Get()->Shutdown();
  return 0;
}
