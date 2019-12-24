#include <base/logging.h>
#include <base/message_loop/message_loop.h>
#include <base/task/post_task.h>
#include <base/task/task_scheduler/task_scheduler.h>
#include <base/threading/thread.h>
#include <mojo/core/embedder/embedder.h>
#include <mojo/core/embedder/scoped_ipc_support.h>
#include <mojo/public/c/system/message_pipe.h>

void Hello()
{
    LOG(INFO)<<"hello,demo!";
    //CHECK(false);
}

int main(int argc,char** argv)
{
    // Build UI thread message loop. This is used by platform
    // implementations for event polling & running background tasks.
    base::MessageLoop message_loop;
    //base::TaskScheduler::CreateAndStartWithDefaultParams("OzoneDemo");
    base::RunLoop run_loop;
    
    message_loop.task_runner()->PostTask(FROM_HERE, base::BindOnce(&Hello));
    base::MessageLoopCurrent::Get()->task_runner()->PostTask(FROM_HERE, base::BindOnce(&Hello));
    //scoped_refptr<base::TaskRunner> task_runner_ =
    //  base::CreateTaskRunnerWithTraits({base::TaskPriority::USER_VISIBLE});
    //task_runner_->PostTask(FROM_HERE,base::BindOnce(&Hello));

    //base::PostTask(FROM_HERE, base::BindOnce(&Hello));
    mojo::core::Init();
    //base::Thread ipc_thread("ipc!");
    //ipc_thread.StartWithOptions(
    //  base::Thread::Options(base::MessageLoop::TYPE_IO, 0));

    //// As long as this object is alive, all Mojo API surface relevant to IPC
    //// connections is usable, and message pipes which span a process boundary will
    //// continue to function.
    //mojo::core::ScopedIPCSupport ipc_support(
    //  ipc_thread.task_runner(),
    //  mojo::core::ScopedIPCSupport::ShutdownPolicy::CLEAN);

    MojoHandle a, b;
    MojoResult result = MojoCreateMessagePipe(NULL, &a, &b);
    DCHECK_EQ(result,MOJO_RESULT_OK);

    {
        MojoMessageHandle message;
        result = MojoCreateMessage(nullptr, &message);
        DCHECK_EQ(result,MOJO_RESULT_OK);
        MojoAppendMessageDataOptions options;
        options.struct_size = sizeof(options);
        options.flags = MOJO_APPEND_MESSAGE_DATA_FLAG_COMMIT_SIZE;
        void* buffer;
        uint32_t buffer_size;
        result = MojoAppendMessageData(message, 6, nullptr, 0, &options, &buffer, &buffer_size);
        DCHECK_EQ(result,MOJO_RESULT_OK);
        memcpy(buffer, "hello", 6);

        result = MojoWriteMessage(a, message, nullptr);
        DCHECK_EQ(result,MOJO_RESULT_OK);
    }

    {
        MojoMessageHandle message;
        MojoResult result = MojoReadMessage(b, nullptr, &message);
        DCHECK_EQ(result,MOJO_RESULT_OK);

        void* buffer = NULL;
        uint32_t num_bytes;
        result = MojoGetMessageData(message, nullptr, &buffer, &num_bytes, nullptr, nullptr);
        printf("Pipe says: %s\n", (const char*)buffer);
    }

    run_loop.Run();
    return 0;
}
