
#include "demo/demo_android/demo_android_skia/cpp/skia_canvas_software.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/bind.h"
#include "base/lazy_instance.h"

namespace demo_jni {

namespace {
  
std::string ToString(const ANativeWindow_Buffer& buffer,const ARect& rect) {
    std::stringstream ss;
    ss  << "[SkiaCanvasSoftware] "
        << "format=" << buffer.format 
        << ", width=" << buffer.width
        << ", height=" << buffer.height 
        << ", stride=" << buffer.stride
        << ", dirtyRect=" << rect.left << "," << rect.top << ","
        << rect.right << "," << rect.bottom;
    return ss.str();
}

}

SkiaCanvasSoftware::SkiaCanvasSoftware(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    const base::android::JavaParamRef<jobject>& surface)
    : SkiaCanvas(env, caller, surface) {
  SkImageInfo info =
        SkImageInfo::Make(width_, height_, kRGB_565_SkColorType,
                          kPremul_SkAlphaType, nullptr);
  ANativeWindow_setBuffersGeometry(nativeWindow_, width_, height_,
                                   WINDOW_FORMAT_RGB_565);
  ANativeWindow_Buffer buffer;
  if (ANativeWindow_lock(nativeWindow_, &buffer, &dirtyRect_) == 0) {
    ShowInfo(ToString(buffer,dirtyRect_));
    // 当 format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM = 4
    // 时，一个像素占2个字节，所以x2
    // memset(buffer.bits, 0xAA, buffer.stride * buffer.height * 2);
    SkImageInfo info =
        SkImageInfo::Make(width_, height_, kRGB_565_SkColorType,
                          kPremul_SkAlphaType, nullptr);
    skSurface_ = SkSurface::MakeRaster(info, buffer.stride * 2, nullptr);
    skCanvas_ = skSurface_->getCanvas();
    skCanvas_->drawColor(0xFF00DE96);
    skCanvas_->drawCircle(50, 50, 50, circlePaint_);
    skSurface_->flush();

    auto surface = SkSurface::MakeRasterDirect(
        info, buffer.bits, buffer.stride * 2, nullptr);
    auto canvas = surface->getCanvas();
    skSurface_->draw(canvas, 0, 0, nullptr);
    surface->flush();
    ANativeWindow_unlockAndPost(nativeWindow_);
  }
}

void SkiaCanvasSoftware::OnTouch(JNIEnv* env, int action, jfloat x, jfloat y) {
  DLOG(INFO) << "[SkiaCanvasSoftware] OnTouch: action,x,y=" << action << "," << x
             << "," << y;
  ANativeWindow_Buffer buffer;
  if (ANativeWindow_lock(nativeWindow_, &buffer, &dirtyRect_) == 0) {
    ShowInfo(ToString(buffer,dirtyRect_));
    if(action == 0) { //down
      skPath_.moveTo(x, y);
    } else if (action == 2 || action == 1) {  // move or up
      skPath_.lineTo(x, y);
      skCanvas_->drawPath(skPath_, pathPaint_);
    }
    //skCanvas_->drawCircle(x, y, 10, circlePaint_);
    skSurface_->flush();

    SkImageInfo info = SkImageInfo::Make(width_, height_, kRGB_565_SkColorType,
                                         kPremul_SkAlphaType, nullptr);
    auto surface = SkSurface::MakeRasterDirect(
        info, buffer.bits, buffer.stride * 2, nullptr);
    auto canvas = surface->getCanvas();
    skSurface_->draw(canvas, 0, 0, nullptr);
    surface->flush();
    ANativeWindow_unlockAndPost(nativeWindow_);
  }
}

}  // namespace demo_jni