#include "base/bind.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"

// 1. 基本使用 使用函数的回调
void SayHello(const std::string& name) {
  LOG(INFO) << "Hello " << name << "!";
}
void BasicUsage() {
  // OnceCallback是一次性的回调,绑定处理函数对应BindOne
  base::OnceCallback<void(const std::string&)> callback =
      base::BindOnce(&SayHello);
  ;
  //使用时需要转移所有权，之后就不能再调用了
  std::move(callback).Run("Basic Usage");
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
    base::OnceCallback<void(const std::string&)> callback =
        base::BindOnce(&ClassUsage::SayHello, base::Unretained(this));

    std::move(callback).Run("ClassUsage");
    //下面的代码会报错：
    //error: static_assert failed due to requirement '!sizeof (*this)' "OnceCallback::Run() 
    //may only be invoked on a non-const rvalue, i.e. std::move(callback).Run()."
    //callback.Run("ClassUsage");
  }

 private:
  void SayHello(const std::string& name) {
    LOG(INFO) << "Hello " << name << "!";
  }
};

// 使用Lambda表达式
void LambdaUsage() {
  // Lambda
  base::OnceCallback<void(const std::string&)> callback = base::BindOnce(
      [](const std::string& name) { LOG(INFO) << "Hello " << name << "!"; });

  std::move(callback).Run("LambdaUsage");
}

// 更多的参数与与返回值
void MoreParamsAndReturnUsage() {
  // 这里我们修改刚刚的模板
  // 在拥有返回值后，无论是函数，方法，还是lambda记得返回相同类型的返回值
  base::OnceCallback<int(int, int)> callback =
      base::BindOnce([](int a, int b) { return a + b; });

  LOG(INFO) << "CallbackWithReturn "
            << "1+1=" << std::move(callback).Run(1, 1);
}
// Bind时捕获外部参数
void BindWithOutterParamsUsage() {
  int power = 3;

  // 可以在BindOnce后面增加外部参数，额外增加的参数，在回调参数列表内是倒序排列的
  // 例如 void(int) + 参数a + 参数b
  // 最后的处理函数中的顺序是 (b,a,runArgs);
  base::OnceCallback<void(int)> callback = base::BindOnce(
      [](int power, int source) {
        LOG(INFO) << "Power " << source << " With " << power << " is "
                  << power * source;
      },
      power);

  std::move(callback).Run(3);
}
// 闭包用例
void BindWithClosureUsage() {
  // 当函数签名为 void(void)时，我们可以使用闭包代替BindOnce<void()
  base::OnceClosure callback =
      base::BindOnce([]() { LOG(INFO) << "Hello Closure"; });

  std::move(callback).Run();
}

// 将callback作为参数进行传递
void TransmitUsage(base::OnceCallback<void(const std::string&)> callback) {
  std::move(callback).Run("Helo World!");
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

  // OnceCallback作为参数时，传递参数需要转移所有权
  base::OnceCallback<void(const std::string&)> cb = base::BindOnce(
      [](const std::string& msg) { LOG(INFO) << "TransmitUsage " << msg; });
  TransmitUsage(std::move(cb));

  // 创建单线程任务运行环境
  base::SingleThreadTaskExecutor main_task_executor;
  // 启动事件循环
  base::RunLoop().Run();

  return 0;
}