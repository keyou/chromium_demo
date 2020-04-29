# content

//content 模块可以分为以下几个相对独立的部分：

1. 负责多进程架构通用部分的初始化；
2. 负责 Browser/Renderer/Gpu/Utility 等进程专有部分的初始化；
3. Browser 进程负责的功能： 浏览器界面的创建，相关窗口句柄的维护，生成浏览器界面相关的CompositorFrame发送到Gpu进程，WebContents接口的调用，Renderer/Gpu进程的创建及相关通信通道的建立；
4. Renderer 进程负责的功能： Blink 的初始化，网页的渲染，生成网页相关的CompositorFrame发送到Gpu进程；
5. Gpu 进程负责的功能： CompositorFrame的渲染，提供Gpu Raster功能；

TODO: 完善 content 模块的相关文档
