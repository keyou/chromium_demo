#include "demo/demo_mojo_browser_render_v8/render/demo_v8_binding.h"

#include "content/public/renderer/render_thread.h"
#include "demo/demo_mojo_browser_render_v8/v8/node_version.h"
#include "demo/demo_mojo_browser_render_v8/v8/node.h"


namespace {
// 构造函数(Class) 的名称
constexpr static const char* kClassName = "Demo";

inline demo::DemoV8Binding* GetSelfPointer(v8::Local<v8::Object> handle) {
  return node::ObjectWrap::Unwrap<demo::DemoV8Binding>(handle);
}

}  // namespace

namespace demo {

DemoV8Binding::DemoV8Binding() : weak_ptr_factory_(this) {
  BindRemote();
}
DemoV8Binding::~DemoV8Binding() {
  // Release Mojo
}

bool DemoV8Binding::Initialize(v8::Isolate* bind_isolate,
                               v8::Local<v8::Context> bind_context,
                               v8::Local<v8::Object> bind_parent_object) {
  v8::Local<v8::ObjectTemplate> binding_settings_tempalte =
      v8::ObjectTemplate::New(bind_isolate);
  binding_settings_tempalte->SetInternalFieldCount(1);
  v8::Local<v8::Object> object =
      binding_settings_tempalte->NewInstance(bind_context).ToLocalChecked();
  // constructor
  v8::Local<v8::FunctionTemplate> function_template =
      v8::FunctionTemplate::New(bind_isolate, V8New, object);
  function_template->SetClassName(
      v8::String::NewFromUtf8(bind_isolate, kClassName).ToLocalChecked());
  v8::Local<v8::ObjectTemplate> instance_template =
      function_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);

  // 进行属性绑定
  NODE_SET_PROTOTYPE_METHOD(function_template, "hello", V8Hello);

  v8::Local<v8::Function> constructor =
      function_template->GetFunction(bind_context).ToLocalChecked();
  object->SetInternalField(0, constructor);
  bind_parent_object
      ->Set(bind_context,
            v8::String::NewFromUtf8(bind_isolate, kClassName).ToLocalChecked(),
            constructor)
      .FromJust();
  return true;
}

void DemoV8Binding::V8New(const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();

  if (!info.IsConstructCall()) {
    isolate->ThrowException(v8::Exception::TypeError(
        v8::String::NewFromUtf8(isolate, "Must invoke by new !")
            .ToLocalChecked()));
    return;
  }

  DemoV8Binding* pointer = new DemoV8Binding();
  pointer->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void DemoV8Binding::V8Hello(const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  DCHECK(isolate);
  if ((info.Length() != 1) || !info[0]->IsString()) {
    isolate->ThrowException(v8::Exception::TypeError(
        v8::String::NewFromUtf8(isolate, "args number valid !")
            .ToLocalChecked()));
    return;
  }
  v8::String::Utf8Value utf8_value(isolate, info[0]);
  std::string utf8_string(*utf8_value);
  DemoV8Binding* self_pointer = GetSelfPointer(info.Holder());
  DCHECK(self_pointer);
  self_pointer->Hello(utf8_string);
}

void DemoV8Binding::Hello(const std::string& message) {
  remote_->Hello(message);
}

// 绑定Remote
bool DemoV8Binding::BindRemote() {
  if (remote_) {
    return false;
  }
  content::RenderThread::Get()->BindHostReceiver(
      remote_.BindNewPipeAndPassReceiver());
  remote_.set_disconnect_handler(base::BindOnce(
      &DemoV8Binding::MojomDisconnected, weak_ptr_factory_.GetWeakPtr()));

  LOG(INFO) << "Binding Call BindRemote";
  return true;
}

void DemoV8Binding::MojomDisconnected() {}

}  // namespace demo
