#include "demo_gin/extends/async_demo.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/thread_task_runner_handle.h"

#include "gin/arguments.h"
#include "gin/converter.h"
#include "v8/include/v8.h"

namespace demo {

namespace {

// 实际运行的Add计算的异步方法
void AsyncAdd(v8::Global<v8::Promise::Resolver> resolver,
              v8::Global<v8::Context> original_context,
              v8::Isolate* isolate,
              int a,
              int b) {
  // 创建微任务Scope
  v8::MicrotasksScope microtasks_scope(
      isolate, v8::MicrotasksScope::kDoNotRunMicrotasks);
  v8::HandleScope handle_scope(isolate);
  // 将Persistent 转为Local
  v8::Local<v8::Context> context = original_context.Get(isolate);
  //  真正执行计算
  int result = a + b;
  LOG(INFO) << "Add Arg1:" << a << " And Arg2:" << b << " Result:" << result;
  // 获取LocalResolver 并执行Resolve
  resolver.Get(isolate)
      ->Resolve(context, v8::Integer::New(isolate, result))
      .ToChecked();
}

// Add 方法 产生一个Promise
void Add(const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto* isolate = info.GetIsolate();
  auto context = isolate->GetCurrentContext();
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();

  // 获取参数
  gin::Arguments args(info);
  int a = 0;
  int b = 0;
  if (!args.GetNext(&a) || !args.GetNext(&b)) {
    args.ThrowError();
    return;
  }

  // 构建Persistent Resolver和Context
  // 保证他们的生命周期超过当前函数栈
  v8::Global<v8::Promise::Resolver> unique_resolver =
      v8::Global<v8::Promise::Resolver>(isolate, resolver);
  v8::Global<v8::Context> persisted_context =
      v8::Global<v8::Context>(isolate, context);

  // 返回Promise
  info.GetReturnValue().Set(resolver->GetPromise());

  // 执行异步计算
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::BindOnce(AsyncAdd, std::move(unique_resolver),
                                std::move(persisted_context), isolate, a, b));
}

}  // namespace

void AsyncDemo::Register(v8::Isolate* isolate,
                         v8::Local<v8::ObjectTemplate> global_tmpl) {
  // 构建AsyncDemo Object模板
  v8::Local<v8::ObjectTemplate> async_demo_tmpl =
      v8::ObjectTemplate::New(isolate);
  global_tmpl->Set(gin::StringToSymbol(isolate, "asyncDemo"), async_demo_tmpl);

  // 构建AsyncDemo Object Function模板
  v8::Local<v8::FunctionTemplate> add_tmpl = v8::FunctionTemplate::New(
      isolate, Add, v8::Local<v8::Value>(), v8::Local<v8::Signature>(), 0,
      v8::ConstructorBehavior::kThrow);

  async_demo_tmpl->Set(gin::StringToSymbol(isolate, "add"), add_tmpl);
}
}  // namespace demo