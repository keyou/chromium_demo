# 浏览器启动流程简述

以下内容可以参考`demo_shell`的相关代码。

浏览器的启动流程可以简单分为以下几个阶段：

1. 初始化Content模块；
2. 初始化Browser进程；
3. 创建GPU进程；
4. 创建UI；
5. 创建Render进程；

## 初始化Content模块（和Browser进程）

content 通过被 services 模块启动。

content实现了services 模块提供的接口，并且在 content::ContentMain() 中启动（services模块提供的） service_manager::Main() ,在这个函数中进行一些通用的初始化工作，最后（将content作为services的embedder）启动由content实现的RunEmbedderProcess()方法，至此servcies模块将控制权完全转移到content模块的ContentMainRunner。

ContentMainRunner会启动ServiceManager，最后通过调用BrowserMain()将控制权转移到browser子模块。

BrowserMain()通过BrowserMainRunner继续启动，BrowserMainRunner包装了BrowserMainLoop，BrowserMainLoop的主要功能包括：

1. 调用BrowserMainParts，以便允许用户的逻辑被执行；
1. 调用content::BrowserStartupComplete()，以便通知Android初始化完成；
1. 初始化主线程；
1. 创建IO线程；
1. 初始化Mojo;
1. 初始化音视频/麦克风/触摸屏等设备；
1. 初始化WebRTC;
1. 初始化剪切板相关服务；
1. 根据需求启动GPU进程；
1. 启动主线程的消息循环；

BrowserMainParts作为Browser初始化的末端，用户在这里进行扩展，插入创建UI的逻辑。

```
#0 content/public/browser/browser_main_parts.cc:*()
#1 content/browser/browser_main_loop.cc:*()
#2 content/browser/browser_main_runner_impl.cc:*()
#3 content/browser/browser_main.cc:43:BrowserMain()
#4 content/app/content_main_runner_impl.cc529:RunBrowserProcessMain()
#5 content/app/content_main_runner_impl.cc:978:RunServiceManager()
#6 content/app/content_main_runner_impl.cc:878:Run()
#7 content/app/content_service_manager_main_delegate.cc:52:RunEmbedderProcess()
```

综上，Browser进程的初始化流程如下：
content::ContentMain()->service_manager::Main()->SerivceManagerMainDelegate->ContentMainRunner->content::BrowserMain()->BrowserMainRunner->BrowserMainLoop->BrowserMainParts->(创建UI)

## 创建GPU进程

TODO

## 创建UI

Browser初始化完成之后，会在BrowserMainParts中创建UI。

> 注意：并不是一定要在这里创建UI，因为创建UI是一个相对独立的过程，可以放在任何业务觉得合适的时机，比如放在用户打开某一个网页的时候再创建。

创建UI可以分为以下几步：

1. 创建Native窗口；
2. 创建WebContents，并将WebContents和Native窗口关联；

TODO

## 创建Render进程

Render进程并不会一开始就创建，而是在要打开某一个网页的时候才创建。

Render进程的启动流程如下：
WebContents->NavigationController->NavigationRequest->RenderFrameHostManager->SiteInstance->RenderProcessHost::Init()->ChildProcessLauncher

由于RenderProcess是按需启动的，因此它的启动流程比较复杂。

TODO
