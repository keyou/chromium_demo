#include "demo/demo_mojo_browser_render_v8/render/render_frame_observer.h"
#include "content/public/renderer/render_frame.h"

namespace demo {
DemoRenderFrameObserver::DemoRenderFrameObserver(
    content::RenderFrame* render_frame,
    Delegate* delegate)
    : content::RenderFrameObserver(render_frame),
      render_frame_(render_frame),
      delegate_(delegate) {}

DemoRenderFrameObserver::~DemoRenderFrameObserver() {}

void DemoRenderFrameObserver::DidCreateScriptContext(
    v8::Local<v8::Context> context,
    int32_t world_id) {
  if (delegate_) {
    delegate_->DidCreateScriptContext(context, render_frame_);
  }
}

void DemoRenderFrameObserver::WillReleaseScriptContext(
    v8::Local<v8::Context> context,
    int32_t world_id) {
  if (delegate_) {
    delegate_->WillReleaseScriptContext(context, render_frame_);
  }
}

void DemoRenderFrameObserver::OnDestruct() {
  delete this;
}
}  // namespace demo