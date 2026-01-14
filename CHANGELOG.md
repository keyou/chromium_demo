## Changelog


### 2026.01.12
- Upgraded demo_views to M141 with the following main changes:
  - `views::WidgetDelegateView` is marked as `Deprecated` and PassKey is added to control usage, see [6441736](https://chromium-review.googlesource.com/c/chromium/src/+/6441736). This example directly inherits from the two subclasses according to the comments, and separates View and Delegate according to the design logic, binding the View instance through `SetContentsView()` inside Delegate.
  - Migrated `views::Background` to `ui::ColorVariant` with method renaming, see [6331510](https://chromium-review.googlesource.com/c/chromium/src/+/6331510)
  - Starting from [4518125](https://chromium-review.googlesource.com/c/chromium/src/+/4518125), Windows UI requires a thread pool initialized with MTA.
- Upgraded demo_linktest to M141 with Windows support.

### 2026.01.09
- Upgraded demo_gin to M141 with the following main changes:
  1. `v8::MicrotasksScope::MicrotasksScope(Isolate* isolate, Type type)` has been removed, see [5539888](https://chromium-review.googlesource.com/c/v8/v8/+/5539888)
  2. Moved the `extends` part as a separate `component` to avoid inconsistent `GIN_EXPORT` definition issues
  3. Added `v8::Isolate::Scope` in `AsyncAdd` to fix the issue where `v8::internal::g_current_isolate_` is null causing `v8::Integer::New` to trigger DCheck

### 2025.12.25
- Upgraded demo_skia to M141 on Windows platform (software only) with the following main changes:
  1. Added `WinSoftwareBitmapPresenter` to support presenting software rendered bitmaps on Windows platform using GDI API.
  2. Modified `SkiaCanvasSoftware` to utilize `WinSoftwareBitmapPresenter` when running on Windows.
  3. Updated build configuration to include necessary dependencies for Windows platform.
  4. Rename namespace `demo_jni` to `demo` in `demo_skia` module for consistency.

### 2025.12.24
- Upgraded demo_viz_offscreen to M141 with the following main changes:
  1. `SharedBitmapManager` has been removed from `FrameSinkManagerImpl::InitParams`, see [6180026](https://chromium-review.googlesource.com/c/chromium/src/+/6180026)
  2. `FrameRateDecider` has been removed, see [6515298](https://chromium-review.googlesource.com/c/chromium/src/+/6515298)
  3. Instantiation of `viz::Display` now requires `gpu::Scheduler`, see [5757160](https://chromium-review.googlesource.com/c/chromium/src/+/5757160)
- Documentation update: converted demo list to table format and added compatibility test status markers

### 2025.12.17
- Upgraded demo_viz_gui to M141 with the following main changes:
  1. `components/viz/common/resources/bitmap_allocation.h` has been removed
  2. `viz::TileDrawQuad::SetNew` removed the `is_premultiplied` parameter, see [6500701](https://chromium-review.googlesource.com/c/chromium/src/+/6500701)
  3. `viz::ContentDrawQuadBase::texture_size` has been removed, multiple quad types were modified, see [6653194](https://chromium-review.googlesource.com/c/chromium/src/+/6653194)
  4. `viz::TextureDrawQuad::SetNew` removed the `is_premultiplied`, `flipped`, and `opacity` parameters, see [6516061](https://chromium-review.googlesource.com/c/chromium/src/+/6516061), [6019939](https://chromium-review.googlesource.com/c/chromium/src/+/6019939), [5119685](https://chromium-review.googlesource.com/c/chromium/src/+/5119685)
  5. `SharedBitmap` has been deleted, see [6180918](https://chromium-review.googlesource.com/c/chromium/src/+/6180918). Normally, `SharedImageInterface` provided by `viz::RasterContextProvider` should be used instead; here we bypass using the low-level interface provided by `viz::SharedImageInterfaceProvider`
  6. The `frame_ack` parameter in `OnBeginFrame` of `RootFrameSink` has been removed, and a new pure virtual method `OnSurfaceEvicted()` has been added, see [6417183](https://chromium-review.googlesource.com/c/chromium/src/+/6417183), [4956873](https://chromium-review.googlesource.com/c/chromium/src/+/4956873)
  7. Fixed initialization on Windows according to commit [7230224](https://chromium-review.googlesource.com/c/chromium/src/+/7230224) to avoid crashes
  8. Fixed the `clip` parameter of `SharedQuadState`. This change has not yet found a corresponding upstream change; testing found that without this change, drawing content that exceeds the `clip` after transform will be discarded (made transparent) directly
  9. Since all resources are passed through `SharedImage`, the corresponding release logic is complex; after saving the trace, changed to directly force exit the process to simplify the implementation

### 2025.11.16
- Upgraded demo_gl to M141 with the following main changes:
  1. The return type of `ui::Event::type()` has changed to `ui::EventType`, keywords modified
  2. The `OnMouseEnter()` method of `ui::PlatformWindowDelegate` has been renamed to `OnCursorUpdate()`
  3. `//gpu` can no longer be externally visible; the required functionality in this case is concentrated in `//gpu/config`
  4. gl's `SharedContextState` requires surface to be off-screen; temporarily commented out the creation and use of `SharedContextState`

### 2025.11.11
- Upgraded core modules (log, task, memory, tracing, resources, mojo, mojo_v8) to M141 with the following changes:
  1. The `DISALLOW_COPY_AND_ASSIGN` macro is deprecated
  2. `base::MemoryPressureMonitor::GetCurrentPressureLevel()` now requires a `base::MemoryPressureMonitorTag` parameter
  3. mojo's `ReadData()` and `WriteData()` now use `std::span` to replace the previous buffer pointer + length parameters, and a new reference parameter has been added to indicate the actual amount of data read/written
  4. `v8::FunctionCallbackInfo::Holder()` is deprecated, use `This()` instead
  5. Resources `kDemoGenResources` array no longer provides length information
  6. Tracing fully enables Perfetto, traditional initialization methods have been removed; Startup initialization is now moved forward to execute `tracing::InitTracingPostFeatureList()` once after only initializing FeatureGating
- Added a new case `demo_mojo_child_process` under mojo, demonstrating direct inter-process communication between two child processes through parent process establishment
- ChangeLog split to a separate file

### 2024.6.23

- Add demo_task_thread, which demonstrates the use of a task in a new thread;

### 2023.12.24

Migrate some demos to M120, the upgrade is described below:

1. `ThreadTaskRunnerHandle` was replaced by `SingleThreadTaskRunner`;
2. The `TRACE_*` macro enables the perfetto client library by default;
3. mojo enables ipcz by default, and parameters must be passed for multi-process initialization;
4. The gl initialization interface supports selecting GPU;
5. The `SharedQuadState::SetAll` interface supports setting fast_rounded_corner;
6. `viz::TransferableResource` supports a clearer distinction between single-plane and multi-plane resources;
7. In order to reduce the number of viz PostTasks, `CompositorFrameSinkClient::OnBeginFrame` merges the functions of `ReclaimResources` and `DidReceiveCompositorFrameAck`;
8. Deleted the `SkEncodeImage` interface and replaced it with `SkXXXEncoder::Encode`;
9. Skia moved the DDL-related interfaces to chromium private and renamed them. The public DDL interface will no longer be provided in the future. After the change, only ganesh supports DrawDDL, and graphite does not.

### 2023.8.24

- Add demo_linktest, demonstrates incorrect use of gn causing link errors;

### 2023.3.25

- Migrate demo_skia to M110;

### 2023.3.16

- Migrate demo_views to M110;

### 2023.3.15

- Migrate demo_cc_offscreen to M110;
- Migrate demo_gin to M110;

### 2023.3.12

- Migrate demo_viz_gui to M110;

### 2023.3.11

- Update 110 branch to 5481;

### 2022.12.4

- Migrate some demo to M110;

### 2022.4.27

- Add demo_gin，demonstrate how to use gin to create a javascript runtime;

### 2021.9.13

- Add demo_mojo_v8，demonstrate how to use js in the render process to access the mojo interface provided by the browser process;

### 2021.9.6

- Migrate demo_views to M91；
- Migrate demo_cc_offscreen to M91；

### 2021.9.4

- Migrate demo_viz_offscreen to M91；

### 2021.9.3

- Migrate demo_ipc to M91;
- Migrate demo_services to M91;

### 2021.8.15

- Migrate demo_gl to M91;

### 2021.7.29

- Migrate below demos to M91：
    - demo_resources

### 2021.7.22

- Migrate below demos to M91：
    - demo_memory

### 2021.7.17

- Migrate below demos to M91：
    - demo_exe
    - demo_tracing_perfetto
    - demo_tracing_perfetto_content
    - demo_messageloop
    - demo_tasks
    - demo_mojo_single_process
    - demo_mojo_multiple_process
    - demo_mojo_multiple_process_binding
-  Rename below demos：
    - demo_messageloop to demo_task_executor
    - demo_tasks to demo_task_thread_pool

### 2020.8.10

- Add demo_gl, the demo uses the `//ui/gl` module to call GL for rendering;

### 2020.7.31

- demo_viz_layer_offscreen supports off-screen rendering using the SkiaOutputDeviceOffscreen interface;

### 2020.7.28

- Add demo_viz_layer_offscreen to demonstrate using VIZ's CopyOutput interface for off-screen rendering;

### 2020.7.24

- Add demo_viz_layer, demonstrate the use of VIZ for handwriting writing, and support the use of command lines to switch between software and hardware rendering;

### 2020.7.18

- Add demo_viz_gui_gpu to demonstrate the use of VIZ for hardware accelerated rendering and VIZ Client nesting;

### 2020.7.15

- Add demo_x11_glx and demo_x11_egl;

### 2020.7.11

- Add demo_x11 to demonstrate the use of X11 to create transparent windows;
- Update demo_skia, use software rendering by default, support transparent windows, GL rendering supports GL_RGB565 pixel format;

### 2020.6.28

- Add demo_skia to demonstrate the software/hardware rendering using Skia in Linux;

### 2020.6.4

- Update demo_android_skia, add multi-thread rendering and frame rate synchronization;

### 2020.5.31

- Add demo_android_skia to demonstrate the use of Skia in Android for software/hardware rendering;

### 2020.5.21

- Add demo_tracing_perfetto_content to demonstrate how the content module saves trace to a file, which can be used for chrome://tracing;
- Add demo_tracing documentation [demo_tracing](./demo_tracing/README.md);

### 2020.5.18

- Move demo_tracing to demo_tracing folder and rename it to demo_tracing_console, add Flush function;
- Add demo_tracing_perfetto to demonstrate the integration and use of trace and perfetto;

### 2020.4.29

- Add demo_cc_gui to demonstrate using `cc` to display GUI interface;

### 2020.4.17

- Added TRACE.txt of demo_cc to help understand the runtime behavior of cc;

### 2020.4.10

- Add demo_cc_offscreen, demonstrate the use of `cc` for off-screen rendering;

### 2020.4.6

- Add demo_viz_offscreen, demonstrate using `viz` for off-screen rendering;
- Modify demo_viz to demo_viz_gui, the function is unchanged;

### 2020.3.31

- Add demo_viz, demonstrate using `viz` module;
- Add documentation for `viz`: [viz](./demo_viz/README.md)

### 2020.3.21

- Add demo_views, demonstrate using `//ui/views` to develop UI;

### 2020.3.12

- demo_apk supports JNI to call instance method of C++ class;
- Add document: [browser startup process brief description] (./docs/startup.md)

### 2020.3.7

- demo_apk supports JNI;
- Add document: [demo_apk](./demo_android/README.md)

### 2020.3.4

- Add demo_tracing to demonstrate the use of trace;
- Add demo_apk to demonstrate how to use gn to create Android applications;
- Add demo_shell to demonstrate how to use the Content API to create a streamlined browser;

### Earlier

Add the following demo and related documents:

- demo_exe: the simplest demo, demonstrate gn and create your own exe;
- demo_log: demonstrate the use of log library;
- demo_tracing: demonstrate the use of Trace;
- demo_tasks: demonstrate the use of thread pool ThreadPool;
- demo_messageloop: demonstrate the use of message loop MessageLoop;
- demo_mojo_single_process: demonstrate the use of mojo library in a single process;
- demo_mojo_multiple_process: demonstrate the use of the mojo library in multiple processes;
- demo_mojo_multiple_process_binding: demonstrate the use of the binding layer of the mojo library in multiple processes;
- demo_services: demonstrate the use of mojo-based servcies and multi-process architecture;
- demo_ipc: demonstrate the use of mojo-based IPC interface;
- demo_memory: demonstrate the use of SharedMemory;