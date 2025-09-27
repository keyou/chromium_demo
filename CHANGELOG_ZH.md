## 更新日志

### 2024.6.23

- 添加 demo_task_thread, 演示在新线程中使用 task, 位于 demo_task 下;

### 2023.12.24

- 升级 demos 到 chromium 120，升级内容如下：
  1. `ThreadTaskRunnerHandle` 被 `SingleThreadTaskRunner` 取代；
  2. `TRACE_*` 宏默认启用 perfetto client 库；
  3. mojo 默认启用了 ipcz，多进程初始化必须传参；
  4. gl 初始化接口支持选择 GPU；
  5. `SharedQuadState::SetAll` 接口支持设置 fast_rounded_corner;
  6. `viz::TransferableResource` 支持更明确的区分单 plane 和多 plane 资源；
  7. 为了减少 viz PostTask 的数量，`CompositorFrameSinkClient::OnBeginFrame` 合并了 `ReclaimResources` 和 `DidReceiveCompositorFrameAck` 的功能；
  8. 删除了 `SkEncodeImage` 接口，使用 `SkXXXEncoder::Encode` 替代；
  9. skia 将 DDL 相关接口移动到 chromium private 内，并进行了重命名，后续不再提供 public 的 DDL 接口，变动后只有 ganesh 支持 DrawDDL，graphite 不支持。

### 2023.8.24

- 添加 demo_linktest，演示错误的使用 gn 导致链接错误；

### 2023.3.25

- 升级 demo_skia 到 110 版本；

### 2023.3.16

- 升级 demo_views 到 110 版本；

### 2023.3.15

- 升级 demo_cc_offscreen 到 110 版本；
- 升级 demo_gin 到 110 版本；

### 2023.3.12

- 升级 demo_viz_gui 到 110 版本；

### 2023.3.11

- 更新 110 版本的分支到 chromium 正式分支 5481;

### 2022.12.4

- 升级一些 demo 到 110 版本；

### 2022.04.27

- 添加 demo_gin，演示如何使用gin创建一个简单的js运行时。

### 2021.9.13

- 添加 demo_mojo_v8，演示如何在 render 进程中使用 js 访问 browser 进程提供的 mojo 接口；

### 2021.9.6

- 升级 demo_views 到 M91 版本；
- 升级 demo_cc_offscreen 到 M91 版本；

### 2021.9.4

- 升级 demo_viz_offscreen 到 M91 版本；

### 2021.9.3

- 升级 demo_ipc 到 M91 版本；
- 升级 demo_services 到 M91 版本；

### 2021.8.15

- 升级 demo_gl 到 M91 版本；

### 2021.7.29

- 升级以下 demo 到 M91 版本：
    - demo_resources

### 2021.7.22

- 升级以下 demo 到 M91 版本：
    - demo_memory

### 2021.7.17

- 升级以下 demo 到 M91 版本：
    - demo_exe
    - demo_tracing_perfetto
    - demo_tracing_perfetto_content
    - demo_messageloop （改名为 demo_task_executor）
    - demo_tasks （改名为 demo_task_thread_pool）
    - demo_mojo_single_process
    - demo_mojo_multiple_process
    - demo_mojo_multiple_process_binding
-  重命名以下 demo：
    - demo_messageloop 改名为 demo_task_executor
    - demo_tasks 改名为 demo_task_thread_pool

### 2020.8.10

- 添加 demo_gl, 演示通过 `//ui/gl` 模块调用 GL 进行渲染；

### 2020.7.31

- demo_viz_layer_offscreen 支持使用 SkiaOutputDeviceOffscreen 接口进行离屏渲染；

### 2020.7.28

- 添加 demo_viz_layer_offscreen, 演示使用 VIZ 的 CopyOutput 接口进行离屏渲染；

### 2020.7.24

- 添加 demo_viz_layer, 演示使用 VIZ 进行笔迹书写，同时支持使用命令行进行软件和硬件渲染的切换；

### 2020.7.18

- 添加 demo_viz_gui_gpu, 演示使用 VIZ 进行硬件加速渲染以及 VIZ Client 的嵌套；

### 2020.7.15

- 添加 demo_x11_glx 和 demo_x11_egl；

### 2020.7.11

- 添加 demo_x11, 演示使用 X11 创建透明窗口；
- 更新 demo_skia, 默认使用软件渲染，支持透明窗口，GL 渲染支持 GL_RGB565 像素格式；

### 2020.6.28

- 添加 demo_skia, 演示在 Linux 中使用 Skia 进行软/硬件渲染；

### 2020.6.4

- 更新 demo_android_skia， 添加多线程渲染以及帧率同步；

### 2020.5.31

- 添加 demo_android_skia, 演示在 Android 中使用 Skia 进行软/硬件渲染；

### 2020.5.21

- 添加 demo_tracing_perfetto_content, 演示 content 模块是如何将 trace 保存到文件的，该文件可以用于 chrome://tracing；
- 添加 demo_tracing 的文档 [demo_tracing](./demo_tracing/README.md)；

### 2020.5.18

- 将 demo_tracing 移动到 demo_tracing 文件夹，并改名为 demo_tracing_console, 添加 Flush 功能；
- 添加 demo_tracing_perfetto, 演示 trace 和 perfetto 的集成及使用；

### 2020.4.29

- 添加 demo_cc_gui， 演示使用 `cc` 显示 GUI 界面；

### 2020.4.17

- 添加 demo_cc 的 TRACE.txt, 用于协助理解 cc 的运行时行为；

### 2020.4.10

- 添加 demo_cc_offscreen, 演示使用 `cc` 进行离屏渲染；

### 2020.4.6

- 添加 demo_viz_offscreen, 演示使用 `viz` 进行离屏渲染；
- 修改 demo_viz 为 demo_viz_gui，功能不变；

### 2020.3.31

- 添加 demo_viz, 演示使用 `viz` 模块；
- 添加 `viz` 的文档：[viz](./demo_viz/README.md)

### 2020.3.21

- 添加 demo_views，演示使用 `//ui/views` 开发 UI;

### 2020.3.12

- demo_apk 支持 JNI 调用 C++类的实例方法；
- 添加文档：[浏览器启动流程简述](./docs/startup.md)

### 2020.3.7

- demo_apk 支持 JNI；
- 添加文档： [demo_apk](./demo_android/README.md)

### 2020.3.4

- 添加 demo_tracing，用来演示 trace 的使用；
- 添加 demo_apk，用来演示如何使用 gn 创建 Android 应用；
- 添加 demo_shell，用来演示如何使用 Content API 创建一个精简浏览器；

### 更早

添加以下 demo 及相关文档：

- demo_exe: 最简单的 demo，演示 gn 及创建自己的 exe；
- demo_log: 演示使用日志库；
- demo_tracing: 演示使用 Trace；
- demo_tasks: 演示使用线程池 ThreadPool;
- demo_messageloop: 演示使用消息循环 MessageLoop;
- demo_mojo_single_process: 演示在单进程中使用 mojo 库；
- demo_mojo_multiple_process: 演示在多进程中使用 mojo 库；
- demo_mojo_multiple_process_binding: 演示在多进程中使用 mojo 库的 binding 层；
- demo_services: 演示使用基于 mojo 的 servcies 及多进程架构；
- demo_ipc: 演示使用基于 mojo 的 IPC 接口；
- demo_memory: 演示使用 SharedMemory；