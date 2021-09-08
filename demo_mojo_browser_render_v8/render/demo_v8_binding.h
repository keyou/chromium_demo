#pragma once

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/sequenced_task_runner.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "v8/include/v8.h"

#include "demo/demo_mojo_browser_render_v8/mojom/demo.mojom.h"
#include "demo/demo_mojo_browser_render_v8/v8/node_object_wrap.h"

namespace demo {

// 这是一个V8对象
class DemoV8Binding : public node::ObjectWrap {
 public:
  static bool Initialize(v8::Isolate* bind_isolate,
                         v8::Local<v8::Context> bind_context,
                         v8::Local<v8::Object> bind_parent_object);

 private:
  static void V8New(const v8::FunctionCallbackInfo<v8::Value>& info);
  static void V8Hello(const v8::FunctionCallbackInfo<v8::Value>& info);

 private:
  explicit DemoV8Binding();
  ~DemoV8Binding() override;

  void Hello(const std::string& name);
  bool BindRemote();
  void MojomDisconnected();

 private:
  SEQUENCE_CHECKER(sequence_checker_);
  mojo::Remote<demo::mojom::Demo> remote_;
  base::WeakPtrFactory<DemoV8Binding> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(DemoV8Binding);
};
}  // namespace demo