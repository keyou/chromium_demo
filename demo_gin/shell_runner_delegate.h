#pragma once
#include "gin/shell_runner.h"
#include "gin/try_catch.h"

namespace demo {

// 实现我们自己的ShellRunnerDelegate
// 主要用于注册我们自己提供的方法
class DemoShellRunnerDelegate : public gin::ShellRunnerDelegate {
 public:
  DemoShellRunnerDelegate() = default;
  DemoShellRunnerDelegate(const DemoShellRunnerDelegate&) = delete;
  DemoShellRunnerDelegate& operator=(const DemoShellRunnerDelegate&) = delete;

  // 获取GlobalThis对象
  v8::Local<v8::ObjectTemplate> GetGlobalTemplate(
      gin::ShellRunner* runner,
      v8::Isolate* isolate) override;

  // 处理异常
  void UnhandledException(gin::ShellRunner* runner,
                          gin::TryCatch& try_catch) override;
};
}  // namespace demo