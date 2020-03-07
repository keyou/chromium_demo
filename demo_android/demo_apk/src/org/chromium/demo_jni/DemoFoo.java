
package org.chromium.demo_jni;

import android.util.Log;
import java.lang.String;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;

@JNINamespace("demo_jni")
public class DemoFoo {

    private static final String TAG = "DemoApk.DemoFoo";

    public DemoFoo() {
        Log.i(TAG,"Call DemoFoo.ctor");
        DemoFooJni.get().init(this);
    }

    public void Hello(String who) {
        // DemoFooJni 是根据DemoFoo.Natives自动生成的类
        Log.i(TAG,"Call Hello");
        DemoFoo.Natives foo = DemoFooJni.get();
        foo.Hello(who);
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
        void init(Object foo);
        void Hello(String who);
    }
}