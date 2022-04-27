#include "demo_gin/extends/demo.h"
#include "base/strings/string_util.h"
#include "gin/arguments.h"
#include "gin/converter.h"
#include "v8/include/v8-template.h"

#include "base/logging.h"

namespace demo {

namespace {

// 实际运行的Add方法
void Add(const v8::FunctionCallbackInfo<v8::Value>& info) {
  gin::Arguments args(info);
  int a = 0;
  int b = 0;
  if (!args.GetNext(&a) || !args.GetNext(&b)) {
    args.ThrowError();
    return;
  }

  LOG(INFO) << "Add Arg1:" << a << " And Arg2:" << b << " Result:" << a + b;

  info.GetReturnValue().Set(v8::Integer::New(info.GetIsolate(), a + b));
}

}  // namespace

void Demo::Register(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate> global_tmpl) {
  // 构建Demo Object模板
  v8::Local<v8::ObjectTemplate> demo_tmpl = v8::ObjectTemplate::New(isolate);
  global_tmpl->Set(gin::StringToSymbol(isolate, "demo"), demo_tmpl);

  // 构建Demo Object Function模板
  v8::Local<v8::FunctionTemplate> add_tmpl = v8::FunctionTemplate::New(
      isolate, Add, v8::Local<v8::Value>(), v8::Local<v8::Signature>(), 0,
      v8::ConstructorBehavior::kThrow);

  demo_tmpl->Set(gin::StringToSymbol(isolate, "add"), add_tmpl);
}
}  // namespace demo