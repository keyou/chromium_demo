# demo_views

demo_views 使用 `//ui/views` 来构建程序界面。

## `//ui/views`

主要用于桌面端的 UI 开发，比如 Windows/Linux 上的浏览器工具栏，地址栏等。它依赖 cc,aura 和 viz 等底层框架。

TODO: 完善文档

Views 在 OnPaint 中将自己所要呈现的内容绘制到 Canvas 中，这个 Canvas 将绘制操作记录到 PaintOpBuffer 中（并不立即进行真实的绘制）。

在以下文件中可以查看哪些绘制操作可以被记录，以及如何被记录。
cc/paint/paint_op_buffer.h
