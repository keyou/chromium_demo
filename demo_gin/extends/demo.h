#pragma once

#include "gin/gin_export.h"
#include "v8/include/v8-forward.h"

namespace demo {
// 提供Add方法的类
class GIN_EXPORT Demo {
 public:
  // 注册对应的方法和对象
  static void Register(v8::Isolate* isolate,
                       v8::Local<v8::ObjectTemplate> templ);
};
}  // namespace demo