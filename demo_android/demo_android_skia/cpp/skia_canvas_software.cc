
#include "demo/demo_android/demo_android_skia/cpp/skia_canvas_software.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/bind.h"
#include "base/lazy_instance.h"

namespace demo_jni {

SkiaCanvasSoftware::SkiaCanvasSoftware(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    const base::android::JavaParamRef<jobject>& jsurface)
    : SkiaCanvas(env, caller, jsurface) {
  background_ = 0xFF00DE96;
  tag_ = "SkiaCanvasSoftware";

  ANativeWindow_setBuffersGeometry(nativeWindow_, width_, height_,
                                   WINDOW_FORMAT_RGB_565);

  // 当 format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM = 4
  // 时，一个像素占2个字节，所以x2
  // memset(buffer.bits, 0xAA, buffer.stride * buffer.height * 2);
  
  auto canvas = BeginPaint();
  canvas->clear(background_);
  //canvas->drawCircle(20, 20, 20, circlePaint_);
  SwapBuffer();
}

SkCanvas* SkiaCanvasSoftware::BeginPaint() {
  ANativeWindow_Buffer buffer;
  DCHECK(ANativeWindow_lock(nativeWindow_, &buffer, &dirtyRect_)==0);
  SkImageInfo info = SkImageInfo::Make(width_, height_, kRGB_565_SkColorType,
                                       kPremul_SkAlphaType, nullptr);
  skSurface_ = SkSurface::MakeRasterDirect(info, buffer.bits, buffer.stride * 2,
                                           nullptr);
  return skSurface_->getCanvas();
}

void SkiaCanvasSoftware::SwapBuffer() {
  skSurface_->flush();
  ANativeWindow_unlockAndPost(nativeWindow_);
}

}  // namespace demo_jni