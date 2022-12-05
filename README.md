# Demo
<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-5-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->

[中文 README](./README_zh.md)

---

> This project is in the process of migrating to chromium version `110`, it is partially completed, please choose the appropriate branch for your needs and make sure chromium switches to the corresponding branch as well (you will need to run `glcient sync` to sync the code after the switch). If you are interested in this, please feel free to contact me in the Discussions channel.  
> I have built a WeChat group, used to facilitate technical discussions, if you are interested, welcome to add my WeChat, I will pull you into the discussion group. WeChat:  
> <img src="https://user-images.githubusercontent.com/1212025/126026381-b153090c-f53f-4aa8-8204-d830d8fe0a6d.jpeg" width="200">

This project is used to demonstrate how to use some basic mechanisms in chromium, including asynchronous multitasking, mojo, multi-process, viz, cc, gl etc.

> Tips:   
> 1. If you are new to chromium, it is recommended to study these demos in order.  
> 1. These demos are ONLY tested on Linux and Android.
> 1. The label in front of the demo list below indicates the highest supported chromium version of the demo, e.g. v110 means chromium 110 is supported, demos without labels mean that they are only verified on chromium 80.
> 1. Feel free to submit new demos for PR or migrate these demos to other chromium versions

Demo list:

1. [v110] `demo_exe`: The simplest demo to demonstrate gn and create your own exe;
1. [v110] `demo_log`: Demo log library;
1. [v110] `demo_tracing_console`: Demonstrate using Trace to output to the console;
1. [v110] `demo_task_thread_pool`: Demonstrate the use of thread pool ThreadPool;
1. [v110] `demo_task_executor`: Demonstrate using the message loop SingleThreadTaskExecutor;
1. [v110] `demo_callback`: Demo Bind&Callback related content；
1. [v110] `demo_mojo_single_process`: Demonstrate the use of the `mojo` library in a single process;
1. [v110] `demo_mojo_multiple_process`: Demonstrate the use of the `mojo` library in multiple processes;
1. [v110] `demo_mojo_multiple_process_binding`: Demonstrate using the binding layer of the `mojo` library in multiple processes;
1. [v91] `demo_services`: Demonstrate the use of servcies and multi-process architecture based on `mojo`;
1. [v110] `demo_ipc`: Demonstrate the use of IPC interface based on `mojo`;
1. [v91] `demo_mojo_v8`: Demonstrate the use of js to access the mojo interface;
1. [v110] `demo_memory`: Demonstrate the use of SharedMemory;
1. [v110] `demo_tracing_perfetto`: Demonstrate the output of Trace as Json format (used to interface with perfetto);
1. [v110] `demo_tracing_perfetto_content`: Demonstrate how the content module is connected to perfetto;
1. [v110] `demo_resources`: Demo resources related content, including grit, l10n, pak, etc.;
1. [v110] `demo_gl`: Demonstrate using `//ui/gl` for GPU rendering;
1. `demo_viz_gui`: Demonstrate using `viz` to display the GUI interface;
1. [v110] `demo_viz_offscreen`: Demonstrate using `viz` for off-screen rendering;
1. `demo_viz_gui_gpu`: Demonstrate the use of `viz` for hardware accelerated rendering;
1. `demo_viz_layer`: Demonstrate the use of `viz` for interactive rendering;
1. `demo_viz_layer_offscreen`, demonstrate using VIZ's `CopyOutput` interface for off-screen rendering;
1. `demo_cc_gui`: Demonstrate using `cc` to display GUI interface;
1. [v91] `demo_cc_offscreen`: Demonstrate using `cc` for off-screen rendering;
1. [v91] `demo_views`: Demonstrate the use of `//ui/views` to create UI;
1. `demo_apk`: Demonstrate the creation of Android applications, the use of base::android::* and JNI;
1. `demo_android_skia`: Demonstrate the use of Skia for software rendering and hardware rendering on Android;
1. `demo_skia`: Demonstrate the use of Skia for software rendering and hardware rendering on Linux;
1. `demo_x11`: Demonstrate using X11 to create transparent windows;
1. `demo_x11_glx`: Demonstrate the use of glx in a transparent window;
1. `demo_x11_egl`: Demonstrate the use of egl in a transparent window;
1. `demo_shell`: Demonstrate the use of content api to create a streamlined browser that supports Linux and Android;
1. [v91] `demo_gin`: Demonstrate the use of gin to create a javascript runtime;


Documents:

Public documents are in the [docs](./docs) directory, and other documents are in the corresponding directory of the code.

## Usage

1. Go to the `src` directory of chromium and switch to a supported branch, such as `80.0.3987.165` for version 80 or `91.0.4472.144` for version 91 (the last version number does not matter). And run `gclient sync` to synchronize the code.
2. Run the following command to clone this repository to the `src/demo` directory and switch to the corresponding branch, e.g. `c/80.0.3987` for version 80 or `c/91.0.4472` for version 91.

    ```sh
    git clone <address of current repository> demo
    git checkout <the branch>
    ```

3. Find the `out/Default/args.gn` file in your compilation output directory and add the following parameters:

    ```python
    # add extra deps to gn root
    root_extra_deps = ["//demo"]
    
    # If you want to compile the demo of android platform, you need to add the following parameters
    # target_os="android"
    # target_cpu="arm64" # Other architectures can be selected as needed x86, x64, arm, mipsel
    ```

4. Run `ninja -C out/Default <name in demo list>` to generate the required demo (see [BUILD.gn](. /BUILD.gn)), for example using the name `demo_exe` to generate the demo_exe program. Or use `demo` to generate all programs.

> REPEAT: These demos are ONLY tested on Linux and Android.

## TODO

- Add v8 related demo to demonstrate how to inject js objects/methods into v8;
- Improve the documentation of the process initialization part ([docs/startup.md](docs/startup.md));
- Improve the documentation of the UI part ([docs/ui.md](docs/ui.md));
- Improve the documentation of the content module ([docs/content.md](docs/content.md));
- Improve the documentation of demo_shell ([demo_shell/README.md](demo_shell/README.md));
- Add demo to demonstrate how to create aar component;
- Add demo to demonstrate how to use aura to create UI interface;
- Add demo to demonstrate how to use PlatformWindow to create UI interface;
- Add demo to demonstrate how to implement off-screen rendering of web pages;
- Add a demo to demonstrate how to inject new JS objects into Blink;
- Add a demo to demonstrate the principle of `navigator.mediaDevices.getUserMedia()`;
- Add a demo to demonstrate the principle of `tab capture api`;

## Changelog

### 2022.12.4

- Migrate some demo to v110;

### 2022.4.27

- Add demo_gin，demonstrate how to use gin to create a javascript runtime;

### 2021.9.13

- Add demo_mojo_v8，demonstrate how to use js in the render process to access the mojo interface provided by the browser process;

### 2021.9.6

- Migrate demo_views to v91；
- Migrate demo_cc_offscreen to v91；

### 2021.9.4

- Migrate demo_viz_offscreen to v91；

### 2021.9.3

- Migrate demo_ipc to v91;
- Migrate demo_services to v91;

### 2021.8.15

- Migrate demo_gl to v91;

### 2021.7.29

- Migrate below demos to v91：  
    - demo_resources

### 2021.7.22

- Migrate below demos to v91：  
    - demo_memory

### 2021.7.17

- Migrate below demos to v91：  
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

## Contributors ✨

Thanks goes to these wonderful people ([emoji key](https://allcontributors.org/docs/en/emoji-key)):

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tr>
    <td align="center"><a href="https://github.com/ManonLoki"><img src="https://avatars.githubusercontent.com/u/10202538?v=4?s=100" width="100px;" alt=""/><br /><sub><b>ManonLoki</b></sub></a><br /><a href="#ideas-ManonLoki" title="Ideas, Planning, & Feedback">🤔</a> <a href="https://github.com/keyou/chromium_demo/commits?author=ManonLoki" title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/lgjh123"><img src="https://avatars.githubusercontent.com/u/33198766?v=4?s=100" width="100px;" alt=""/><br /><sub><b>ligaojin</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=lgjh123" title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/Drecc"><img src="https://avatars.githubusercontent.com/u/12831867?v=4?s=100" width="100px;" alt=""/><br /><sub><b>Drecc</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=Drecc" title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/hc-tec"><img src="https://avatars.githubusercontent.com/u/59106739?v=4?s=100" width="100px;" alt=""/><br /><sub><b>hc-tec</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=hc-tec" title="Code">💻</a></td>
    <td align="center"><a href="https://qzq.at"><img src="https://avatars.githubusercontent.com/u/19623228?v=4?s=100" width="100px;" alt=""/><br /><sub><b>QZQ</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=SamuelQZQ" title="Code">💻</a></td>
  </tr>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome!
