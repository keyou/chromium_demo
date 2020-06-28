#ifndef DEMO_DEMO_SKIA_SKIA_CANVAS_H
#define DEMO_DEMO_SKIA_SKIA_CANVAS_H

#include "base/threading/thread.h"
#include "base/sequenced_task_runner.h"

#include "ui/gfx/native_widget_types.h"

#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkSurface.h"

#include "base/timer/timer.h"

namespace demo_jni {

class SkiaCanvas {
 public:
  void OnTouch(int action, float x, float y);

 protected:
  SkiaCanvas(gfx::AcceleratedWidget widget,int width,int height);
  virtual void InitializeOnRenderThread();
  virtual SkCanvas* BeginPaint() = 0;
  virtual void OnPaint(SkCanvas* canvas) {}
  virtual void SwapBuffer() = 0;
  void SetNeedsRedraw(bool need_redraw);
  void ShowInfo(std::string info);

  gfx::AcceleratedWidget nativeWindow_;
  int width_;
  int height_;
  base::Thread render_thread_;
  scoped_refptr<base::SequencedTaskRunner> render_task_runner_;

  sk_sp<SkSurface> skSurface_ = nullptr;
  SkPaint pathPaint_;
  SkPaint circlePaint_;
  SkPath skPath_;
  SkScalar strokeWidth_ = 5.f;
  SkColor background_ = SK_ColorBLACK;
  std::string tag_;
  bool need_redraw_ = false;
  bool is_drawing_ = false;
  base::TimeDelta total_frame_time_;
  base::TimeTicks last_frame_time_;
  unsigned int frame_count_ = 0;
  base::TimeDelta total_paint_time_;
  base::TimeDelta total_swap_time_;
  
  unsigned int touch_count_ = 0;
  base::TimeTicks touch_start_time_;
  base::TimeDelta total_touch_time_;

 private:
  void OnTouchOnRenderThread(int action, float x, float y);
  void OnRenderOnRenderThread();
  void ShowFrameRateOnRenderThread();
  base::RepeatingTimer timer_;
  base::RepeatingClosure render_closure_;
  base::WeakPtrFactory<SkiaCanvas> weak_factory_{this};
};

}  // namespace demo_jni

#endif  // DEMO_DEMO_SKIA_SKIA_CANVAS_H