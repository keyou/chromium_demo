#ifndef DEMO_DEMO_SKIA_SKIA_CANVAS_SOFTWARE_H
#define DEMO_DEMO_SKIA_SKIA_CANVAS_SOFTWARE_H

#include "demo/demo_skia/skia_canvas.h"

#include "ui/base/x/x11_software_bitmap_presenter.h"

namespace demo_jni {

class SkiaCanvasSoftware : public SkiaCanvas {
 public:
  SkiaCanvasSoftware(gfx::AcceleratedWidget widget,int width,int height);
  ~SkiaCanvasSoftware() override;
  void InitializeOnRenderThread() override;
  void Resize(int width, int height) override;
  void OnPaint(SkCanvas* canvas) override;

 private:
  SkCanvas* BeginPaint() override;
  void SwapBuffer() override;

  std::unique_ptr<ui::X11SoftwareBitmapPresenter> x11_presenter_;
};

} // namespace demo_jni

#endif // DEMO_DEMO_SKIA_SKIA_CANVAS_SOFTWARE_H
