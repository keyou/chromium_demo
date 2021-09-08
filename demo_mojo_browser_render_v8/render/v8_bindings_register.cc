#include "demo/demo_mojo_browser_render_v8/render/v8_bindings_register.h"
#include "demo/demo_mojo_browser_render_v8/render/demo_v8_binding.h"

namespace demo {

void V8BindingsRegister::RegisterAll(v8::Isolate* isolate) {
  v8::Local<v8::Context> current_context = isolate->GetCurrentContext();
  // 初始化我们的V8实现，并绑定到当前作用域下
  DemoV8Binding::Initialize(isolate, current_context,
                            current_context->Global());
}
}  // namespace demo