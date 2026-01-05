#ifndef DEMO_DEMO_SKIA_SKIA_CANVAS_SOFTWARE_H
#define DEMO_DEMO_SKIA_SKIA_CANVAS_SOFTWARE_H

#include "demo/demo_skia/skia_canvas.h"

#if defined(USE_X11)
#include "ui/base/x/x11_software_bitmap_presenter.h"
#endif  // defined(USE_X11)

#if defined(OS_WIN)
#include "demo/demo_skia/win_software_bitmap_presenter.h"
#endif  // defined (OS_WIN)

namespace demo {

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
#if defined(USE_X11)
  std::unique_ptr<ui::X11SoftwareBitmapPresenter> x11_presenter_;
#endif  // defined(USE_X11)
#if defined(OS_WIN)
  std::unique_ptr<WinSoftwareBitmapPresenter> win_presenter_;
#endif  // defined (OS_WIN)
};

}  // namespace demo

#endif // DEMO_DEMO_SKIA_SKIA_CANVAS_SOFTWARE_H
