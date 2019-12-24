#include <base/logging.h>
#include <base/message_loop/message_loop.h>
#include <base/task/post_task.h>
#include <base/task/task_scheduler/task_scheduler.h>

void Hello()
{
    LOG(INFO)<<"hello,demo!";
    //CHECK(false);
}
#ifdef DEMO_MESSAGE_LOOP
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


    run_loop.Run();

    return 0;
}
#else // DEMO_TASK
int main(int argc,char** argv)
{
    base::TaskScheduler::CreateAndStartWithDefaultParams("OzoneDemo");
    //scoped_refptr<base::TaskRunner> task_runner_ =
    //  base::CreateTaskRunnerWithTraits({base::TaskPriority::USER_VISIBLE});
    //task_runner_->PostTask(FROM_HERE,base::BindOnce(&Hello));
    base::PostTask(FROM_HERE, base::BindOnce(&Hello));

    //base::MessageLoopCurrent::Get()->task_runner()->PostTask(FROM_HERE, base::BindOnce(&Hello));

    getchar();
    return 0;
}
#endif
