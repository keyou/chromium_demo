#pragma once

namespace v8 {
class Isolate;
}

namespace demo {
// V8绑定管理类
class V8BindingsRegister {
 public:
  static void RegisterAll(v8::Isolate* isolate);
 
};
}  // namespace  demo