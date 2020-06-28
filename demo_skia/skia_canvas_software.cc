
#include "demo/demo_skia/skia_canvas_software.h"

#include "base/bind.h"
#include "base/lazy_instance.h"

namespace demo_jni {

SkiaCanvasSoftware::SkiaCanvasSoftware(gfx::AcceleratedWidget widget,int width,int height)
    : SkiaCanvas(widget, width, height) {
  background_ = 0xFF00DE96;
  tag_ = "SkiaCanvasSoftware";
}

void SkiaCanvasSoftware::InitializeOnRenderThread() {
  x11_presenter_ = std::make_unique<ui::X11SoftwareBitmapPresenter>(
      nativeWindow_, render_task_runner_.get(), nullptr);

  // 当 format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM = 4
  // 时，一个像素占2个字节，所以x2
  // memset(buffer.bits, 0xAA, buffer.stride * buffer.height * 2);
  x11_presenter_->Resize(gfx::Size(width_, height_));
  auto canvas = BeginPaint();
  canvas->clear(background_);
  //canvas->drawCircle(20, 20, 20, circlePaint_);
  OnPaint(canvas);
  SwapBuffer();
  SkiaCanvas::InitializeOnRenderThread();
}

SkCanvas* SkiaCanvasSoftware::BeginPaint() {
  return x11_presenter_->GetSkCanvas();
}

void SkiaCanvasSoftware::OnPaint(SkCanvas* canvas) {
  x11_presenter_->EndPaint(gfx::Rect(width_,height_));
}

void SkiaCanvasSoftware::SwapBuffer() {
  x11_presenter_->OnSwapBuffers(base::BindOnce([](const gfx::Size&){}));
}

}  // namespace demo_jni