#include "base/command_line.h"
#include "base/logging.h"
#include "base/debug/stack_trace.h"

int main(int argc, char** argv) {
  // 使用log依赖它
  base::CommandLine::Init(argc, argv);

  int loglevel = -1;

  logging::SetLogPrefix("demo");
  logging::SetLogItems(true,false,true,false);
  // 设置为负数启用VLOG，在chromium中使用 --v=1 达到同样效果
  logging::SetMinLogLevel(loglevel);

  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG | logging::LOG_TO_STDERR;
  settings.log_file_path = nullptr;
  settings.lock_log = logging::DONT_LOCK_LOG_FILE;
  settings.delete_old = logging::APPEND_TO_OLD_LOG_FILE;
  bool logging_res = logging::InitLogging(settings);
  CHECK(logging_res);

  // 主动打印堆栈信息
  LOG(INFO) << "Stack: " << base::debug::StackTrace().ToString();

  LOG(INFO) << "set loglevel to: " << loglevel;

  LOG(INFO) << "LOG: INFO";
  DLOG(INFO) << "DLOG: INFO";

  LOG(WARNING) << "LOG: WARNING";
  LOG(ERROR) << "LOG: ERROR";

  LOG_IF(INFO,2>1) << "LOG_IF: INFO 2>1";
  DLOG_IF(INFO,2>1) << "DLOG_IF: INFO 2>1";

  // 不会打印出来，因为我们设置了LogLevel为 -1
  VLOG(2) << "VLOG: 2";
  VLOG(3) << "VLOG: 3";

  VLOG(1) << "VLOG: 1";
  DVLOG(1) << "DVLOG: 1";

  // 在日志后面追加 GetLastError() on Windows and errno on POSIX
  PLOG(INFO) << "PLOG: INFO";
  DPLOG(INFO) << "DPLOG: INFO";

  // 会使进程立即退出
  DLOG(FATAL) << "DLOG: FATAL";
  LOG(FATAL) << "LOG: FATAL";

  return 0;
}
