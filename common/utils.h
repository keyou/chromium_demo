#ifndef DEMO_COMMON_UTILS_H
#define DEMO_COMMON_UTILS_H

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/ref_counted_memory.h"
#include "base/strings/sys_string_conversions.h"
#include "base/threading/thread.h"
#include "base/trace_event/trace_buffer.h"

namespace demo {

std::unique_ptr<base::File>& trace_file() {
  static std::unique_ptr<base::File> g_trace_file;
  return g_trace_file;
}

void InitTrace(const std::string& file) {
#if defined(OS_WIN)
  trace_file() = std::make_unique<base::File>(
      base::FilePath(base::SysUTF8ToWide(file)), base::File::FLAG_OPEN_ALWAYS |
                                base::File::FLAG_WRITE);
#else
  trace_file() = std::make_unique<base::File>(
      base::FilePath(file), base::File::FLAG_OPEN_ALWAYS |
                                base::File::FLAG_WRITE |
                                base::File::FLAG_OPEN_TRUNCATED);
#endif
  DCHECK(trace_file()->IsValid());
  trace_file()->WriteAtCurrentPos("[", 1);
  DLOG(INFO) << "Init trace file: " << file;
}

void StartTrace(const std::string& categories = "",base::trace_event::TraceRecordMode mode = base::trace_event::RECORD_AS_MUCH_AS_POSSIBLE) {
  DLOG(INFO) << "Start trace: " << categories;
  static std::string categories_;
  if (!categories.empty())
    categories_ = categories;
  // 配置及启动 Trace
  base::trace_event::TraceConfig trace_config =
      base::trace_event::TraceConfig(categories_,mode);
  base::trace_event::TraceLog::GetInstance()->SetEnabled(
      trace_config, base::trace_event::TraceLog::RECORDING_MODE);
}

void FlushTrace(base::RepeatingClosure quit_closure) {
  DLOG(INFO) << "Flush trace begin.";

  base::trace_event::TraceLog::GetInstance()->SetDisabled();
  base::trace_event::TraceLog::GetInstance()->Flush(base::BindRepeating(
      [](base::OnceClosure quit_closure,
         const scoped_refptr<base::RefCountedString>& events_str,
         bool has_more_events) {
        // LOG(INFO) << std::endl << events_str->data();
        trace_file()->WriteAtCurrentPos(events_str->data().c_str(),
                                        events_str->size());
        trace_file()->WriteAtCurrentPos(",\n", 2);
        if (!has_more_events) {
          trace_file()->WriteAtCurrentPos("\n", 1);
          trace_file()->Flush();
          DLOG(INFO) << "Flush trace end.";
          if (quit_closure)
            std::move(quit_closure).Run();
        }
      },
      std::move(quit_closure)));
}

}  // namespace demo

#endif  // !DEMO_COMMON_UTILS_H
