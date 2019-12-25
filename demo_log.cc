#include <base/command_line.h>
#include <base/logging.h>

int main(int argc, char** argv) {
  base::CommandLine::Init(argc, argv);

  logging::SetLogPrefix("demo");
  logging::SetLogItems(true,false,true,false);
  logging::SetMinLogLevel(-1);

  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
  settings.log_file = nullptr;
  settings.lock_log = logging::DONT_LOCK_LOG_FILE;
  settings.delete_old = logging::APPEND_TO_OLD_LOG_FILE;
  bool logging_res = logging::InitLogging(settings);
  CHECK(logging_res);

  LOG(INFO) << "hello,demo!";
  VLOG(2) << "hello,vlog 2";
  VLOG(1) << "hello,vlog 1";

  return 0;
}
