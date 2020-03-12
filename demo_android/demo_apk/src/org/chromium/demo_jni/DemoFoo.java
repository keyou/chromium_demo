
package org.chromium.demo_jni;

import android.util.Log;
import java.lang.String;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;

@JNINamespace("demo_jni")
public class DemoFoo {

    private static final String TAG = "DemoApk.DemoFoo";
    private long mNativeDemoFoo;

    public DemoFoo() {
        Log.i(TAG,"Call DemoFoo.ctor");
        mNativeDemoFoo = DemoFooJni.get().init(this);
    }

    public void HelloDemoFoo(String who) {
        Log.i(TAG,"Call HelloDemoFoo");
        DemoFoo.Natives foo = DemoFooJni.get();
        foo.HelloDemoFoo(mNativeDemoFoo,who);
    }

    public void Hello(String who) {
        // DemoFooJni 是根据DemoFoo.Natives自动生成的类
        Log.i(TAG,"Call Hello");
        DemoFooJni.get().Hello(who);
    }

    // C++和Java都可以调用该方法
    // 演示实例方法的使用
    @CalledByNative
    public void Hi(String who) {
        Log.i(TAG,"Hi,"+who);
    }

    // C++和Java都可以调用该方法
    // 演示static方法的使用
    @CalledByNative
    public static void HiStatic(String who) {
       Log.i(TAG,"Hi Static,"+who);
    }

    // 固定格式，用于定义并生成JNI接口
    @NativeMethods
    public interface Natives {
        // 初始化，传入Java的this，返回C++的this
        long init(DemoFoo caller);
        // 第一个参数以 native 开头，因此调用nativeDemoFoo对象的实例方法
        void HelloDemoFoo(long nativeDemoFoo,String who);
        // 第一个参数不是以 native 开头，因此调用全局方法
        void Hello(String who);

    }
}