#include <base/command_line.h>
#include <base/logging.h>
#include <base/run_loop.h>
#include <base/task/single_thread_task_executor.h>
#include <base/task/post_task.h>
#include <base/task/thread_pool/thread_pool_instance.h>

#include <base/threading/thread.h>
#include <mojo/core/embedder/embedder.h>
#include <mojo/core/embedder/scoped_ipc_support.h>
#include "base/process/launch.h"
#include "mojo/public/cpp/platform/platform_channel.h"
#include "mojo/public/cpp/system/invitation.h"

#include <mojo/public/c/system/buffer.h>
#include <mojo/public/c/system/data_pipe.h>
#include <mojo/public/c/system/message_pipe.h>
#include <mojo/public/cpp/system/buffer.h>
#include <mojo/public/cpp/system/data_pipe.h>
#include <mojo/public/cpp/system/message_pipe.h>
#include <mojo/public/cpp/system/simple_watcher.h>
#include <mojo/public/cpp/system/wait.h>

// For bindings API
#include "demo/mojom/test.mojom.h"
#include "demo/mojom/test2.mojom.h"
// #include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/bindings/interface_ptr.h"

class PipeReader {
 public:
  PipeReader(mojo::ScopedMessagePipeHandle pipe)
      : pipe_(std::move(pipe)),
        watcher_(FROM_HERE, mojo::SimpleWatcher::ArmingPolicy::AUTOMATIC) {
    // NOTE: base::Unretained is safe because the callback can never be run
    // after SimpleWatcher destruction.
    watcher_.Watch(
        pipe_.get(), MOJO_HANDLE_SIGNAL_READABLE,
        base::BindRepeating(&PipeReader::OnReadable, base::Unretained(this)));
  }

  ~PipeReader() {}

 private:
  void OnReadable(MojoResult result) {
    if (result == MOJO_RESULT_OK) {
      // 推荐一次性把数据读完
      while (result == MOJO_RESULT_OK) {
        std::vector<uint8_t> data;
        result = mojo::ReadMessageRaw(pipe_.get(), &data, nullptr,
                                      MOJO_READ_MESSAGE_FLAG_NONE);
        if (result == MOJO_RESULT_OK)
          LOG(INFO) << "receive msg(watcher): " << (char*)&data[0];
        else
          LOG(INFO) << "receive finished.";
      }
    } else {
      LOG(INFO) << "pipe closed. result= " << result;
    }
  }

  mojo::ScopedMessagePipeHandle pipe_;
  mojo::SimpleWatcher watcher_;
};

void MojoProducer() {
  // 创建一条系统级的IPC通信通道
  // 在linux上是 domain socket, Windows 是 named pipe，MacOS是Mach Port,该通道用于支持夸进程的消息通信
  mojo::PlatformChannel channel;
  LOG(INFO) << "local: "
            << channel.local_endpoint().platform_handle().GetFD().get()
            << " remote: "
            << channel.remote_endpoint().platform_handle().GetFD().get();

  mojo::OutgoingInvitation invitation;
  // 创建1个Ｍessage Pipe用来和其他进程通信
  mojo::ScopedMessagePipeHandle pipe =
      invitation.AttachMessagePipe("my raw pipe");
  LOG(INFO) << "pipe: " << pipe->value();

  base::LaunchOptions options;
  base::CommandLine command_line(
      base::CommandLine::ForCurrentProcess()->GetProgram());
  // 将PlatformChannel中的RemoteEndpoint的fd作为参数传递给子进程
  // 在posix中，fd会被复制到新的随机的fd，fd号改变
  // 在windows中，fd被复制后会直接进行传递，fd号不变
  channel.PrepareToPassRemoteEndpoint(&options, &command_line);
  base::Process child_process = base::LaunchProcess(command_line, options);
  channel.RemoteProcessLaunchAttempted();

  mojo::OutgoingInvitation::Send(
      std::move(invitation), child_process.Handle(),
      channel.TakeLocalEndpoint(),
      base::BindRepeating(
          [](const std::string& error) { LOG(ERROR) << error; }));

  MojoResult result;

  // C++ platform API, message pipe write test
  {
    const char kMessage[] = "hello";
    result = mojo::WriteMessageRaw(pipe.get(), kMessage, sizeof(kMessage),
                                   nullptr, 0, MOJO_WRITE_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "send: " << kMessage;
  }

  // C platform API, message pipe write test
  {
    const char kMessage[] = "HELLO";
    MojoMessageHandle message;
    result = MojoCreateMessage(nullptr, &message);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    MojoAppendMessageDataOptions options;
    options.struct_size = sizeof(options);
    options.flags = MOJO_APPEND_MESSAGE_DATA_FLAG_COMMIT_SIZE;
    void* buffer;
    uint32_t buffer_size;
    result = MojoAppendMessageData(message, 6, nullptr, 0, &options, &buffer,
                                   &buffer_size);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    memcpy(buffer, kMessage, sizeof(kMessage));
    result = MojoWriteMessage(pipe->value(), message, nullptr);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "send: " << (char*)buffer;
  }
  // Data Pipe transport by MessagePipe
  {
    // DataPipe 内部使用Shared Ｍemory实现
    // TODO: 研究 base::WritableSharedMemoryRegion
    const char kMessage[] = "DataPipe";
    mojo::ScopedDataPipeProducerHandle producer;
    mojo::ScopedDataPipeConsumerHandle consumer;
    // 内部涉及系统资源的分配，可能会失败，因此不建议使用 mojo::DataPipe
    // 来创建，会导致崩溃
    result = mojo::CreateDataPipe(nullptr, producer, consumer);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    result = mojo::WriteMessageRaw(pipe.get(), kMessage, sizeof(kMessage),
                                   &consumer->value(), 1,
                                   MOJO_WRITE_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "send msg: " << kMessage << " producer: " << producer->value()
              << " consumer: " << consumer->value();

    uint32_t length = sizeof(kMessage);
    result = producer->WriteData(kMessage, &length, MOJO_WRITE_DATA_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "send data: " << kMessage;
    // 这里需要release
    // handle，因为WriteMessage内部会close发送的handle,也可以在ＷriteMessage的时候使用consumer.release().value()来同时释放所有权
    ALLOW_UNUSED_LOCAL(consumer.release());
    // 不是必须的，这里是为了调试，故意不释放,后面的同理
    ALLOW_UNUSED_LOCAL(producer.release());
  }
  // Message Pipe transport by MessagePipe
  {
    const std::string kMessage("MessagePipe\0", 12);
    mojo::ScopedMessagePipeHandle client;
    mojo::ScopedMessagePipeHandle server;
    // 也可以使用 mojo::MessagePipe
    // 来更方便的创建，它内部不涉及系统资源的创建因此不会失败
    result = mojo::CreateMessagePipe(nullptr, &client, &server);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    result = mojo::WriteMessageRaw(pipe.get(), kMessage.c_str(),
                                   kMessage.length(), &client->value(), 1,
                                   MOJO_WRITE_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "send msg: " << kMessage << " client: " << client->value()
              << " server: " << server->value();

    result =
        mojo::WriteMessageRaw(server.get(), kMessage.c_str(), kMessage.length(),
                              nullptr, 0, MOJO_WRITE_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "send msg server: " << kMessage;
    ALLOW_UNUSED_LOCAL(client.release());
    ALLOW_UNUSED_LOCAL(server.release());
  }
  // Shared Buffer Test
  {
    // Shared Buffer 内部也使用 Shared Memory 实现
    const std::string kMessage("SharedBuffer\0", 13);
    mojo::ScopedSharedBufferHandle buffer =
        mojo::SharedBufferHandle::Create(4096);
    // 复制一个handle，因为ＷriteMessageRaw会close传入的handle
    mojo::ScopedSharedBufferHandle buffer_clone =
        buffer->Clone(mojo::SharedBufferHandle::AccessMode::READ_WRITE);
    result = mojo::WriteMessageRaw(pipe.get(), kMessage.c_str(),
                                   kMessage.length(), &buffer_clone->value(), 1,
                                   MOJO_WRITE_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "send msg: " << kMessage << " buffer: " << buffer->value();

    mojo::ScopedSharedBufferMapping mapping = buffer->Map(kMessage.length());
    DCHECK(mapping);
    std::copy(kMessage.begin(), kMessage.end(),
              static_cast<char*>(mapping.get()));
    LOG(INFO) << "write buffer: " << kMessage;
    ALLOW_UNUSED_LOCAL(buffer_clone.release());
    ALLOW_UNUSED_LOCAL(buffer.release());
  }
  // C++ Signal&Trap test
  {
    const char kMessage[] = "SimplerWatcher";
    MojoResult result =
        mojo::WriteMessageRaw(pipe.get(), kMessage, sizeof(kMessage), nullptr,
                              0, MOJO_WRITE_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "send: " << kMessage;

    // 延迟5s再次发送消息
    base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(
            [](mojo::ScopedMessagePipeHandle pipe) {
              const char kMessage[] = "SimplerWatcher2";
              MojoResult result = mojo::WriteMessageRaw(
                  pipe.get(), kMessage, sizeof(kMessage), nullptr, 0,
                  MOJO_WRITE_MESSAGE_FLAG_NONE);
              DCHECK_EQ(result, MOJO_RESULT_OK);
              LOG(INFO) << "send: " << kMessage;
              // pipe 生命周期结束，被销毁
            },
            std::move(pipe)),
        base::TimeDelta::FromSeconds(5));
  }
}

void MojoConsumer() {
  // Accept an invitation.
  mojo::IncomingInvitation invitation = mojo::IncomingInvitation::Accept(
      mojo::PlatformChannel::RecoverPassedEndpointFromCommandLine(
          *base::CommandLine::ForCurrentProcess()));
  mojo::ScopedMessagePipeHandle pipe =
      invitation.ExtractMessagePipe("my raw pipe");
  LOG(INFO) << "pipe: " << pipe->value();

  MojoResult result;
  // 保证有数据可读
  if (!pipe->QuerySignalsState().readable()) {
    // 等待至少有一个消息可读
    result = mojo::Wait(pipe.get(), MOJO_HANDLE_SIGNAL_READABLE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    // 仅用于测试，上面只能保证至少有一个消息可读，为了代码简单sleep
    // 1s，保证下面的所有读都可以成功
    sleep(1);
  }
  // C++ platform API, message pipe read test
  {
    std::vector<uint8_t> data;
    result = mojo::ReadMessageRaw(pipe.get(), &data, nullptr,
                                  MOJO_READ_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "receive msg: " << (char*)&data[0];
  }
  // C platform API, message pipe read test
  {
    MojoMessageHandle message;
    MojoResult result = MojoReadMessage(pipe->value(), nullptr, &message);
    DCHECK_EQ(result, MOJO_RESULT_OK);

    void* buffer = nullptr;
    uint32_t num_bytes;
    result = MojoGetMessageData(message, nullptr, &buffer, &num_bytes, nullptr,
                                nullptr);
    LOG(INFO) << "receive msg: " << (const char*)buffer;
  }
  // Data Pipe transport by MessagePipe
  {
    mojo::ScopedMessageHandle message;
    result =
        mojo::ReadMessageNew(pipe.get(), &message, MOJO_READ_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    void* data = nullptr;
    uint32_t length;
    std::vector<mojo::ScopedHandle> handles;
    result = mojo::GetMessageData(message.get(), &data, &length, &handles,
                                  MOJO_GET_MESSAGE_DATA_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "receive msg: " << (char*)data
              << " consumer: " << handles[0]->value();

    mojo::ScopedDataPipeConsumerHandle consumer =
        mojo::ScopedDataPipeConsumerHandle::From(std::move(handles[0]));
    char buffer[100];
    uint32_t num_bytes = 100;
    result = consumer->ReadData(buffer, &num_bytes, MOJO_READ_DATA_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "receive data: " << buffer;
    ALLOW_UNUSED_LOCAL(consumer.release());
  }
  // Message Pipe transport by MessagePipe
  {
    std::vector<uint8_t> data;
    std::vector<mojo::ScopedHandle> handles;
    result = mojo::ReadMessageRaw(pipe.get(), &data, &handles,
                                  MOJO_READ_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "receive msg: " << (char*)&data[0]
              << " client: " << handles[0]->value();

    mojo::ScopedMessagePipeHandle client =
        mojo::ScopedMessagePipeHandle::From(std::move(handles[0]));
    std::vector<uint8_t> data2;
    result = mojo::ReadMessageRaw(client.get(), &data2, nullptr,
                                  MOJO_READ_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "receive msg client: " << (char*)&data2[0];
    ALLOW_UNUSED_LOCAL(client.release());
  }
  // Shared Buffer Test
  {
    std::vector<uint8_t> data;
    std::vector<mojo::ScopedHandle> handles;
    result = mojo::ReadMessageRaw(pipe.get(), &data, &handles,
                                  MOJO_READ_MESSAGE_FLAG_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    LOG(INFO) << "receive msg: " << (char*)&data[0]
              << " buffer: " << handles[0]->value();

    mojo::ScopedSharedBufferHandle buffer =
        mojo::ScopedSharedBufferHandle::From(std::move(handles[0]));
    mojo::ScopedSharedBufferMapping mapping = buffer->Map(64);
    LOG(INFO) << "read buffer: " << static_cast<char*>(mapping.get());
    ALLOW_UNUSED_LOCAL(buffer.release());
  }
  // C++ Signal&Trap test
  {
    // 为了测试，故意泄漏
    new PipeReader(std::move(pipe));
  }
}

int main(int argc, char** argv) {
  base::CommandLine::Init(argc, argv);
  LOG(INFO) << base::CommandLine::ForCurrentProcess()->GetCommandLineString();
  // 创建主线程消息循环
  base::SingleThreadTaskExecutor main_task_executor;
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
