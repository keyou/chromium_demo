#include "demo/demo_shell/browser/demo_shell_content_browser_client.h"
#include "demo/demo_shell/browser/demo_shell_browser_main_parts.h"
#include "demo/demo_shell/browser/demo_shell_browser_context.h"


namespace content{

DemoShellContentBrowserClient* g_demo_content_browser_client;
DemoShellContentBrowserClient* DemoShellContentBrowserClient::Get(){
    return g_demo_content_browser_client;
}

DemoShellContentBrowserClient::DemoShellContentBrowserClient()
:demo_shell_browser_main_parts_(nullptr){
    DCHECK(!g_demo_content_browser_client);
    g_demo_content_browser_client = this;
}

DemoShellContentBrowserClient::~DemoShellContentBrowserClient() {
    g_demo_content_browser_client = nullptr;
}

std::unique_ptr<BrowserMainParts> DemoShellContentBrowserClient::CreateBrowserMainParts(
    const MainFunctionParams& parameters) {
    auto parts = std::make_unique<DemoShellBrowserMainParts>(parameters);
    demo_shell_browser_main_parts_ = parts.get();
    return parts;
}

DemoShellBrowserContext* DemoShellContentBrowserClient::browser_context() {
    if(demo_shell_browser_main_parts_) {
        return demo_shell_browser_main_parts_->browser_context();
    }
    return nullptr;
}

}