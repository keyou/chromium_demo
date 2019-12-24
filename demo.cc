#include <base/logging.h>

int main(int argc, char** argv) {
  LOG(INFO) << "hello,demo!";
  !((::logging::ShouldCreateLogMessage(::logging::LOG_INFO)))
      ? (void)0
      : ::logging::LogMessageVoidify() &
            (::logging::LogMessage("demo/demo.cc", 5, ::logging::LOG_INFO)
                 .stream())
                << "hello,demo!";
  ::logging::LogMessageVoidify() &
      (::logging::LogMessage("demo/demo.cc", 5, ::logging::LOG_INFO).stream())
          << "hello,demo!";
  ::logging::LogMessage("demo/demo.cc", 5, ::logging::LOG_VERBOSE).stream()
      << "hello,demo!";

  return 0;
}
