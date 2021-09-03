# demo_mojo

该目录中的 demo 用来演示 chromium 中的 IPC 组件 `mojo`。 它是 chromium 多进程架构依赖的基础组件，因此非常重要。关于它的详细介绍可以参考官方文档或者我的 [Blog](https://keyou.github.io/blog/2020/01/03/Chromium-Mojo&IPC/)。

需要注意的是，这里演示的是 mojo 本身的接口及其使用方法，这些接口不依赖于任何的 chromium 核心逻辑，因此在 chromium 中看起来可能不太一样，原因是 chromium 对一些接口进行了更多的包装。

该目录包含以下几个 demo：
- demo_mojo_single_process: 演示在单进程中使用 mojo 的 C 接口；
- demo_mojo_multiple_process: 演示在多进程使用 mojo 的 C 接口；
- demo_mojo_multiple_process_binding: 演示在多进程中使用 mojo 的 C++ 接口；
- demo_services: （**DEPRECATED**）演示在多进程中使用 service 机制；