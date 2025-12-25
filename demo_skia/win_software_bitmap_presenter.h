#ifndef DEMO_DEMO_SKIA_WIN_SOFTWARE_BITMAP_PRESENTER_H_
#define DEMO_DEMO_SKIA_WIN_SOFTWARE_BITMAP_PRESENTER_H_

#include <windows.h>

#include "base/memory/raw_ptr.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

// 一个帮助类，使用 GDI 在 Windows 上呈现 Skia 软件位图
// （很 Legacy 的方案，仅演示思路）
class WinSoftwareBitmapPresenter {
 public:
  explicit WinSoftwareBitmapPresenter(HWND hwnd);
  ~WinSoftwareBitmapPresenter();

  void Resize(const gfx::Size& size);
  SkCanvas* GetSkCanvas();
  void EndPaint(const gfx::Rect& damage);

 private:
  HWND hwnd_;
  gfx::Size size_;
  HBITMAP h_bitmap_ = nullptr;
  base::raw_ptr<void> pixel_ptr_ = nullptr;
  sk_sp<SkSurface> surface_;
};

#endif  // DEMO_DEMO_SKIA_WIN_SOFTWARE_BITMAP_PRESENTER_H_
