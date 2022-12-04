#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/debug/stack_trace.h"
#include "base/logging.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_refptr.h"
#include "base/task/single_thread_task_executor.h"
#include "base/run_loop.h"
// #include "base/task/post_task.h"
#include "base/trace_event/common/trace_event_common.h"
#include "base/trace_event/trace_event.h"
#include "base/trace_event/trace_log.h"
// #include "components/tracing/common/trace_startup_config.h"
// #include "components/tracing/common/trace_to_console.h"
// #include "components/tracing/common/tracing_switches.h"

void TraceMe() {
  TRACE_EVENT0("test", "TraceMe");
  TRACE_EVENT1("test", "TraceMe", "value", 1);
  TRACE_EVENT2("test", "TraceMe", "value", 1, "value2", 2);
}

void TraceCount(int times) {
  TRACE_COUNTER1("test", "TraceCount", times);
}

int main(int argc, char** argv) {
  base::AtExitManager at_exit;
  base::CommandLine::Init(argc, argv);

  // 1. 创建配置文件对象；
  base::trace_event::TraceConfig trace_config;
  // 手动创建TraceConfig
  trace_config =
      base::trace_event::TraceConfig("test,testxxx", "trace-to-console");
  
  // 也可以使用component模块的方法来获取TraceConfig
  // 方法1 获取用于输出到控制台的TraceConfig
  // trace_config = tracing::GetConfigForTraceToConsole();
  // 方法2 获取用于追踪Startup的TraceConfig，输出到文件的功能不是base库提供的，而是content提供的
  // trace_config =
  // tracing::TraceStartupConfig::GetInstance()->GetTraceConfig();

  // 2. 启动Trace
  base::trace_event::TraceLog::GetInstance()->SetEnabled(
      trace_config, base::trace_event::TraceLog::RECORDING_MODE);

  // 3. 使用Trace
  // 第一个参数是category，并不可以随意写，否则会编译报错，test是测试用的category
  // 可以在base/trace_event/builtin_categories.h中添加新的category
  TRACE_EVENT0("test", "main");

  // 这里使用hack的方式避免修改builtin_categories.h文件：在category后面添加","
  TRACE_EVENT0("testxxx,", "main");
  //==============================================================
  // 以上宏展开后等价于下面的代码
  static_assert(
      base::trace_event::BuiltinCategories::IsAllowedCategory("testxxx,"),
      "Unknown tracing category is used. Please register your "
      "category in base/trace_event/builtin_categories.h");
  constexpr const unsigned char* trace_event_unique_k_category_group_enabled31 =
      base::trace_event::TraceLog::GetBuiltinCategoryEnabled("testxxx,");
  const unsigned char* trace_event_unique_category_group_enabled31;
  if (trace_event_unique_k_category_group_enabled31) {
    trace_event_unique_category_group_enabled31 =
        trace_event_unique_k_category_group_enabled31;
  } else {
    static base::subtle::AtomicWord trace_event_unique_atomic31 = 0;
    trace_event_unique_category_group_enabled31 =
        reinterpret_cast<const unsigned char*>(
            base::subtle::NoBarrier_Load(&(trace_event_unique_atomic31)));
    // __builtin_expect 用于编译优化
    if (__builtin_expect(!!(!trace_event_unique_category_group_enabled31), 0)) {
      trace_event_unique_category_group_enabled31 =
          base::trace_event::TraceLog::GetCategoryGroupEnabled("testxxx,");
      base::subtle::NoBarrier_Store(
          &(trace_event_unique_atomic31),
          (reinterpret_cast<base::subtle::AtomicWord>(
              trace_event_unique_category_group_enabled31)));
    };
  };
  ;
  trace_event_internal::ScopedTracer trace_event_unique_tracer31;
  if (__builtin_expect(
          !!(*trace_event_unique_category_group_enabled31 &
             (base::trace_event::TraceCategory::ENABLED_FOR_RECORDING |
              base::trace_event::TraceCategory::ENABLED_FOR_ETW_EXPORT |
              base::trace_event::TraceCategory::ENABLED_FOR_FILTERING)),
          0)) {
    // 在这里输出trace日志
    base::trace_event::TraceEventHandle h = trace_event_internal::AddTraceEvent(
        ('X'), trace_event_unique_category_group_enabled31, "main",
        trace_event_internal::kGlobalScope, trace_event_internal::kNoId,
        (static_cast<unsigned int>(0)), trace_event_internal::kNoId);
    trace_event_unique_tracer31.Initialize(
        trace_event_unique_category_group_enabled31, "main", h);
  };
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
