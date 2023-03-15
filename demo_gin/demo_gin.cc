#include "base/at_exit.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/single_thread_task_runner.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/thread_task_runner_handle.h"
#include "demo_gin/shell_runner_delegate.h"
#include "gin/array_buffer.h"
#include "gin/public/isolate_holder.h"
#include "gin/shell_runner.h"
#include "gin/v8_initializer.h"

// 要执行的脚本
// 可以改为读取文件形式
const char kScript[] =
    R"script(
  // 输出Hello World
  log('hello world');

  // 进行计算 输出结果 Result 3
  let a = demo.add(1,2);
  log(`Result ${a}`);

  // 异步计算 输出结果 1.before 2.after 3. result:3
  log('before call async');
  asyncDemo.add(1,2).then(m=>log(`result:${m}`));
  log('after call async');

  )script";

namespace {
// 执行脚本
void Run(base::WeakPtr<gin::Runner> runner) {
  if (!runner)
    return;
  gin::Runner::Scope scope(runner.get());

  runner->Run(kScript, "demo");
}

}  // namespace

int main(int argc, char** argv) {
  // 构建事件循环及相关线程
  base::AtExitManager at_exit;
  base::CommandLine::Init(argc, argv);
  base::SingleThreadTaskExecutor main_thread_task_executor;
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("gin");

  // Load V8快照默认资源 这个是 GN 里配置的哪个
  gin::V8Initializer::LoadV8Snapshot();

  {
    // 初始化VM
    gin::IsolateHolder::Initialize(gin::IsolateHolder::kNonStrictMode,
                                   gin::ArrayBufferAllocator::SharedInstance());

    // 创建VM实例
    gin::IsolateHolder instance(
        base::ThreadTaskRunnerHandle::Get(),
        gin::IsolateHolder::IsolateType::kBlinkMainThread);

    // 构建GinShellRunner 用于执行文件脚本
    demo::DemoShellRunnerDelegate delegate;
    gin::ShellRunner runner(&delegate, instance.isolate());

    // 创建Handle容器
    {
      gin::Runner::Scope scope(&runner);
      runner.GetContextHolder()
          ->isolate()
          ->SetCaptureStackTraceForUncaughtExceptions(true);
    }

    // 执行脚本
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::BindOnce(Run, runner.GetWeakPtr()));
    base::RunLoop().RunUntilIdle();
  }

  // 结束线程池
  base::ThreadPoolInstance::Get()->Shutdown();

  return 0;
}
