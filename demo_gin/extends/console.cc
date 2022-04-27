#include "demo_gin/extends/console.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "gin/arguments.h"
#include "gin/converter.h"
#include "v8/include/v8-template.h"
namespace demo {

namespace {

void Log(const v8::FunctionCallbackInfo<v8::Value>& info) {
  gin::Arguments args(info);
  std::vector<std::string> messages;
  if (!args.GetRemaining(&messages)) {
    args.ThrowError();
    return;
  }
  printf("%s\n", base::JoinString(messages, " ").c_str());
}

}  // namespace

void Console::Register(v8::Isolate* isolate,
                       v8::Local<v8::ObjectTemplate> global_tmpl) {
  //  构建Log Function
  v8::Local<v8::FunctionTemplate> log_tmpl = v8::FunctionTemplate::New(
      isolate, Log, v8::Local<v8::Value>(), v8::Local<v8::Signature>(), 0,
      v8::ConstructorBehavior::kThrow);

  global_tmpl->Set(gin::StringToSymbol(isolate, "log"), log_tmpl);
}
}  // namespace demo