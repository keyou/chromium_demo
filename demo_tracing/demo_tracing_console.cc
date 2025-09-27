#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_refptr.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/trace_event/trace_event.h"
#include "base/trace_event/trace_log.h"

// 宏展开工具宏定义 - 用于将宏最大化展开成字符串
#define STRINGIZE(x) #x
#define STRINGIZE_EXPANDED(x) STRINGIZE(x)
#define EXPAND(x) x
#define EXPAND_1(x) EXPAND(x)
#define EXPAND_2(x) EXPAND_1(EXPAND_1(x))
#define EXPAND_4(x) EXPAND_2(EXPAND_2(x))
#define EXPAND_8(x) EXPAND_4(EXPAND_4(x))
#define EXPAND_TO_STRING(x) STRINGIZE_EXPANDED(EXPAND_8(x))

void TraceMe() {
  TRACE_EVENT0("test", "TraceMe1");
  TRACE_EVENT1("test", "TraceMe2", "value", 1);
  TRACE_EVENT2("test", "TraceMe3", "value", 1, "value2", 2);
}

void TraceCount(int times) {
  TRACE_COUNTER1("test", "TraceCount", times);
}

int main(int argc, char** argv) {
  base::AtExitManager at_exit;
  base::CommandLine::Init(argc, argv);
  base::debug::EnableInProcessStackDumping();

  // 1. 创建配置文件对象；
  base::trace_event::TraceConfig trace_config;
  // 手动创建TraceConfig
  // M120 在 Linux/Android 已经默认启用 perfetto client，这种模式下
  // trace-to-console 和运行时捕获 trace 不兼容，所以这里改为
  // record-as-much-as-possible.
  trace_config =
      base::trace_event::TraceConfig("*", "record-as-much-as-possible");

  // M142: 必须初始化 perfetto，并设置为测试用途，原来的两种方法已不再有：
  //    方法1 获取用于输出到控制台的TraceConfig
  //    trace_config = tracing::GetConfigForTraceToConsole();
  //    方法2
  //    获取用于追踪Startup的TraceConfig，输出到文件的功能不是base库提供的，而是content提供的
  //    trace_config =
  //    tracing::TraceStartupConfig::GetInstance()->GetTraceConfig();

  base::trace_event::InitializeInProcessPerfettoBackend();
  base::trace_event::SetPerfettoInitializedForTesting();

  // 2. 启动Trace
  base::trace_event::TraceLog::GetInstance()->SetEnabled(trace_config);

  // 3. 使用Trace
  // 第一个参数是category，并不可以随意写，否则会编译报错，test是测试用的category
  // 可以在base/trace_event/builtin_categories.h中添加新的category
  TRACE_EVENT0("test", "main");

  // 这里使用hack的方式避免修改builtin_categories.h文件：在category后面添加","
  TRACE_EVENT0("testxxx,", "main");
  //==============================================================
  // 以上宏展开后等价于下面的代码 - 打到日志里
  LOG(INFO) << EXPAND_TO_STRING(TRACE_EVENT0("testxxx,", "main"));
  //==============================================================

  TraceMe();
  int i = 0;
  TraceCount(i++);
  TraceCount(i++);
  TraceCount(i++);

  base::SingleThreadTaskExecutor main_thread_task_executor;

  base::RunLoop run_loop;
  // 停止接收新的 Trace
  base::trace_event::TraceLog::GetInstance()->SetDisabled();
  // 获取 Trace 的结果，必须要先停止接收 Trace 才能执行 Flush
  base::trace_event::TraceLog::GetInstance()->Flush(base::BindRepeating(
      [](base::OnceClosure quit_closure,
         const scoped_refptr<base::RefCountedString>& events_str,
         bool has_more_events) {
        // 如果 Trace 的数据量比较大，会多次调用到这里，分批传输数据
        // 把这些数据包装进 [...] 或者 {"traceEvents":[ ... ]} 就可以使用 TraceViewer 来查看了
        // 可以参考 base/test/trace_to_file.cc
        // Trace 文件格式的详细信息见: https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit#
        const char* header = "{\"traceEvents\":[\n";
        LOG(INFO) << "result=\n" << header << events_str->data() << "\n]}";
        if(!has_more_events)
          std::move(quit_closure).Run();
      },
      run_loop.QuitClosure()));
  run_loop.Run();

  return 0;
}
