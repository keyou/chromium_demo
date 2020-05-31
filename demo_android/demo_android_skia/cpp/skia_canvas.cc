
#include "demo/demo_android/demo_android_skia/cpp/skia_canvas.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/bind.h"
#include "base/lazy_instance.h"
#include "demo/demo_android/demo_android_skia/cpp/skia_canvas_software.h"
#include "demo/demo_android/demo_android_skia/cpp/skia_canvas_gl.h"
#include "demo/demo_android/demo_android_skia/demo_android_skia_jni_headers/SkiaCanvas_jni.h"

namespace demo_jni {

static jlong JNI_SkiaCanvas_Init(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    const base::android::JavaParamRef<jobject>& surface,
    jboolean useGL) {
  DLOG(INFO) << "[demo_android_skia] JNI_SkiaCanvas_Init";
  if(useGL)
    return reinterpret_cast<intptr_t>(new SkiaCanvasGL(env, caller, surface));
  return reinterpret_cast<intptr_t>(new SkiaCanvasSoftware(env, caller, surface));
}

SkiaCanvas::SkiaCanvas(JNIEnv* env,
                       const base::android::JavaParamRef<jobject>& caller,
                       const base::android::JavaParamRef<jobject>& surface)
    : caller_(caller),
      nativeWindow_(ANativeWindow_fromSurface(env, surface.obj())),
      width_(ANativeWindow_getWidth(nativeWindow_)),
      height_(ANativeWindow_getHeight(nativeWindow_)) {
  DCHECK(nativeWindow_);
  circlePaint_.setAntiAlias(false);
  circlePaint_.setColor(SK_ColorRED);
  pathPaint_.setAntiAlias(false);
  pathPaint_.setColor(SK_ColorWHITE);
  pathPaint_.setStyle(SkPaint::kStroke_Style);
  pathPaint_.setStrokeWidth(strokeWidth_);
}

void SkiaCanvas::ShowInfo(std::string info) {
  DLOG(INFO) << "[demo_android_skia] Info: " << info;
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_SkiaCanvas_showInfo(env,caller_,base::android::ConvertUTF8ToJavaString(env,info));
}

}  // namespace demo_jni