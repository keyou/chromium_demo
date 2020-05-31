
package org.demo.demo_jni;

import android.util.Log;
import android.view.Surface;

import java.lang.String;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;

@JNINamespace("demo_jni")
public class SkiaCanvas {

  public interface Callback {
    public void showInfo(String info);
  }

  private static final String TAG = "DemoAndroidSkia.SkiaCanvas";
  private long mNativeSkiaCanvas;
  private Callback mCallback;

  public SkiaCanvas(Surface surface,Callback callback, boolean useGL) {
    mCallback = callback;
    mNativeSkiaCanvas = SkiaCanvasJni.get().init(this, surface, useGL);
  }

  public void onTouch(int action,float x,float y){
    SkiaCanvasJni.get().onTouch(mNativeSkiaCanvas, action, x, y);
  }

  @CalledByNative
  public void showInfo(String info) {
    Log.i(TAG, "info: " + info);
    mCallback.showInfo(info);
  }

  // 固定格式，用于定义并生成JNI接口
  @NativeMethods
  public interface Natives {
    // 初始化，传入Java的this，返回C++的this
    long init(SkiaCanvas caller, Surface surface, boolean useGL);

    void onTouch(long nativeSkiaCanvas,int action, float x, float y);
  }
}