#include "demo/demo_mojo_v8/renderer/demo_render_frame_observer.h"
#include "demo/demo_mojo_v8/renderer/demo_v8_binding.h"
#include "content/public/renderer/render_frame.h"

namespace demo {
DemoRenderFrameObserver::DemoRenderFrameObserver(
    content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame),
      render_frame_(render_frame) {}

DemoRenderFrameObserver::~DemoRenderFrameObserver() {}

// content::RenderFrameObserver implementation
void DemoRenderFrameObserver::DidCreateScriptContext(
    v8::Local<v8::Context> context,
    int32_t world_id) {
  if (!render_frame_->IsMainFrame()) {
    return;
  }
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope scope(isolate);
  v8::Context::Scope context_scope(context);
  
  // 初始化我们的V8实现，并绑定到当前作用域下
  DemoV8Binding::Initialize(isolate, context, context->Global());
}

void DemoRenderFrameObserver::WillReleaseScriptContext(
    v8::Local<v8::Context> context,
    int32_t world_id) {}

void DemoRenderFrameObserver::OnDestruct() {
  delete this;
}
}  // namespace demo