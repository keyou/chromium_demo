#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/path_service.h"
#include "base/task/thread_pool/thread_pool_instance.h"

#include <iostream>

// for resources
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"

// grit生成的头文件
#include "demo/demo_resources/grit/demo_gen_resources.h"
#include "demo/demo_resources/grit/demo_gen_resources_map.h"
#include "demo/demo_resources/grit/demo_gen_strings.h"
#include "demo/demo_resources/grit/demo_gen_strings_map.h"

void LoadResources(base::StringPiece resource_file, bool use_strings = false) {
  // 因为InitSharedInstanceWithxxx只能执行一次，因此这里先清除之前的数据
  if (ui::ResourceBundle::HasSharedInstance())
    ui::ResourceBundle::CleanupSharedInstance();

  // 初始化locale,也就是本地化/语言资源包
  // 会导致代码去加载
  // chrome_100_percent.pak，chrome_200_percent.pak，locale/xxx.pak
  // 前2个资源如果不存在的,会报WARNING.
  // 最后一个资源我本机是有的，如果你本地没有会崩溃。
  // 这里传入了空的语言类型，因为在Linux/Android等系统上这个值是没有用的，代码会从系统获取语言类型
  // 具体逻辑见 l10n_util::GetApplicationLocaleInternal
  // 实际项目中应该使用这个进行初始化
  // ui::ResourceBundle::InitSharedInstanceWithLocale(
  //     "", nullptr, ui::ResourceBundle::LOAD_COMMON_RESOURCES);

  // 加载数据资源包
  base::FilePath resource_path;
  if (base::PathService::Get(base::DIR_MODULE, &resource_path))
    resource_path = resource_path.AppendASCII(resource_file);
  ui::ResourceBundle::InitSharedInstanceWithPakPath(resource_path);

  ui::ResourceBundle& bundle = ui::ResourceBundle::GetSharedInstance();
  // 你也可以使用这个方法来加载其他的pak资源
  // SCALE_FATOR
  // 缩放因子，用来支持高分屏等情况，用户可以根据当前显示的情况选择使用拥有哪一个缩放因子的资源
  // 在初始化的时候，大多数情况会支持SCALE_FACTOR_100P和SCALE_FACTOR_200P，在IOS上还支持SCALE_FACTOR_300P
  // 具有相同ID并且SCALE_FATOR相同的资源只能有一个，否则会导致崩溃
  // bundle.AddDataPackFromPath(resource_path, ui::SCALE_FACTOR_NONE);

  std::cout << "Load: " << resource_file << std::endl;

  // 遍历demo_gen_resources.pak资源
  // text6会显示类似乱码的样子，因为grit使用了伪翻译，详见demo_resources.grd文件
  // 这里故意保持乱码以便读者注意这个问题
  for (size_t i = 0; i < kDemoGenResourcesSize; i++) {
    auto resource = kDemoGenResources[i];

    std::cout << resource.path << ": ["
              << bundle.GetRawDataResource(resource.id) << "]" << std::endl;
    // 读取语言内容
    // 本来应该读不到的，因为语言资源包里没有这些资源，但是内部代码为了单元测试
    // 添加了failback到数据资源包的逻辑，所以这里也能读取出来数据资源
    // 注意我本机是有 locale/xxx.pak 文件的(因为编译过content_shell)
    std::cout << resource.path << ": ["
              << l10n_util::GetStringUTF16(resource.id) << "]" << std::endl;
  }
  if (use_strings) {
    std::cout << "------------------------------------------" << std::endl;
    // 遍历demo_gen_strings.pak资源
    for (size_t i = 0; i < kDemoGenStringsSize; i++) {
      auto resource = kDemoGenStrings[i];
      std::cout << resource.path << ": ["
                << bundle.GetRawDataResource(resource.id) << "]"
                << std::endl;
      std::cout << resource.path << ": ["
                << l10n_util::GetStringUTF16(resource.id) << "]"
                << std::endl;
    }
  }
  std::cout << "==========================================" << std::endl;
}

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true, false, true, false);
  // 创建主消息循环
  base::SingleThreadTaskExecutor single_thread_task_executor;
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("Demo");

  // 提供 DIR_LOCALE 路径,用于寻找资源
  ui::RegisterPathProvider();

  LoadResources("gen/demo/demo_resources/grit/demo_gen_resources_en.pak");
  LoadResources("gen/demo/demo_resources/grit/demo_gen_resources_zh-CN.pak");
  LoadResources("gen/demo/demo_resources/grit/demo_gen_pak_en.pak", true);
  LoadResources("gen/demo/demo_resources/grit/demo_gen_pak_zh-CN.pak", true);

  LOG(INFO) << "running...";
  base::RunLoop().Run();
  return 0;
}
