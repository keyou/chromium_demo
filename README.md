# Demo
<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-8-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->

- English [README](./README.md) [Change Log](./CHANGELOG.md)
- 中文 [说明](./README_zh.md) [更新日志](./CHANGELOG_ZH.md)

---

> This project is in the process of migrating to chromium version `141`(branch 7390), please choose the appropriate branch for your needs and make sure chromium switches to the corresponding branch as well (you will need to run `glcient sync` to sync the code after the switch). If you are interested in this, please feel free to contact me in the Discussions channel.
> I have built a WeChat group, used to facilitate technical discussions, if you are interested, welcome to add my WeChat, I will pull you into the discussion group. WeChat:
> <img src="https://user-images.githubusercontent.com/1212025/126026381-b153090c-f53f-4aa8-8204-d830d8fe0a6d.jpeg" width="200">

This project is used to demonstrate how to use some basic mechanisms in chromium, including asynchronous multitasking, mojo, multi-process, viz, cc, gl etc.

> Tips:
> 1. If you are new to chromium, it is recommended to study these demos in order.
> 1. These demos are ONLY tested on Linux and Android. The list of supported demos for each platform can be found in the BUILD.gn file.
> 1. The label in front of the demo list below indicates the highest supported chromium version of the demo, e.g. `M120` means chromium 120 is supported, demos without labels mean that they are only verified on chromium 80.
> 1. Feel free to submit new demos for PR or migrate these demos to other chromium versions.
> 1. Due to limited resources, the project updates its kernel once a year, approximately every 10 versions.

Demo list:

1. [M141] `demo_exe`: The simplest demo to demonstrate gn and create your own exe;
2. [M141] `demo_log`: Demo log library;
3. [M141] `demo_task_thread_pool`: Demonstrate the use of thread pool ThreadPool;
4. [M141] `demo_task_executor`: Demonstrate using the message loop SingleThreadTaskExecutor;
5. [M141] `demo_task_thread`: Demonstrates the use of a task in a new thread;
6. [M141] `demo_callback`: Demo Bind&Callback related content；
7. [M120] `demo_linktest`: Demonstrates incorrect use of gn causing link errors;
8.  [M141] `demo_mojo_single_process`: Demonstrate the use of the `mojo` library in a single process;
9.  [M141] `demo_mojo_multiple_process`: Demonstrate the use of the `mojo` library in multiple processes;
10. [M141] `demo_mojo_multiple_process_binding`: Demonstrate using the binding layer of the `mojo` library in multiple processes;
11. [M141] `demo_mojo_child_process`: Demonstrate using the the `mojo` library to connect via parent and directly communicate between two child process
12. [M91] `demo_services`: Demonstrate the use of servcies and multi-process architecture based on `mojo`;
13. [M120] `demo_ipc`: Demonstrate the use of IPC interface based on `mojo`;
14. [M141] `demo_mojo_v8`: Demonstrate the use of js to access the mojo interface;
15. [M141] `demo_memory`: Demonstrate the use of SharedMemory;
16. [M141] `demo_tracing_console`: Demonstrate using Trace to output to the console;
17. [M141] `demo_tracing_perfetto`: Demonstrate the output of Trace as Json format (used to interface with perfetto);
18. [M141] `demo_tracing_perfetto_content`: Demonstrate how the content module is connected to perfetto;
19. [M141] `demo_resources`: Demo resources related content, including grit, l10n, pak, etc.;
20. [M141] `demo_gl`: Demonstrate using `//ui/gl` for GPU rendering;
21. [M120] `demo_viz_gui`: Demonstrate using `viz` to display the GUI interface;
22. [M120] `demo_viz_offscreen`: Demonstrate using `viz` for off-screen rendering;
23. `demo_viz_gui_gpu`: Demonstrate the use of `viz` for hardware accelerated rendering;
24. `demo_viz_layer`: Demonstrate the use of `viz` for interactive rendering;
25. `demo_viz_layer_offscreen`, demonstrate using VIZ's `CopyOutput` interface for off-screen rendering;
26. [M120] `demo_cc_gui`: Demonstrate using `cc` to display GUI interface;
27. [M120] `demo_cc_offscreen`: Demonstrate using `cc` for off-screen rendering;
28. [M120] `demo_views`: Demonstrate the use of `//ui/views` to create UI;
29. `demo_apk`: Demonstrate the creation of Android applications, the use of base::android::* and JNI;
30. `demo_android_skia`: Demonstrate the use of Skia for software rendering and hardware rendering on Android;
31. [M120] `demo_skia`: Demonstrate the use of Skia for software rendering and hardware rendering on Linux;
32. `demo_x11`: Demonstrate using X11 to create transparent windows;
33. `demo_x11_glx`: Demonstrate the use of glx in a transparent window;
34. `demo_x11_egl`: Demonstrate the use of egl in a transparent window;
35. [M120] `demo_gin`: Demonstrate the use of gin to create a javascript runtime;
36. `demo_shell`: Demonstrate the use of content api to create a streamlined browser that supports Linux and Android;

Documents:

Public documents are in the [docs](./docs) directory, and other documents are in the corresponding directory of the code.

## Usage

1. Go to the `src` directory of chromium and switch to a supported branch, such as `120.0.6099.40` for version 120 or `91.0.4472.144` for version 91 (the last version number does not matter). And run `gclient sync` to synchronize the code.
2. Run the following command to clone this repository to the `src/demo` directory and switch to the corresponding branch, e.g. `c/120.0.6099` for version 120 or `c/91.0.4472` for version 91.

    ```sh
    git clone <address of current repository> demo
    git checkout <the branch>
    ```

3. Find the `out/Default/args.gn` file in your compilation output directory and add the following parameters:

    ```python
    # add extra deps to gn root
    root_extra_deps = ["//demo"]
    # disable warnings as errors
    treat_warnings_as_errors = false

    # If you want to compile the demo of android platform, you need to add the following parameters
    # target_os="android"
    # target_cpu="arm64" # Other architectures can be selected as needed x86, x64, arm, mipsel
    ```

4. Run `ninja -C out/Default <name in demo list>` to generate the required demo (see [BUILD.gn](./BUILD.gn)), for example using the name `demo_exe` to generate the demo_exe program. Or use `demo` to generate all programs.

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
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/shaochenguang"><img src="https://avatars.githubusercontent.com/u/33643378?v=4?s=100" width="100px;" alt="chenguang shao"/><br /><sub><b>chenguang shao</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=shaochenguang" title="Code">💻</a></td>
    </tr>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="http://www.dyldr.top"><img src="https://avatars.githubusercontent.com/u/53362310?v=4?s=100" width="100px;" alt="Yi Lu"/><br /><sub><b>Yi Lu</b></sub></a><br /><a href="https://github.com/keyou/chromium_demo/commits?author=DynamicLoader" title="Code">💻</a></td>
    </tr>
  </tbody>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome!
