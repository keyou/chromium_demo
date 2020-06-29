# Demo

[中文](./README_zh.md) | EN

---

This project is used to demonstrate how to use some basic mechanisms in chromium, including asynchronous multitasking, mojo, multi-process, etc.

> Tip: If you are new to chromium, it is recommended to study these demos in order.

Demo list:

1. `demo_exe`: The simplest demo, demonstrate gn and create your own exe;
2. `demo_log`: Demonstrate the use of log library;
3. `demo_tracing_console`: Demonstrate the use of Trace output to the console;
4. `demo_tasks`: demonstrate the use of thread pool ThreadPool;
5. `demo_messageloop`: demonstrate the use of message loop MessageLoop;
6. `demo_mojo_single_process`: demonstrate the use of `mojo` library in a single process;
7. `demo_mojo_multiple_process`: demonstrate the use of the `mojo` library in multiple processes;
8. `demo_mojo_multiple_process_binding`: demonstrate the use of the binding layer of the `mojo` library in multiple processes;
9. `demo_services`: Demonstrate the use of servcies and multi-process architecture based on `mojo`;
10. `demo_ipc`: Demonstrate the use of an IPC interface based on `mojo`;
11. `demo_memory`: demonstrate the use of SharedMemory;
12. `demo_tracing_perfetto`: Demonstrates the output of Trace to Json format (used to connect with perfetto);
13. `demo_tracing_perfetto_content`: Demonstrates how the content module interfaces with perfetto;
14. `demo_resources`: Demo resources related components, including grit, l10n, pak, etc.;
15. `demo_viz_gui`: demonstrate using `viz` to display the GUI interface;
16. `demo_viz_offscreen`: Demonstrate off-screen rendering using `viz`;
17. `demo_cc_gui`: demonstrate using `cc` to display GUI interface;
18. `demo_cc_offscreen`: Demonstrates the use of `cc` for off-screen rendering;
19. `demo_views`: Demonstrate using `//ui/views` to create UI;
20. `demo_apk`: Demonstrate the creation of Android applications, the use of base::android::* and JNI;
21. `demo_android_skia`: Demonstrate software rendering and hardware rendering using Skia on Android;
22. `demo_skia`: Demonstrates the use of Skia for software rendering and hardware rendering on Linux;
23. `demo_shell`: Demonstrate the use of content api to create a streamlined browser that supports Linux and Android;

Document list:

1. [Mojo](./docs/mojo.md)
1. [Browser Startup Process Brief](./docs/startup.md)

Public documents are in the [docs](./docs) directory, and other documents are in the corresponding directory of the code.

## Usage one (recommended)

1. Enter chromium's `src` directory;
2. Execute the following command to clone the repository to the `src/demo` directory;

    ```sh
    git clone <address of current repository> demo
    ```

3. Find the `out/Default/args.gn` file in your compilation output directory and add the following parameters:

    ```python
    # add extra deps to gn root
    root_extra_deps = ["//demo"]
    
    # If you want to compile the demo of android platform, you need to add the following parameters
    target_os="android"
    target_cpu="arm64" # Other architectures can be selected as needed x86, x64, arm, mipsel
    ```

4. Run `ninja -C out/Default demo` to generate all demo programs (see [BUILD.gn](./BUILD.gn) for details);

## Usage two (use gclient)

1. Go to the root directory of the chromium project (the directory above the src) and find the `.gclient` file;
2. Open the `.gclient` file and modify it according to the following settings:

    ```python
    solutions = [
        {"name": "src",
            "url": "https://chromium.googlesource.com/chromium/src.git",
            "deps_file": "DEPS",
            "managed": False,
            "custom_deps": {
                # let gclient pull demo project to'src/demo' dir
                "src/demo": "<address of current repository>",
            },
            "custom_vars": {},
        }
    ]
    ...
    ```

3. Find the `out/Default/args.gn` file in your compilation output directory and add the following parameters:

    ```python
    # add extra deps to gn root
    root_extra_deps = ["//demo"]
    
    # If you want to compile the demo of android platform, you need to add the following parameters
    target_os="android"
    target_cpu="arm64" # Other architectures can be selected as needed x86, x64, arm, mipsel
    ```

4. Execute the `gclient sync` synchronization code, which will pull the `demo` repository to `src/demo`;
5. Run `ninja -C out/Default demo` to generate all demo programs (see [BUILD.gn](./BUILD.gn) for details);

## TODO

- Improve the documentation of the process initialization part ([docs/startup.md](docs/startup.md));
- Improve the documentation of the UI part ([docs/ui.md](docs/ui.md));
- Improve the documentation of the content module ([docs/content.md](docs/content.md));
- Improve the documentation of demo_shell ([demo_shell/README.md](demo_shell/README.md));
- Add demo to demonstrate how to create aar components;
- Add demo to demonstrate how to inject new JS objects into Blink;
- Add demo to demonstrate how to render offscreen web pages;
- Add demo to demonstrate how to use aura to create UI interface;
- Add demo to demonstrate how to use PlatformWindow to create UI interface;

## Changelog

### 2020.6.28

- Add demo_skia to demonstrate the software/hardware rendering using Skia in Linux;

### 2020.6.4

- Update demo_android_skia, add multi-thread rendering and frame rate synchronization;

### 2020.5.31

- Add demo_android_skia to demonstrate the use of Skia in Android for software/hardware rendering;

### 2020.5.21

- Add demo_tracing_perfetto_content to demonstrate how the content module saves trace to a file, which can be used for chrome://tracing;
- Add demo_tracing documentation [demo_tracing](./demo_tracing/README.md);

### 2020.5.18

- Move demo_tracing to demo_tracing folder and rename it to demo_tracing_console, add Flush function;
- Add demo_tracing_perfetto to demonstrate the integration and use of trace and perfetto;

### 2020.4.29

- Add demo_cc_gui to demonstrate using `cc` to display GUI interface;

### 2020.4.17

- Added TRACE.txt of demo_cc to help understand the runtime behavior of cc;

### 2020.4.10

- Add demo_cc_offscreen, demonstrate the use of `cc` for off-screen rendering;

### 2020.4.6

- Add demo_viz_offscreen, demonstrate using `viz` for off-screen rendering;
- Modify demo_viz to demo_viz_gui, the function is unchanged;

### 2020.3.31

- Add demo_viz, demonstrate using `viz` module;
- Add documentation for `viz`: [viz](./demo_viz/README.md)

### 2020.3.21

- Add demo_views, demonstrate using `//ui/views` to develop UI;

### 2020.3.12

- demo_apk supports JNI to call instance method of C++ class;
- Add document: [browser startup process brief description] (./docs/startup.md)

### 2020.3.7

- demo_apk supports JNI;
- Add document: [demo_apk](./demo_android/README.md)

### 2020.3.4

- Add demo_tracing to demonstrate the use of trace;
- Add demo_apk to demonstrate how to use gn to create Android applications;
- Add demo_shell to demonstrate how to use the Content API to create a streamlined browser;

### Earlier

Add the following demo and related documents:

- demo_exe: the simplest demo, demonstrate gn and create your own exe;
- demo_log: demonstrate the use of log library;
- demo_tracing: demonstrate the use of Trace;
- demo_tasks: demonstrate the use of thread pool ThreadPool;
- demo_messageloop: demonstrate the use of message loop MessageLoop;
- demo_mojo_single_process: demonstrate the use of mojo library in a single process;
- demo_mojo_multiple_process: demonstrate the use of the mojo library in multiple processes;
- demo_mojo_multiple_process_binding: demonstrate the use of the binding layer of the mojo library in multiple processes;
- demo_services: demonstrate the use of mojo-based servcies and multi-process architecture;
- demo_ipc: demonstrate the use of mojo-based IPC interface;
- demo_memory: demonstrate the use of SharedMemory;
