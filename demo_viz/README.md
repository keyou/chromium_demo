# demo_viz

## demo_viz_offscreen

demo_viz_offscreen 演示了直接使用 viz 内部的接口来进行离屏渲染。

## demo_viz_gui

demo_viz_gui 演示了使用 viz 提供的 mojo 接口进行 GUI 软件渲染。
这些 mojo 接口内部包装了 demo_viz_offscreen 所演示的技术。
该 demo 还增加了 client raster 以及多 FrameSink 的演示。

## demo_viz_gui_gpu

demo_viz_gui_gpu 在 demo_viz_gui 的基础上添加了对硬件渲染的支持。
重点在于如何初始化硬件渲染环境以及如何使用GPU资源，项目中需要注意资源的释放。

TODO: 解决 client 端 raster 的偏色问题。

## demo_viz_layer

demo_viz_layer 添加了用户交互控制，重点在于如何控制 UI 的局部刷新及 UI 没有改变的时候的不刷新。
同时该 demo 支持使用命令行切换渲染模式以及将渲染结果保存到文件。

## demo_viz_layer_offscreen

demo_viz_layer 演示使用 CopyOutput 接口来实现 viz 离屏渲染，然后再将离屏画面渲染到窗口上。

## viz 调试技巧

tracing categories:

`viz,gpu,ipc,mojom,skia,disabled-by-default-toplevel.flow` 获取详细的 viz 以及 gpu 执行情况。
`disabled-by-default-gpu.service` 获取 commandbuffer decoder的执行情况。

命令行参数：

`--enable-gpu-service-tracing` 启动 gpu service tracing，每一个 GL 调用都会被记录到 Trace 中。
`--use_virtualized_gl_contexts=0` 禁用 virtual GL Context，降低追踪的复杂度。
