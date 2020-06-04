# demo_android_skia

这个 demo 用于演示在 Android 中使用 skia 进行绘图，主要用来演示使用 skia 进行软件渲染和硬件渲染的性能差异，顺便演示 chromium 的渲染和 android 对接的方式。

在 demo 中主要演示了划线的功能，对比 chromium 中 canvas 的划线，这里演示的方法应该是 canvas 划线的性能上限，可以用来评估 chromium 渲染引入的性能损失情况。

- [gapid_opengles_trace.gfxtrace](./gapid_opengles_trace.gfxtrace) 是通过 [gapid](https://github.com/google/gapid) 抓到的 OpenGLES API 的调用数据。
- [systrace_without_vsync.html](./systrace_without_vsync.html) 是通过 [systrace](https://developer.android.com/topic/performance/tracing/command-line#app-trace) 抓到的 trace 数据，可以使用浏览器直接打开。
