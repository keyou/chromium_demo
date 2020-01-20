#ifndef CONTENT_DEMO_SHELL_BROWSER_DEMO_SHELL_BROWSER_MAIN_PARTS_H_
#define CONTENT_DEMO_SHELL_BROWSER_DEMO_SHELL_BROWSER_MAIN_PARTS_H_

#include "content/public/browser/browser_main_parts.h"
#include "content/public/common/main_function_params.h"
#include "base/macros.h"

#include "demo/demo_shell/browser/demo_shell_browser_context.h"

namespace content {

class DemoShellBrowserMainParts : public BrowserMainParts {
public:
    DemoShellBrowserMainParts(const MainFunctionParams& params);
    ~DemoShellBrowserMainParts() override;

    // BrowserMainParts overrides.
    int PreEarlyInitialization() override;
    int PreCreateThreads() override;
    void PreMainMessageLoopStart() override;
    void PostMainMessageLoopStart() override;
    void PreMainMessageLoopRun() override;
    bool MainMessageLoopRun(int* result_code) override;
    void PreDefaultMainMessageLoopRun(base::OnceClosure quit_closure) override;
    void PostMainMessageLoopRun() override;
    void PostDestroyThreads() override;

    DemoShellBrowserContext* browser_context() {
        return browser_context_.get();
    }

protected:
    virtual void InitializeBrowserContexts();
    virtual void InitializeMessageLoopContext();


private:
    const MainFunctionParams main_function_params_;
    bool run_message_loop_;

    std::unique_ptr<DemoShellBrowserContext> browser_context_;

    DISALLOW_COPY_AND_ASSIGN(DemoShellBrowserMainParts);
};

}

#endif //CONTENT_DEMO_SHELL_BROWSER_DEMO_SHELL_BROWSER_MAIN_PARTS_H_