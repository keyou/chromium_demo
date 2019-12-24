# Demo

这个项目用来演示如何使用chromium中的一些机制，包括异步多任务，mojo等。

Demo 列表：

- demo:demo
- demo:demo_tasks
- demo:demo_messageloop
- demo:demo_mojo_multiple_process
- demo:demo_mojo_single_process

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

4. 执行 `ninja -C out/Default demo:demo` 生成 `demo` 程序（查看`BUILD.gn`文件查看其他demo）；

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
5. 执行 `ninja -C out/Default demo:demo` 生成 `demo` 程序（查看`BUILD.gn`文件查看其他demo）；
