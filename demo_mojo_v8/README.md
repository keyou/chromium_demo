# 这是一个演示mojo在chromium中实际应用的Demo

这个Demo主要演示了在Chromium实际开发中，如何通过Mojo进行Browser与Render进程之间的通信。

如果希望用在ContentShell中，只需要把下边的注入点替换到ContentShell对应位置即可。

这里用到了 Mojo 与 V8相关知识，其中V8部分请自行查阅资料，Mojo部分则可参考Mojo相关Demo。

# 感谢
这里要感谢 飞书 @博博 大佬的支持，本Demo完全基于它的Demo修改而成。

## 调用方向
   V8 -> Render -> Browser
   其中Render与Browser通过自定义Mojo进行通讯


## 使用方式

#### Browser

chrome/browser/BUILD.gn

```python
static_library("browser") {
   #...
   public_deps = [
       # 加入我们的依赖
       "//demo/demo_mojo_v8:browser",
       #...
   ]
}

```

chrome/browser/chrome_content_browser_client_receiver_bindings.cc

```cpp
// ...
#include "demo/demo_mojo_v8/browser/receiver_register.h"

void ChromeContentBrowserClient::ExposeInterfacesToRenderer(
    service_manager::BinderRegistry* registry,
    blink::AssociatedInterfaceRegistry* associated_registry,
    content::RenderProcessHost* render_process_host) {

        // 注册我们的Receiver
        demo::ReceiverRegister::RegisterAll(registry);
    }


```

#### Render

chrome/renderer/BUILD.gn

```python

static_library("renderer") {
    #...

    deps = [
          #加入我们的依赖
        "//demo/demo_mojo_v8:renderer",
        # ...
    ]
}

```

chrome/renderer/chrome_content_renderer_client.h

```cpp

// ...
#include "demo/demo_mojo_v8/renderer/render_frame_observer.h"
#include "demo/demo_mojo_v8/renderer/v8_bindings_register.h"

class ChromeContentRendererClient
    : public content::ContentRendererClient,
      public service_manager::LocalInterfaceProvider,
      public demo::DemoRenderFrameObserver::Delegate {

          // ...

          public:
            	void DidCreateScriptContext(v8::Local<v8::Context> context,
                              content::RenderFrame* render_frame) override;
  		        void WillReleaseScriptContext(v8::Local<v8::Context> context,
                                content::RenderFrame* render_frame) override;

};

```

chrome/renderer/chrome_content_renderer_client.cc

```cpp

void ChromeContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
    
    // 创建Observer 并监听
    new demo::DemoRenderFrameObserver(render_frame, this);

    // ...

}

void ChromeContentRendererClient::DidCreateScriptContext(
    v8::Local<v8::Context> context,
    content::RenderFrame* render_frame) {
 
  if (!render_frame->IsMainFrame()) {
    return;
  }
  v8::Isolate* current_isolate = v8::Isolate::GetCurrent();
  v8::Isolate::Scope isolate_scope(current_isolate);
  v8::HandleScope scope(current_isolate);
  v8::Context::Scope context_scope(context);
    
  //这里注入我们所有的V8实现
  demo::V8BindingsRegister::RegisterAll(current_isolate);
}

void ChromeContentRendererClient::WillReleaseScriptContext(
    v8::Local<v8::Context> context,
    content::RenderFrame* render_frame) {}


```

#### 忽略lint错误
由于chromium自己的lint可能会组织我们编译，我们需要把demo加入到检查忽略列表中

.gn

``` python
# ...

no_check_targets = [
    "demo/demo_mojo_v8:*"
    #...
]

```

#### 测试页面
测试页面位于 test文件夹下，将index.html拖入浏览器中即可，最后请观察调试输出内容
