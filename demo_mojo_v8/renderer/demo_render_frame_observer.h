
#pragma once
#include "content/public/renderer/render_frame_observer.h"

namespace content {
class RenderFrame;
}

namespace demo {
class DemoRenderFrameObserver : public content::RenderFrameObserver {
 public:
  DemoRenderFrameObserver() = delete;
  DemoRenderFrameObserver(const DemoRenderFrameObserver&) = delete;
  DemoRenderFrameObserver& operator=(const DemoRenderFrameObserver&) = delete;

  explicit DemoRenderFrameObserver(content::RenderFrame* render_frame);
  ~DemoRenderFrameObserver() override;

  // content::RenderFrameObserver override
  void DidCreateScriptContext(v8::Local<v8::Context> context,
                              int32_t world_id) override;

  void WillReleaseScriptContext(v8::Local<v8::Context> context,
                                int32_t world_id) override;

  void OnDestruct() override;

 private:
  content::RenderFrame* render_frame_;
};
}  // namespace demo
