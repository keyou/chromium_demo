# Viz (Visuals)

`Viz` 用于 `compositing`,`gl`,`hit testing`,`media`。

`Viz` 的代码在以下几个目录中：

- `//services/viz`: 主要定义 viz 的 mojo 接口；
- `//components/viz`: compositing 和 hit testing 部分的实现；
- `//gpu`: gl 部分的实现；
- `//media`: media 部分的实现；

> 在 `//components/viz/demo` 下有一个官方提供的 Viz 的 demo, 这里的 demo_viz 是参考那个 demo 完成的。在以上几个目录中也存放有关于 viz 的文档，这篇文档是对那些文档的总结和补充。

Viz 涉及以下三个端：

- `client`: 用于生成要显示的画面，至少有一个根 client，可以有多个子 client，它们组成了一个 client 树，每一个 Client 至少对应一个 FrameSinkId 和一个 LocalSurfaceId，如果父子 client 之间的 UI 需要嵌入，则子 client 作为 SurfaceDrawQuad 嵌入到父 client 中；
- `host`： 用于注册 client 端，只能运行在特权端，负责协助 client 建立起和 service 的连接，从而建立 client 和 client 之间的树形关系，因此它看起来就像是在把很多 client 组合 (Composite) 在一起，所以 ui::Compositor 是 host 的一个实现就比较好理解了；
- `service`: 运行 Viz 的内核，host 和 client 都是在和它通信。

> 以上描述比较抽象，简单理解就是： 运行 `viz::mojom::FrameSinkMnagerClient` 的端叫做 host，运行 `viz::mojom::FrameSinkManager` 和 `viz::mojom::CompositorFrameSink` 的端叫 service, 运行 `viz::mojom::CompositorFrameSinkClient` 的端叫 client。  
> 从名字可以看出在通信层面没有所谓的 host 端，host 端只是 FrameSinkManager 的 client.

## 主要概念解释

- `viz::mojom::CompositorFrameSinkClient (CFSC)` = Client：表示一个画面的来源；
- `viz::mojom::CompositorFrameSink (CFS)` = FrameSink (FS)：用于处理 CF 的地方，每一个 FrameSink 都单独处理某一部分的画面；
- `viz::mojom::FrameSinkManager (FSM)` = FrameSinkManager：用于管理 CFS, 包括其创建和销毁；
- `viz::CompositorFrame (CF)` = Frame：表示一个画面，可以是一帧完整的画面或者某一个区域的画面，它内部保存一个 `viz::RenderPass` 的列表；
- `viz::RenderPass`：内部包含了一个 `viz::DrawQuad` 的列表；
- `viz::DrawQuad`：包含具体的显示内容，目前共有 9 种显示内容，有简单的 DrawQuad，比如图片，视频，纯色等，也有用于嵌套的 DrawQuad，比如使用 `RenderPassDrawQuad` 可以嵌入其他的 RenderPass，使用 `SurfaceDrawQuad` 可以嵌入其他的 `viz::Surface` 等。
- `viz::Surface`: 内部维护了 2 个 CompositorFrame 对象，分别为 ActiveFrame 和 PendingFrame，当 CF 所表示的画面的 size 或者 scale factor 改变的时候，会创建新的 Surface；

> DrawQuad 类型列表：([draw_quad.h](https://source.chromium.org/chromium/chromium/src/+/master:components/viz/common/quads/draw_quad.h))  
    kPictureContent,
    kRenderPass,
    kSolidColor,
    kStreamVideoContent,
    kSurfaceContent,
    kTextureContent,
    kTiledContent,
    kYuvVideoContent,
    kVideoHole.

> Viz 和 cc 相关的类图见： <https://drive.google.com/file/d/1LW5d0GzlksSVtncDO1kpeHCVs3YD3rl0/view?usp=sharing>

## Viz 的 Id 设计

每一个 client 都至少对应一个 `FrameSinkId` 和 `LocalSurfaceId`，在 client 的整个生命周期中所有 FrameSink 的 `client_id`（见下文）都是固定的，而 `LocalSurfaceId` 会根据 client 显示画面的 size 或 scale factor 的改变而改变。他们两个共同组成了 `SurfaceId`，用于在 service 端全局标识一个 `Surface` 对象。也就是说对于每一个 `Surface`，都可以获得它是由谁在什么 size 或 scale facotr 下产生的。

### `FrameSinkId`

- `client_id` = uint32_t, 每个 client 都会有唯一的一个 ClientId 作为标识符，标识一个 CompositorFrame 是由哪个 client 产生的，也就是标识 CompositorFrame 的来源；
- `sink_id` = uint32_t, 在 service 端标识一个 CompositorFrameSink 实例，Manager 会为每个 client 在 service 端创建一个 CompositorFrameSink，专门用于处理该 client 生成的 CompositorFrame，也就是标识 CompositorFrame 的处理端；
- `FrameSinkId = client_id + sink_id`，将 CF 的来源和处理者关联起来。

FrameSinkId 可以由 `FrameSinkIdAllocator` 辅助类生成。

> 什么时候需要使用单 client 多 FrameSink？  
（以下为推测，没有验证。TODO: 验证以下想法。）  
在实际使用中，往往一个进程就是一个 client，专门负责一个业务模块 UI 的实现，而一个业务往往由很多的 UI 元素组成，因此可以让每个 FrameSink 负责一部分的 UI，此时就需要单使用单 client 多 FrameSink 的机制了，这种机制可以实现 UI 的局部刷新。当然，多 client 也能实现这一点，从设计上来讲，此时 client_id 的设计意义以及允许单 client 多 FrameSink 的设计意义就没有那么大了。所以推测在 chrome 中浏览器主程序是一个 client，而每一个插件都是一个独立的 client。

### `LocalSurfaceId`

- `parent_sequence_number` = uint32_t, 当自己作为父 client，并且 surface 的 size 和 device scale factor 改变的时候改变；
- `child_sequence_number` = uint32_t, 当自己作为子 client，并且 surface 的 size 和 device scale factor 改变的时候改变；
- `embed_token` = 可以理解为一个随机数， 用于避免 LocalSurafaceId 可猜测，当父 client 和子 client 的父子关系改变的时候改变；
- `LocalSurfaceId = parent_sequence_number + child_sequence_number + embed_token`，当 client 的 size 当前 client 产生的画面改变。

LocalSurfaceId 可以由 `ParentLocalSurfaceIdAllocator` 或者 `ChildLocalSurfaceIdAllocator` 这两个辅助类生成。前者用于由父 client 负责生成自己的 LocalSurfaceId 的时候，后者用于由子 client 自己负责生成自己的 LocalSurfaceId 的时候。使用哪种方式要看自己的 UI 组件之间的依赖关系的设计。

LocalSurfaceId 在很多时候都包装在 `LocalSurfaceIdAllocation` 内，该类记录了 LocalSurfaceId 的创建时间。改时间在创建 CF 的时候需要用到。

### `SurfaceId`

`SurfaceId = FrameSinkId + LocalSurfaceId`

SurfaceId 全局唯一记录一个显示画面，它可以被嵌入其他的 CF 或者 RenderPass 中，从而实现显示界面的嵌入和局部刷新。

## Viz 的显示原理

TODO: 研究 Viz 底层对接 OpenGL 的逻辑以及 向上对接 cc 的逻辑。
