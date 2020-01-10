#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/task/task_scheduler/task_scheduler.h"
#include "base/path_service.h"

#include "ui/base/ui_base_paths.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/l10n/l10n_util.h"

// grit生成的头文件
#include "demo/grit/demo_gen_resources.h"

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true,false,true,false);
  // 创建主消息循环
  base::MessageLoop message_loop;
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::TaskScheduler::CreateAndStartWithDefaultParams("Demo");

  // 提供 DIR_LOCALE 路径,用于寻找资源
  ui::RegisterPathProvider();
  
  // 初始化locale,也就是本地化/语言资源包
  // LOAD_COMMON_RESOURCES 会导致代码去加载 chrome_100_percent.pak，chrome_200_percent.pak
  // 这些资源如果不存在的,会报WARNING. 注意我本机是有 locale/xxx.pak 文件的
  // 这里传入了空的语言类型，因为在Linux/Android等系统上这个值是没有用的，代码会从系统获取语言类型
  // 具体逻辑见 l10n_util::GetApplicationLocaleInternal
  ui::ResourceBundle::InitSharedInstanceWithLocale(
      "", nullptr, ui::ResourceBundle::LOAD_COMMON_RESOURCES);
  
  ui::ResourceBundle& bundle = ui::ResourceBundle::GetSharedInstance();

  // 加载数据资源包
  base::FilePath resource_path;
  if (base::PathService::Get(base::DIR_MODULE, &resource_path))
    resource_path = resource_path.AppendASCII("gen/demo/grit/demo_gen_zh_resources.pak");
  bundle.AddDataPackFromPath(resource_path,ui::SCALE_FACTOR_NONE);

  // 读取资源内容
  base::StringPiece raw_text1 = bundle.GetRawDataResource(IDS_TEXT1);
  base::StringPiece raw_text2 = bundle.GetRawDataResource(IDS_TEXT2);

  // 读取语言内容
  // 本来应该读不到的，因为语言资源包里没有这些资源，但是内部代码为了单元测试
  // 添加了failback到数据资源包的逻辑，所以这里也能读取出来数据资源
  // 注意我本机是有 locale/xxx.pak 文件的(因为编译过content_shell)
  base::string16 l10n_text1 = l10n_util::GetStringUTF16(IDS_TEXT1);
  base::string16 l10n_text2 = l10n_util::GetStringUTF16(IDS_TEXT2);

  // text2会显示类似乱码的样子，因为grit使用了伪翻译，详见demo_resources.grd文件
  // 这里故意保持乱码以便读者注意这个问题
  LOG(INFO) <<"raw text1: ["<< raw_text1 <<"]"<< std::endl
            <<"raw text2: ["<< raw_text2 << "]"<< std::endl
            <<"l10n text1: [" << l10n_text1 << "]"<< std::endl
            <<"l10n text2: [" << l10n_text2 << "]";

  LOG(INFO) << "running...";
  base::RunLoop().Run();
  return 0;
}
