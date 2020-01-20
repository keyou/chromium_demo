#ifndef CONTENT_DEMO_SHELL_APP_DEMO_SHELL_CONTENT_MAIN_DELEGATE_H
#define CONTENT_DEMO_SHELL_APP_DEMO_SHELL_CONTENT_MAIN_DELEGATE_H

#include "content/public/app/content_main_delegate.h"
#include "base/macros.h"
#include "build/build_config.h"

namespace content{

class ContentClient;
class DemoShellContentBrowserClient;
class ShellContentUtilityClient;
class DemoShellContentRendererClient;

class DemoShellContentMainDelegate : public content::ContentMainDelegate {
public:

    DemoShellContentMainDelegate();

    ~DemoShellContentMainDelegate() override;

    //基本的初始化工作完成，此时可以安全创建某些单例和检查命令行工作
    bool BasicStartupComplete(int* exit_code) override;

    //Sandbox启动前的逻辑，主要用于初始化User Data目录和PDF模块
    void PreSandboxStartup() override;

    // 用于请求embedder启动一个进程。
    int RunProcess(
      const std::string& process_type,
      const MainFunctionParams& main_function_params) override;

    #if defined(OS_LINUX)
    void ZygoteForked() override;
    #endif

    //如下函数由DemoContentMainRunnerImpl中的ContentClientInitializer调用
    ContentBrowserClient* CreateContentBrowserClient() override;
    ContentGpuClient* CreateContentGpuClient() override;
    ContentRendererClient* CreateContentRendererClient() override;
    ContentUtilityClient* CreateContentUtilityClient() override;

    static void InitializeResourceBundle();

private:

    std::unique_ptr<DemoShellContentBrowserClient> browser_client_;
    std::unique_ptr<ContentGpuClient> gpu_client_;
    std::unique_ptr<DemoShellContentRendererClient> renderer_client_;
    std::unique_ptr<ContentUtilityClient> utility_client_;
    std::unique_ptr<ContentClient> content_client_;


    DISALLOW_COPY_AND_ASSIGN(DemoShellContentMainDelegate);

};

}


#endif //CONTENT_DEMO_SHELL_APP_DEMO_SHELL_MAIN_DELEGATE_H