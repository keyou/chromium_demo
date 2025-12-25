#include "demo/demo_skia/win_software_bitmap_presenter.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"

namespace demo {

WinSoftwareBitmapPresenter::WinSoftwareBitmapPresenter(HWND hwnd)
    : hwnd_(hwnd) {}

WinSoftwareBitmapPresenter::~WinSoftwareBitmapPresenter() = default;

void WinSoftwareBitmapPresenter::Resize(const gfx::Size& size) {
  if (size_ == size) {
    return;
  }
  size_ = size;

  // 创建 DIB Section：这相当于 X11 里的 Shared Memory
  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = size.width();
  bmi.bmiHeader.biHeight = -size.height();  // Top-down
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  HDC hdc = GetDC(hwnd_);
  // h_bitmap_ 是 GDI 句柄，pixel_ptr_ 是 Skia 可以直接写的内存地址
  void* pptr;
  h_bitmap_ = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pptr, nullptr, 0);
  pixel_ptr_ = pptr;
  ReleaseDC(hwnd_, hdc);

  // 将这块内存包装成 SkCanvas
  SkImageInfo info = SkImageInfo::MakeN32Premul(size.width(), size.height());
  surface_ = SkSurfaces::WrapPixels(info, pixel_ptr_, info.minRowBytes());
}

SkCanvas* WinSoftwareBitmapPresenter::GetSkCanvas() {
  return surface_->getCanvas();
}

void WinSoftwareBitmapPresenter::EndPaint(const gfx::Rect& damage) {
  HDC hdc = GetDC(hwnd_);
  HDC mem_dc = CreateCompatibleDC(hdc);
  HGDIOBJ old_bitmap = SelectObject(mem_dc, h_bitmap_);

  // 这一步相当于 XShmPutImage，将像素拷贝到窗口
  BitBlt(hdc, damage.x(), damage.y(), damage.width(), damage.height(), mem_dc,
         damage.x(), damage.y(), SRCCOPY);

  SelectObject(mem_dc, old_bitmap);
  DeleteDC(mem_dc);
  ReleaseDC(hwnd_, hdc);
}

}  // namespace demo
