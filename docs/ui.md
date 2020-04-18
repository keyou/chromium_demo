# UI

UI 分两部分，一部分是窗口，一部分是控件。

chromium 中对窗口的包装包括：

- aura
- PlatformWindow
- X11/Ozone/Windows

chromium 中对UI控件的包装包括:

- views
- cc
- viz

chromium 中对渲染的包装包括:

- skia
- command buffer
- angle
- egl/opengl(es)

TODO: 完善文档

> views 和 WebContents 相关类图见： <https://drive.google.com/file/d/1iir_i9uiKAR6L4oyCiGFT-0DvPtVB8Xw/view?usp=sharing>
