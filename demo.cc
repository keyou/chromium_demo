#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true,false,true,false);

  // 创建消息循环，旧版本的 MessageLoop 被换成了 SingleThreadTaskExecutor
  // 详见 commit 636e705be41ed9e7f50cdb13ceb5a9af5e3f4e5c
  base::SingleThreadTaskExecutor main_thread_task_executor;
  
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("Demo");

  // 复制当前文件来创建新的demo
  LOG(INFO) << "hello,world!";

  LOG(INFO) << "running...";
  base::RunLoop().Run();
  return 0;
}
