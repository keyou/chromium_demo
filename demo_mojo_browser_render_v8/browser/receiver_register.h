#pragma once
#include "services/service_manager/public/cpp/binder_registry.h"

namespace demo {
/// 用于代理注册我们走动哟Mojo的接收器
class ReceiverRegister {
 public:
  // 注册我们所有的自定义Mojo
  static void RegisterAll(service_manager::BinderRegistry* registry);
};
}  // namespace demo