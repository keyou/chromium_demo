## Changelog


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