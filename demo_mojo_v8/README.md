# demo_mojo_v8

这个 demo 主要演示了如何使用运行在 render 进程中的 js 代码，调用 browser 中的 mojo 接口。

如果希望用在 content_shell 中，只需要把下边的注入点替换到 content_shell 对应位置即可。

这里主要用到了 mojo 与 v8 相关知识。

TODO:  [PR Request] 添加 v8 相关 demo 演示如何用向 v8 中注入 js 对象/方法。

## 感谢

感谢飞书 @博博 大佬的支持，这个 demo 完全基于它的 demo 修改而成。

## 调用方向

v8 js -> render -> mojo -> browser

render 和 browser 通过自定义 mojo 接口进行通信。

## 使用方式

首先确保 `src` 仓库的分支为 `91.0.4472.*`，然后进入 `src` 目录下，使用以下命令应用 0002 号 patch `demo/patches/0002-demo_mojo_v8.patch`：

```sh
git applay demo/patches/0002-demo_mojo_v8.patch
```

然后编译 chrome：

```sh
autoninja -C out/Default chrome
```

最后使用使用编译好的 chrome 打开 `demo/demo_mojo_v8/test/index.html` 文件，观察页面控制台即可看到运行结果。
