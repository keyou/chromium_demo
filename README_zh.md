# Demo

> 这个项目正在迁移到 chromium `120` 版本(6099分支)，请根据你的需要选择合适的分支，并保证 chromium 也切换到对应的分支（切换后需要执行 `glcient sync` 同步代码）。如果你对此感兴趣，欢迎随时联系我！
> 我建了一个微信群，用来方便大家进行技术讨论，如果你感兴趣，欢迎加我微信，我会把你拉入讨论群：
> <img src="https://user-images.githubusercontent.com/1212025/126026381-b153090c-f53f-4aa8-8204-d830d8fe0a6d.jpeg" width="200">

这个项目用来演示如何使用 chromium 中的一些基础机制，包括异步多任务，mojo，多进程，viz，cc，gl等。

> 提示：
> 1. 如果你是 chromium 的新手，建议按照顺序学习这些 demo。
> 1. 这些 demo 只在 Linux 和 Android 上进行了测试。每个平台支持的 demo 列表见 BUILD.gn。
> 1. 下面 demo 列表前面的标签表示该 demo 最高支持的 chromium 版本，比如 `M120` 表示最高支持 chromium 120，没有标签的 demo 表示只在 chromium 80 上验证过。
> 1. 欢迎提交 PR 新增 demo 或者将这些 demo 迁移到其他 chromium 版本。
> 1. 由于精力有限，该项目每年更新一次内核（大约间隔 10 个版本）。

Demo 列表：

1. [M120] `demo_exe`: 最简单的 demo，演示 gn 及创建自己的 exe；
1. [M120] `demo_log`: 演示使用日志库；
1. [M120] `demo_tracing_console`: 演示使用 Trace 输出到控制台；
1. [M120] `demo_task_thread_pool`: 演示使用线程池 ThreadPool 位于demo_task下;
1. [M120] `demo_task_executor`: 演示使用消息循环 SingleThreadTaskExecutor 位于demo_task下;
1. [M120] `demo_callback_(once|repeating)`: 演示 Bind&Callback 相关内容；
1. [M120] `demo_linktest`: 演示错误的使用 gn 导致链接错误；
1. [M120] `demo_mojo_single_process`: 演示在单进程中使用 `mojo` 库；
1. [M120] `demo_mojo_multiple_process`: 演示在多进程中使用 `mojo` 库；
1. [M120] `demo_mojo_multiple_process_binding`: 演示在多进程中使用 `mojo` 库的 binding 层；
1. [M91] `demo_services`: 演示使用基于 `mojo` 的 servcies 及多进程架构；
1. [M120] `demo_ipc`: 演示使用基于 `mojo` 的 IPC 接口；
1. [M91] `demo_mojo_v8`: 演示使用 js 访问 mojo 接口；
1. [M120] `demo_memory`: 演示使用 SharedMemory；
1. [M120] `demo_tracing_perfetto`: 演示将 Trace 输出为 Json 格式（用来对接 perfetto）；
1. [M120] `demo_tracing_perfetto_content`: 演示 content 模块是如何对接 perfetto 的；
1. [M120] `demo_resources`: 演示 resources 相关内容，包括 grit，l10n，pak 等；
1. [M120] `demo_gl`: 演示使用 `//ui/gl` 进行 GPU 渲染；
1. [M110] `demo_viz_gui`: 演示使用 `viz` 显示 GUI 界面；
1. [M110] `demo_viz_offscreen`: 演示使用 `viz` 进行离屏渲染；
1. `demo_viz_gui_gpu`: 演示使用 `viz` 进行硬件加速渲染；
1. `demo_viz_layer`: 演示使用 `viz` 进行交互渲染；
1. `demo_viz_layer_offscreen`, 演示使用 VIZ 的 `CopyOutput` 接口进行离屏渲染；
1. `demo_cc_gui`: 演示使用 `cc` 显示 GUI 界面；
1. [M110] `demo_cc_offscreen`: 演示使用 `cc` 进行离屏渲染；
1. [M110] `demo_views`: 演示使用 `//ui/views` 创建 UI；
1. `demo_apk`: 演示创建 Android 应用，base::android::* 和 JNI 的使用；
1. `demo_android_skia`: 演示在 Android 上使用 Skia 进行软件渲染和硬件渲染；
1. [M110] `demo_skia`: 演示在 Linux 上使用 Skia 进行软件渲染和硬件渲染；
1. `demo_x11`: 演示使用 X11 创建透明窗口；
1. `demo_x11_glx`: 演示在透明窗口中使用 glx;
1. `demo_x11_egl`: 演示在透明窗口中使用 egl；
1. `demo_shell`: 演示使用 content api, 创建一个精简的浏览器，支持 Linux 和 Android；
1. [M110] `demo_gin`: 演示使用 gin, 创建一个精简JS运行时；

文档：

公共文档在 [docs](./docs) 目录，其他文档在代码相应目录下。

## 用法

1. 进入 chromium 的 `src` 目录，并切换到支持的分支，比如 120 版本的 `120.0.6099.40` 或者 91 版本的 `91.0.4472.144`（最后一位版本号不影响）。并执行 `gclient sync` 同步代码；
2. 执行以下命令将该仓库 clone 到 `src/demo` 目录下，并切换到对应分支，比如 80 版本的 `c/120.0.6099` 或者 91 版本的 `c/91.0.4472`；

    ```sh
    git clone <当前仓库的地址> demo
    git checkout <对应的分支>
    ```

3. 找到编译输出目录中的 `out/Default/args.gn` 文件，添加以下参数：

    ```python
    # add extra deps to gn root
    root_extra_deps = ["//demo"]
    # disable warngings as errors
    treat_warnings_as_errors = false

    # 如果要编译 android 平台的 demo 需要额外添加以下参数
    # target_os="android"
    # target_cpu="arm64" # 可以根据需要选择其它架构 x86,x64,arm,mipsel
    ```

4. 执行 `ninja -C out/Default <demo列表中的名称>` 生成所需的demo（详见 [BUILD.gn](./BUILD.gn)），比如使用名称 `demo_exe` 生成 demo_exe 程序。或者使用 `demo` 生成所有的程序；

> 再次强调，这些 demo 只在 Linux 和 Android 上测试通过，其他平台没有测试，欢迎提交 PR/MR 兼容其他平台。

## TODO

- 添加 v8 相关 demo 演示如何用向 v8 中注入 js 对象/方法；
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

### 2023.12.23

- 升级 demos 到 chromium 120。

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

## Contributors ✨

Thanks goes to these wonderful people ([emoji key](https://allcontributors.org/docs/en/emoji-key)):

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tbody>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/ManonLoki"><img src="https://avatars.githubusercontent.com/u/10202538?v=4?s=100" width="100px;" alt="ManonLoki"/><br /><sub><b>ManonLoki</b></sub></a><br /><a href="#ideas-ManonLoki" title="Ideas, Planning, & Feedback">🤔</a> <a href="https://github.com/keyou/chromium_demo/commits?author=ManonLoki" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/lgjh123"><img src="https://avatars.githubusercontent.com/u/33198766?v=4?s=100" width="100px;" alt="ligaojin"/><br /><sub><b>ligaojin</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=lgjh123" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/Drecc"><img src="https://avatars.githubusercontent.com/u/12831867?v=4?s=100" width="100px;" alt="Drecc"/><br /><sub><b>Drecc</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=Drecc" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/hc-tec"><img src="https://avatars.githubusercontent.com/u/59106739?v=4?s=100" width="100px;" alt="hc-tec"/><br /><sub><b>hc-tec</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=hc-tec" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://qzq.at"><img src="https://avatars.githubusercontent.com/u/19623228?v=4?s=100" width="100px;" alt="QZQ"/><br /><sub><b>QZQ</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=SamuelQZQ" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/mikucy"><img src="https://avatars.githubusercontent.com/u/23072202?v=4?s=100" width="100px;" alt="Midori"/><br /><sub><b>Midori</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=mikucy" title="Code">💻</a> <a href="https://github.com/keyou/chromium_demo/commits?author=mikucy" title="Documentation">📖</a></td>
    </tr>
  </tbody>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome!
