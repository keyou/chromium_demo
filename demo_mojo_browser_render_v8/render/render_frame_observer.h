
#pragma once
#include "content/public/renderer/render_frame_observer.h"

namespace content {
class RenderFrame;
}

namespace demo {
class DemoRenderFrameObserver : public content::RenderFrameObserver {
 public:
  class Delegate {
   public:
    virtual void DidCreateScriptContext(v8::Local<v8::Context> context,
                                        content::RenderFrame* render_frame) = 0;
    virtual void WillReleaseScriptContext(
        v8::Local<v8::Context> context,
        content::RenderFrame* render_frame) = 0;
  };

 public:
  DemoRenderFrameObserver() = delete;

  explicit DemoRenderFrameObserver(content::RenderFrame* render_frame,
                                      Delegate* delegate);
  ~DemoRenderFrameObserver() override;

  // 这里通过Delegate重写实现
  // 修改参数为v8::Context & RenderFrame
  void DidCreateScriptContext(v8::Local<v8::Context> context,
                              int32_t world_id) override;

  void WillReleaseScriptContext(v8::Local<v8::Context> context,
                                int32_t world_id) override;

  void OnDestruct() override;

 private:
  content::RenderFrame* render_frame_;
  Delegate* delegate_ = nullptr;
  DISALLOW_COPY_AND_ASSIGN(DemoRenderFrameObserver);
};
}  // namespace demo
