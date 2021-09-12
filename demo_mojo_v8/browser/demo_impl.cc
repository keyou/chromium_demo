#include "demo/demo_mojo_v8/browser/demo_impl.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

namespace demo {
namespace {
void BindMojomImpl(mojo::PendingReceiver<demo::mojom::Demo> receiver) {
  mojo::MakeSelfOwnedReceiver(base::WrapUnique(new DemoImpl),
                              std::move(receiver));
}
} // namespace
// 初始化，向BinderRegistry注册当前实例的工厂
bool DemoImpl::Initialize(service_manager::BinderRegistry* registry) {
  registry->AddInterface(base::BindRepeating(&BindMojomImpl),
                         base::SequencedTaskRunnerHandle::Get());
  return true;
}

DemoImpl::DemoImpl() {}
DemoImpl::~DemoImpl() {}

// 实现Hello
void DemoImpl::Hello(const std::string& who) {
  LOG(INFO) << "DemoImpl Call Hello:" << who;
}
}  // namespace demo