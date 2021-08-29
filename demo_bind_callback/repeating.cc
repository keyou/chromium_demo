#include "base/bind.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"

/**
 * 写在最前
 * RepeatingCallback和OnceCallback最大的区别在于可以被重复调用
 * 因此不需要进行所有权转移
 * 但是需要注意指针和内存问题
 */

// 1. 基本使用 使用函数的回调
void SayHello(const std::string& name) {
  LOG(INFO) << "Hello " << name << "!";
}
void BasicUsage() {
  base::RepeatingCallback<void(const std::string&)> callback =
      base::BindRepeating(&SayHello);
  ;
  //使用时需要转移所有权，之后就不能再调用了
  callback.Run("Basic Usage");
  callback.Run("Basic Usage Repeat");
}

// 2. 使用类的方法
class ClassUsage {
 public:
  ClassUsage() {}
  ~ClassUsage() {}

  void Run() {
    // 如果是调用当前类型内部的方法
    // 参数需要指定当前方法所在的类型实例
    // 这样除了指定当前类型，也可以指定其他类型的方法作为回调函数
    // base::Unretained函数表示当前回调不持有引用
    base::RepeatingCallback<void(const std::string&)> callback =
        base::BindRepeating(&ClassUsage::SayHello, base::Unretained(this));

    callback.Run("ClassUsage");
    callback.Run("Class Useage Repeat");
  }

 private:
  void SayHello(const std::string& name) {
    LOG(INFO) << "Hello " << name << "!";
  }
};

// 使用Lambda表达式
void LambdaUsage() {
  // Lambda
  base::RepeatingCallback<void(const std::string&)> callback =
      base::BindRepeating([](const std::string& name) {
        LOG(INFO) << "Hello " << name << "!";
      });

  callback.Run("LambdaUsage");
  callback.Run("LambdaUsage Repeat");
}

// 更多的参数与与返回值
void MoreParamsAndReturnUsage() {
  // 这里我们修改刚刚的模板
  // 在拥有返回值后，无论是函数，方法，还是lambda记得返回相同类型的返回值
  base::RepeatingCallback<int(int, int)> callback =
      base::BindRepeating([](int a, int b) { return a + b; });

  LOG(INFO) << "CallbackWithReturn "
            << "1+1=" << callback.Run(1, 1);
  LOG(INFO) << "CallbackWithReturn "
            << "2+2=" << callback.Run(2, 2);
}
// Bind时捕获外部参数
void BindWithOutterParamsUsage() {
  int power = 3;

  // 可以在BindRepeating后面增加外部参数，额外增加的参数，在回调参数列表内是倒序排列的
  // 例如 void(int) + 参数a + 参数b
  // 最后的处理函数中的顺序是 (b,a,runArgs);
  base::RepeatingCallback<void(int)> callback = base::BindRepeating(
      [](int power, int source) {
        LOG(INFO) << "Power " << source << " With " << power << " is "
                  << power * source;
      },
      power);

  callback.Run(3);
  callback.Run(4);
}
// 闭包用例
void BindWithClosureUsage() {
  // 当函数签名为 void(void)时，我们可以使用闭包代替BindRepeating<void()
  base::RepeatingClosure callback =
      base::BindRepeating([]() { LOG(INFO) << "Hello Closure"; });

  callback.Run();

  callback.Run();
}

// 将callback作为参数进行传递
void TransmitUsage(base::RepeatingCallback<void(const std::string&)> callback) {
  callback.Run("Helo World!");
  callback.Run("Thanks Keyou And All Group Members");
}

int main(int argc, char** argv) {
  // 设置日志
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_STDERR;
  logging::InitLogging(settings);
  logging::SetLogItems(true, true, true, false);

  // Demos
  BasicUsage();
  (new ClassUsage())->Run();
  LambdaUsage();
  MoreParamsAndReturnUsage();
  BindWithOutterParamsUsage();
  BindWithClosureUsage();

  // RepeatingCallback作为参数时，没有任何限制
  base::RepeatingCallback<void(const std::string&)> cb = base::BindRepeating(
      [](const std::string& msg) { LOG(INFO) << "TransmitUsage " << msg; });
  TransmitUsage(cb);
  TransmitUsage(cb);

  // 创建单线程任务运行环境
  base::SingleThreadTaskExecutor main_task_executor;
  // 启动事件循环
  base::RunLoop().Run();

  return 0;
}