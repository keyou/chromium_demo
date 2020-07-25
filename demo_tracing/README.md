
# demo_tracing

`vsync_auditor.html` 文件用于处理 `chrome://tracing` 页面上的 `Highlight VSync` 显示:

```c++
// third_party/catapult/tracing/tracing/extras/vsync/vsync_auditor.html

const VSYNC_SLICE_PRECISIONS = {
  // Android.
  'RenderWidgetHostViewAndroid::OnVSync': 5,
  // Android. Very precise. Requires "gfx" systrace category to be enabled.
  'VSYNC': 10,
  // Linux. Very precise. Requires "gpu" tracing category to be enabled.
  'vblank': 10,
  // Mac. Derived from a Mac callback (CVDisplayLinkSetOutputCallback).
  'DisplayLinkMac::GetVSyncParameters': 5
};

const BEGIN_FRAME_SLICE_PRECISION = {
  'DisplayScheduler::BeginFrame': 10
};
```

当使用以上这些字符串作为 trace name 的时候，`chrome://tracing` 可以显示帧率信息，比如使用以下任意一种 trace 就可以让自己的程序支持在`chrome://tracing`中显示 vsync。

```c++
TRACE_EVENT0("shell", "VSYNC");
TRACE_EVENT0("shell", "vblank");
// 这个是 chromium 内部使用的
TRACE_EVENT2("viz", "DisplayScheduler::BeginFrame", "args", args,"now", now);
```
