#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/task/task_scheduler/task_scheduler.h"

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true,false,true,false);
  // 创建主消息循环
  base::MessageLoop message_loop;
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::TaskScheduler::CreateAndStartWithDefaultParams("Demo");

  // 复制当前文件来创建新的demo
  LOG(INFO) << "hello,world!";

  LOG(INFO) << "running...";
  base::RunLoop().Run();
  return 0;
}
