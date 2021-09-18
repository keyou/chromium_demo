/***
 * base/memory/sheared_memory.h 在 fa7e8cc73e59e49c7ac12465dee031e7370ce1c0
 * 中被删除: [base] Remove base::SharedMemory and base::SharedMemoryHandle
 *
 * There are no users of the deprecated shared memory left, so its code
 * can safely removed.
 *
 * To use shared memory in Chrome, please refer to
 * base::WritableSharedMemoryRegion and base::ReadOnlySharedMemoryRegion
 * classes.
 *
 * 所以在这里只演示使用 base::WritableSharedMemoryRegion 和
 * base::ReadOnlySharedMemoryRegion
 */

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"

// For SharedMemoryTest
#include "base/memory/read_only_shared_memory_region.h"
#include "base/memory/writable_shared_memory_region.h"
#include "base/system/sys_info.h"

// For MemoryPressureTest
#include "base/memory/memory_pressure_monitor.h"
#if !defined(OS_LINUX)
#include "base/util/memory_pressure/multi_source_memory_pressure_monitor.h"
#endif

void SharedMemoryTest() {
  base::WritableSharedMemoryRegion memory =
      base::WritableSharedMemoryRegion::Create(10);
  LOG(INFO) << "memory size: " << memory.GetSize();
  {
    base::WritableSharedMemoryMapping mapping = memory.Map();
    void* buffer = mapping.memory();
    memset(buffer, 'a', 9);
    LOG(INFO) << "mapping size: " << mapping.size();
  }
  {
    int gra = base::SysInfo::VMAllocationGranularity();
    LOG(INFO) << "VMAllocationGranularity: " << gra;
    // MapAt 的 offset 必须对齐到 VMAllocationGranularity
    base::WritableSharedMemoryMapping mapping = memory.MapAt(0, 5);
    DCHECK(mapping.IsValid());
    char* buffer = mapping.GetMemoryAs<char>();
    LOG(INFO) << "mapping size: " << mapping.size() << ", data=" << buffer;
  }
  {
    // WritableSharedMemoryRegion
    // 可以转换为ReadOnly的，但是转换后原来的Region将不可用
    base::ReadOnlySharedMemoryRegion memory_read_only =
        base::WritableSharedMemoryRegion::ConvertToReadOnly(std::move(memory));
    // ReadOnlySharedMemoryRegion 是可以复制的
    base::ReadOnlySharedMemoryRegion memory_read_only2 =
        memory_read_only.Duplicate();

    base::ReadOnlySharedMemoryMapping mapping_read_only =
        memory_read_only.Map();
    DCHECK(mapping_read_only.IsValid());
    base::ReadOnlySharedMemoryMapping mapping_read_only2 =
        memory_read_only2.Map();
    DCHECK(mapping_read_only2.IsValid());
    LOG(INFO) << "mapping size: " << mapping_read_only.size()
              << ", data=" << mapping_read_only.GetMemoryAs<char>();
    LOG(INFO) << "mapping2 size: " << mapping_read_only2.size()
              << ", data2=" << mapping_read_only2.GetMemoryAs<char>();
  }
}

// 只支持ChromeOS,MAC,WIN，不支持linux
void MemoryPresureTest() {
  // https://source.chromium.org/chromium/chromium/src/+/master:content/browser/browser_main_loop.cc;l=367;drc=88c20a4027a89de88d6c559fe2ae49124a01ff8d
  // 初始化，要一直保活
  // 在新版本才有
#if defined(OS_LINUX)
  return;
#endif
// #if defined(OS_CHROMEOS)
//   if (chromeos::switches::MemoryPressureHandlingEnabled())
//     monitor = std::make_unique<util::MultiSourceMemoryPressureMonitor>();
// #elif defined(OS_MACOSX) || defined(OS_WIN) || defined(OS_FUCHSIA)
//   monitor = std::make_unique<util::MultiSourceMemoryPressureMonitor>();
// #else
//   return;
// #endif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"

  base::MemoryPressureMonitor* monitor = base::MemoryPressureMonitor::Get();
  base::MemoryPressureMonitor::MemoryPressureLevel level =
      monitor->GetCurrentPressureLevel();

  if (level == base::MemoryPressureMonitor::MemoryPressureLevel::
                   MEMORY_PRESSURE_LEVEL_CRITICAL) {
    LOG(INFO) << "应该释放内存";
    // 应该释放内存
  }

#pragma clang diagnostic pop
}

int main(int argc, char** argv) {
  // 类似C++的 atexit() 方法，用于管理程序的销毁逻辑，base::Singleton类依赖它
  base::AtExitManager at_exit;
  // 初始化CommandLine
  base::CommandLine::Init(argc, argv);
  // 设置日志格式
  logging::SetLogItems(true, false, true, false);

#if defined(OS_WIN)
  logging::LoggingSettings logging_setting;
  logging_setting.logging_dest = logging::LOG_TO_STDERR;
  logging::SetLogItems(true, true, false, false);
  logging::InitLogging(logging_setting);
#endif

  // 创建主消息循环
  base::SingleThreadTaskExecutor single_thread_task_executor;
  // 初始化线程池，会创建新的线程，在新的线程中会创建新消息循环MessageLoop
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams("Demo");

  SharedMemoryTest();
  // MemoryPresureTest();

  LOG(INFO) << "running...";
  base::RunLoop().Run();
  return 0;
}
