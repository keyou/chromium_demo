#include "demo/demo_mojo_browser_render_v8/browser/receiver_register.h"
#include "demo/demo_mojo_browser_render_v8/browser/demo_impl.h"

namespace demo {
void ReceiverRegister::RegisterAll(service_manager::BinderRegistry* registry) {
  // 调用DemoImpl的初始化方法
  DemoImpl::Initialize(registry);
}
}  // namespace demo