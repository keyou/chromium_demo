#ifndef CONTENT_DEMO_SHELL_BROWSER_DEMO_SHELL_CONTENT_BROWSER_CLIENT_H_
#define CONTENT_DEMO_SHELL_BROWSER_DEMO_SHELL_CONTENT_BROWSER_CLIENT_H_

#include "content/public/browser/content_browser_client.h"


namespace content{

class DemoShellBrowserMainParts;
class DemoShellBrowserContext;

class DemoShellContentBrowserClient : public ContentBrowserClient {
public:
    static DemoShellContentBrowserClient* Get();

    DemoShellContentBrowserClient();
    ~DemoShellContentBrowserClient() override;

    // ContentBrowserClient overrides.
    std::unique_ptr<BrowserMainParts> CreateBrowserMainParts(
      const MainFunctionParams& parameters) override;

    DemoShellBrowserContext* browser_context();
    
private:
    DemoShellBrowserMainParts* demo_shell_browser_main_parts_ = nullptr;

};

}

#endif //CONTENT_DEMO_SHELL_BROWSER_DEMO_SHELL_CONTENT_BROWSER_CLIENT_H_