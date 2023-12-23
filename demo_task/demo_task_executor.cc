#include "base/logging.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/single_thread_task_runner.h"
// #include "base/threading/thread_task_runner_handle.h"

struct Test {
  int data;
};

void Hello() {
  LOG(INFO) << "hello,demo!";
  // CHECK(false);
}

int main(int argc, char** argv) {
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_STDERR;
  logging::InitLogging(settings);
  logging::SetLogItems(true, true, true, false);
  // 在当前线程中创建消息循环。旧版本的 MessageLoop 被换成了 SingleThreadTaskExecutor
  // 详见 commit 636e705be41ed9e7f50cdb13ceb5a9af5e3f4e5c
  base::SingleThreadTaskExecutor main_thread_task_executor;

  base::RunLoop run_loop;

  // 创建任务
  main_thread_task_executor.task_runner()->PostTask(FROM_HERE, base::BindOnce(&Hello));

  Test t{0};
  main_thread_task_executor.task_runner()->PostTask(FROM_HERE,
                                                    base::BindOnce(
                                                        [](Test& t) {
                                                          t.data = 100;
                                                          DLOG(INFO) << "data1: " << t.data;
                                                        },
                                                        std::ref(t)));

  // 获取当前线程的task runner，并使用它创建任务
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(&Hello));

  // 启动消息循环,即使没有任务也会阻塞程序运行。当前进程中只有一个线程。
  // run_loop.Run();

  // 启动消息循环，当所有的 task 执行完毕后退出
  run_loop.RunUntilIdle();

  DLOG(INFO) << "data2: " << t.data;

  return 0;
}
