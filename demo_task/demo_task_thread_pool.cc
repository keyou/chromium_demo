#include "base/logging.h"
#include "base/task/thread_pool.h"
#include "base/task/thread_pool/thread_pool_impl.h"
#include "base/task/thread_pool/thread_pool_instance.h"

void Hello() {
  LOG(INFO) << "hello,demo!";
  // CHECK(false);
}

int main(int argc, char** argv) {
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_STDERR;
  logging::InitLogging(settings);
  logging::SetLogItems(true, true, true, false);
  // 初始化线程池，会创建新的线程，在新的线程中会创建消息循环 MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("Demo");

  // 通过以下方法创建任务，默认在线程池中运行
  base::ThreadPool::PostTask(FROM_HERE, {base::TaskPriority::USER_VISIBLE}, base::BindOnce(&Hello));
  
  // 或者通过创建新的TaskRunner来创建任务，TaskRunner可以控制任务执行的顺序以及是否在同一个线程中运行
  scoped_refptr<base::TaskRunner> task_runner_ = base::ThreadPool::CreateTaskRunner({
      base::TaskPriority::USER_VISIBLE,
  });
  task_runner_->PostTask(FROM_HERE, base::BindOnce(&Hello));

  // 由于线程池默认不会阻塞程序运行，因此这里为了看到结果使用getchar()阻塞主线程。
  // (depreated)当前进程中共有4个线程，1个主线程，1个线程池Service线程，2个Worker线程。
  getchar();

  return 0;
}
