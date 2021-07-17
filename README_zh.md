# Demo

> 这个项目正在迁移到 chromium `91.0.4472` 版本，目前还没有完成，如果你想成功编译的话，请切换到 `c/80.0.3987` 分支。如果你对此感兴趣，欢迎随时联系我！  
> 我打算建一个微信群，用来方便大家进行技术讨论，如果你感兴趣，欢迎加我微信，我会把你拉入讨论群：  
> <img src="https://user-images.githubusercontent.com/1212025/126026381-b153090c-f53f-4aa8-8204-d830d8fe0a6d.jpeg" width="200">

这个项目用来演示如何使用 chromium 中的一些基础机制，包括异步多任务，mojo，多进程等。

> 提示：如果你是 chromium 的新手，建议按照顺序学习这些 demo。

Demo 列表：

1. `demo_exe`: 最简单的 demo，演示 gn 及创建自己的 exe；
2. `demo_log`: 演示使用日志库；
3. `demo_tracing_console`: 演示使用 Trace 输出到控制台；
4. `demo_task_thread_pool`: 演示使用线程池 ThreadPool;
5. `demo_task_executor`: 演示使用消息循环 SingleThreadTaskExecutor;
6. `demo_mojo_single_process`: 演示在单进程中使用 `mojo` 库；
7. `demo_mojo_multiple_process`: 演示在多进程中使用 `mojo` 库；
8. `demo_mojo_multiple_process_binding`: 演示在多进程中使用 `mojo` 库的 binding 层；
9. `demo_services`: 演示使用基于 `mojo` 的 servcies 及多进程架构；
10. `demo_ipc`: 演示使用基于 `mojo` 的 IPC 接口；
11. `demo_memory`: 演示使用 SharedMemory；
12. `demo_tracing_perfetto`: 演示将 Trace 输出为 Json 格式（用来对接 perfetto）；
13. `demo_tracing_perfetto_content`: 演示 content 模块是如何对接 perfetto 的；
14. `demo_resources`: 演示 resources 相关内容，包括 grit，l10n，pak 等；
15. `demo_gl`: 演示使用 `//ui/gl` 进行 GPU 渲染；
16. `demo_viz_gui`: 演示使用 `viz` 显示 GUI 界面；
17. `demo_viz_offscreen`: 演示使用 `viz` 进行离屏渲染；
18. `demo_viz_gui_gpu`: 演示使用 `viz` 进行硬件加速渲染；
19. `demo_viz_layer`: 演示使用 `viz` 进行交互渲染；
20. `demo_viz_layer_offscreen`, 演示使用 VIZ 的 `CopyOutput` 接口进行离屏渲染；
21. `demo_cc_gui`: 演示使用 `cc` 显示 GUI 界面；
22. `demo_cc_offscreen`: 演示使用 `cc` 进行离屏渲染；
23. `demo_views`: 演示使用 `//ui/views` 创建 UI；
24. `demo_apk`: 演示创建 Android 应用，base::android::* 和 JNI 的使用；
25. `demo_android_skia`: 演示在 Android 上使用 Skia 进行软件渲染和硬件渲染；
26. `demo_skia`: 演示在 Linux 上使用 Skia 进行软件渲染和硬件渲染；
27. `demo_x11`: 演示使用 X11 创建透明窗口；
28. `demo_x11_glx`: 演示在透明窗口中使用 glx;
29. `demo_x11_egl`: 演示在透明窗口中使用 egl；
30. `demo_shell`: 演示使用 content api, 创建一个精简的浏览器，支持 Linux 和 Android；

文档：

公共文档在 [docs](./docs) 目录，其他文档在代码相应目录下。

## 用法一（推荐）

1. 进入 chromium 的 `src` 目录；
2. 执行以下命令将该仓库 clone 到 `src/demo` 目录下；

    ```sh
    git clone <当前仓库的地址> demo
    ```

3. 找到你的编译输出目录中的 `out/Default/args.gn` 文件，添加以下参数：

    ```python
    # add extra deps to gn root
    root_extra_deps = ["//demo"]
    
    # 如果要编译 android 平台的 demo 需要额外添加以下参数
    target_os="android"
    target_cpu="arm64" # 可以根据需要选择其它架构 x86,x64,arm,mipsel
    ```

4. 执行 `ninja -C out/Default demo` 生成所有 demo 程序（详见 [BUILD.gn](./BUILD.gn)）；

## 用法二（使用 gclient)

1. 进入 chromium 项目的根目录（src 上一层目录）找到 `.gclient` 文件；
2. 打开 `.gclient` 文件，参照下面的设置进行修改：

    ```python
    solutions = [
        { "name"        : "src",
            "url"         : "https://chromium.googlesource.com/chromium/src.git",
            "deps_file"   : "DEPS",
            "managed"     : False,
            "custom_deps" : {
                # let gclient pull demo project to 'src/demo' dir
                "src/demo": "<当前仓库的地址>",
            },
            "custom_vars": {},
        }
    ]
    ...
    ```

3. 找到你的编译输出目录中的 `out/Default/args.gn` 文件，添加以下参数：

    ```python
    # add extra deps to gn root
    root_extra_deps = ["//demo"]
    
    # 如果要编译 android 平台的 demo 需要额外添加以下参数
    target_os="android"
    target_cpu="arm64" # 可以根据需要选择其它架构 x86,x64,arm,mipsel
    ```

4. 执行 `gclient sync` 同步代码，这会拉取 `demo` 仓库到 `src/demo` ；
5. 执行 `ninja -C out/Default demo` 生成所有 demo 程序（详见 [BUILD.gn](./BUILD.gn)）；

## TODO

- 完善进程初始化部分的文档 ([docs/startup.md](docs/startup.md))；
- 完善 UI 部分的文档 ([docs/ui.md](docs/ui.md))；
- 完善 content 模块的文档 ([docs/content.md](docs/content.md))；
- 完善 demo_shell 的文档 ([demo_shell/README.md](demo_shell/README.md))；
- 添加 demo, 演示如何创建 aar 组件；
- 添加 demo, 演示如何使用 aura 创建 UI 界面；
- 添加 demo, 演示如何使用 PlatformWindow 创建 UI 界面；
- 添加 demo, 演示如何实现网页的离屏渲染；
- 添加 demo, 演示如何向 Blink 注入新的 JS 对象；
- 添加 demo, 演示 `navigator.mediaDevices.getUserMedia()` 的原理；
- 添加 demo, 演示 `tab capture api` 的原理；

## 更新日志

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
