#pragma once

#include "base/logging.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "services/service_manager/public/cpp/binder_registry.h"

#include "demo/demo_mojo_v8/mojom/demo.mojom.h"
namespace demo {

// 实现Demo Mojom
class DemoImpl : public demo::mojom::Demo {
 public:
  DemoImpl();
  ~DemoImpl() override;

  // 初始化Receiver并注册到BinderRegisty中
  static bool Initialize(service_manager::BinderRegistry* registry);

  // 实现Mojo定义
  void Hello(const std::string& name) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(DemoImpl);
};
}  // namespace demo
