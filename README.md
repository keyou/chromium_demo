# Demo

这个项目用来演示如何使用chromium中的一些基础机制，包括异步多任务，mojo，多进程等。

> 提示：如果你是 chromium 的新手，建议按照顺序学习这些demo。

Demo 列表：

1. `demo`: 最简单的 demo，演示在自己的程序中使用 base 库；
1. `demo_log`: 演示使用日志库；
1. `demo_tracing`: 演示使用Trace；
1. `demo_tasks`: 演示使用线程池 TaskScheduler;
1. `demo_messageloop`: 演示使用消息循环 MessageLoop;
1. `demo_mojo_single_process`: 演示在单进程中使用 `mojo` 库；
1. `demo_mojo_multiple_process`: 演示在多进程中使用 `mojo` 库；
1. `demo_mojo_multiple_process_binding`: 演示在多进程中使用 `mojo` 库的binding层；
1. `demo_services`: 演示使用基于 `mojo` 的 servcies 及多进程架构；
1. `demo_ipc`: 演示使用基于 `mojo` 的 IPC 接口；
1. `demo_memory`: 演示使用SharedMemory；
1. `demo_resources`: 演示resources相关内容，包括grit，l10n，pak等；
1. `demo_apk`: 演示创建Android应用，base::android::*和JNI的使用；
1. `demo_shell`: 演示使用content api,创建一个精简的浏览器，支持Linux和Android；

文档列表：

1. [Mojo](./docs/mojo.md)
1. [浏览器启动流程简述](./docs/startup.md)

公共文档在 [docs](./docs) 目录，其他文档在代码相应目录下。

## 用法一(推荐)

1. 进入 chromium 的 `src` 目录；
2. 执行以下命令将该仓库clone到 `src/demo` 目录下；

    ```sh
    git clone git@gitlab.gz.cvte.cn:CrOS/seewo/demo.git demo
    ```

3. 找到你的编译输出目录中的 `out/Default/args.gn` 文件，添加以下参数：

    ```python
    # add extra deps to gn root
    root_extra_deps = ["//demo"]
    ```

4. 执行 `ninja -C out/Default demo:all` 生成所有 demo 程序（详见 [BUILD.gn](./BUILD.gn)）；

## 用法二(使用gclient)

1. 进入chromium项目的根目录（src上一层目录）找到 `.gclient` 文件；
2. 打开 `.gclient` 文件，参照下面的设置进行修改：

    ```python
    solutions = [
        { "name"        : "src",
            "url"         : "git@gitlab.gz.cvte.cn:CrOS/src.git",
            "deps_file"   : "DEPS",
            "managed"     : False,
            "custom_deps" : {
                # let gclient pull demo project to 'src/demo' dir
                "src/demo": "git@gitlab.gz.cvte.cn:CrOS/seewo/demo.git",
            },
            "custom_vars": {},
        }
    ]
    ...
    ```

3. 找到你的编译输出目录中的 `out/Default/args.gn` 文件，添加以下参数：

    ```python
    # add extra deps to gn root
    root_extra_deps = ["//demo"]
    ```

4. 执行 `gclient sync` 同步代码，这会拉取 `demo` 仓库到 `src/demo` ；
5. 执行 `ninja -C out/Default demo:all` 生成所有 demo 程序（详见 [BUILD.gn](./BUILD.gn)）；

## TODO：

- 添加 demo_aar,用来演示如何创建 aar 组件；
- 给复杂的demo添加文档，例如 demo_shell；

## 更新日志

#### 2020.3.12

- demo_apk 支持JNI调用C++类的实例方法；
- 添加文档：[浏览器启动流程简述](./docs/startup.md)

#### 2020.3.7

- demo_apk 支持JNI；
- 添加文档： [demo_apk](./demo_android/README.md)

#### 2020.3.4:  

- 添加 demo_tracing，用来演示 trace 的使用；
- 添加 demo_apk，用来演示如何使用 gn 创建 Android 应用；
- 添加 demo_shell，用来演示如何使用 Content API 创建一个精简浏览器；

#### 更早

 - 略