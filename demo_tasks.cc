#include <base/logging.h>
#include <base/message_loop/message_loop.h>
#include <base/message_loop/message_loop_current.h>
#include "base/threading/thread_task_runner_handle.h"
#include <base/task/post_task.h>
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/task/thread_pool/thread_pool_impl.h"
#include "base/task/single_thread_task_executor.h"

void Hello()
{
    LOG(INFO)<<"hello,demo!";
    //CHECK(false);
}
#ifdef DEMO_MESSAGE_LOOP
int main(int argc,char** argv)
{
    // 创建消息循环
    base::MessageLoop message_loop;
    // 也可以使用下面的方法。它们的区别仅在于对外MessageLoop对外暴露了更多的内部接口。
    // 在当前线程创建一个可执行task的环境，同样需要使用RunLoop启动
    // base::SingleThreadTaskExecutor main_task_executer;

    base::RunLoop run_loop;
 
    // 使用message_loop对象直接创建任务
    message_loop.task_runner()->PostTask(FROM_HERE, base::BindOnce(&Hello));
    // 获取当前线程的task runner
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE, base::BindOnce(&Hello));

    // 启动消息循环,即使没有任务也会阻塞程序运行。当前进程中只有一个线程。
    run_loop.Run();
 
    return 0;
}
#else // DEMO_TASK
int main(int argc,char** argv)
{
    // 初始化线程池，会创建新的线程，在新的线程中会创建消息循环MessageLoop
    base::ThreadPoolInstance::CreateAndStartWithDefaultParams("Demo");
 
    // 通过以下方法创建任务，默认在线程池中运行
    base::PostTask(FROM_HERE, base::BindOnce(&Hello));
    // 或者通过创建新的TaskRunner来创建任务，TaskRunner可以控制任务执行的顺序以及是否在同一个线程中运行
    scoped_refptr<base::TaskRunner> task_runner_ =
      base::CreateTaskRunner({base::ThreadPool(),base::TaskPriority::USER_VISIBLE,});
    task_runner_->PostTask(FROM_HERE,base::BindOnce(&Hello));

    // 由于线程池默认不会阻塞程序运行，因此这里为了看到结果使用getchar()阻塞主线程。当前进程中共有4个线程，1个主线程，1个线程池Service线程，2个Worker线程。
    getchar();
 
    return 0;
}
#endif
