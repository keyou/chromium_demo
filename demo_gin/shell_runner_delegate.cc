#include "demo_gin/shell_runner_delegate.h"
#include "base/logging.h"
#include "gin/object_template_builder.h"
#include "demo_gin/extends/console.h"
#include "demo_gin/extends/demo.h"
namespace demo {

v8::Local<v8::ObjectTemplate> DemoShellRunnerDelegate::GetGlobalTemplate(
    gin::ShellRunner* runner,
    v8::Isolate* isolate) {
  // 构建GlobalThis对象
  v8::Local<v8::ObjectTemplate> global_tmpl =
      gin::ObjectTemplateBuilder(isolate).Build();

  // 注册Console
  demo::Console::Register(isolate, global_tmpl);

  // 注册Demo
  demo::Demo::Register(isolate, global_tmpl);

  return global_tmpl;
}

void DemoShellRunnerDelegate::UnhandledException(gin::ShellRunner* runner,
                                                 gin::TryCatch& try_catch) {
  LOG(ERROR) << try_catch.GetStackTrace();
}
}  // namespace demo