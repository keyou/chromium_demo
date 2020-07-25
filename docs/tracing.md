
# Trace and Perfetto

Update:

* 2020.5.22: Perfetto 的 [官网](https://perfetto.dev) 更新了，提供了更多 Perfetto 的文档；
* 2020.7.20: 添加了 TraceConfig 初始化时的注意事项；

> 文章基于 chromium v80.0.3987 版本的代码。

Trace 用来追踪程序的执行流程和执行效率，常用来进行调用流程追踪以及性能分析，它的功能有：

1. 追踪异步调用链；
2. 统计代码的执行耗时；
3. 可以反应程序调用堆栈；
4. Trace 数据是结构化的，可以被统计分析以及通过可视化工具进行展示；
5. 支持以线程为单位的 tracing, 各个线程相互独立；
6. 支持追踪进程的资源使用情况，并允许进行 memory dump；

Log 可以理解为简化版的 Trace, Trace 中可以包含 Log 数据，Trace 也可以通过 Log 来输出。

在 chromium 中 Trace 部分的代码分为三个部分：

1. base/trace_event: 由 base 库维护的 trace 基础模块，直接面向用户；
2. components/tracing: 辅助 tracing 初始化；
3. services/tracing: 将 perfetto 包装为 TracingService，支持多进程以及沙箱；

涉及到的文件约 224 个，代码量在 36K 左右。

``` md
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C++                            120           5271           2953          27734
C/C++ Header                    97           2253           3257           8018
Markdown                         6             22              0             48
Java                             1              6              6             17
-------------------------------------------------------------------------------
SUM:                           224           7552           6216          35817
-------------------------------------------------------------------------------
```

## Trace

chromium 在 base 库中提供了 `base::trace_event::TraceLog` 类，该类是 `TRACE_EVENT*` ， `TRACE_COUNTER*` 等宏的底层实现，关于这些宏定义见 `base/trace_event/common/trace_event_common.h` 。

使用 TraceLog 的流程：

1. 创建 TraceLog 的 配置 TraceConfig;
2. 开启 TraceLog： `base::trace_event::TraceLog::GetInstance()->SetEnabled(...)` ;
3. 使用 `TRACE_EVENT*` 等宏记录 Trace： `TRACE_EVENT0("test", "main")` ;
4. 如果配置了输出到 console, 则不需要后续步骤了；
5. 如果想要在程序中获取到 Trace 的结果可以使用： `base::trace_event::TraceLog::GetInstance()->Flush(...)` ;

当使用 `TRACE_EVENT(...)` 来记录 Trace 时，默认情况下 Trace 数据会记录到 TraceLog::logged_events_ 中，流程如下：

```c++
libbase.so!base::trace_event::TraceEvent::Reset(base::trace_event::TraceEvent * this, int thread_id, base::TimeTicks timestamp, base::ThreadTicks thread_timestamp, base::trace_event::ThreadInstructionCount thread_instruction_count, char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned long long bind_id, base::trace_event::TraceArguments * args, unsigned int flags) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_event_impl.cc:87)
libbase.so!base::trace_event::TraceLog::AddTraceEventWithThreadIdAndTimestamp(base::trace_event::TraceLog * this, char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned long long bind_id, int thread_id, const base::TimeTicks & timestamp, base::trace_event::TraceArguments * args, unsigned int flags) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_log.cc:1304)
libbase.so!trace_event_internal::AddTraceEventWithThreadIdAndTimestamp(char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned long long bind_id, int thread_id, const base::TimeTicks & timestamp, base::trace_event::TraceArguments * args, unsigned int flags) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_log.cc:1807)
trace_event_internal::AddTraceEventWithThreadIdAndTimestamp(char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, int thread_id, const base::TimeTicks & timestamp, unsigned int flags, unsigned long long bind_id) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_event.h:792)
trace_event_internal::AddTraceEvent(char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned int flags, unsigned long long bind_id) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_event.h:807)
main(int argc, char ** argv) (\media\keyou\dev2\chromium64\src\demo\demo_tracing\demo_tracing_console.cc:51)
```

TraceLog 只支持将 Trace 输出到 console 或者返回给到程序中，不支持写文件或者多进程，但是它提供了 hook 接口 `base::trace_event::TraceLog::SetAddTraceEventOverrides()` ，
允许外部 hook trace 的流程，从而实现自己的处理。在 chromium 中就使用了这种方式来支持多进程的 Trace。
如果有注册 trace hook，则在 `TraceLog::AddTraceEventWithThreadIdAndTimestamp` 中会将数据发给之前注册的 hook。

当用户调用 Flush 的时候会将 `TraceLog::logged_events_` 中的数据转换为 json, 处理流程如下：

```c++
libbase.so!base::trace_event::TraceEvent::AppendAsJSON(std::__Cr::basic_string<char, std::__Cr::char_traits<char>, std::__Cr::allocator<char> >*, base::RepeatingCallback<bool (char const*, char const*, base::RepeatingCallback<bool (char const*)>*)> const&) const(const base::trace_event::TraceEvent * this, std::__Cr::string * out, const base::trace_event::ArgumentFilterPredicate & argument_filter_predicate) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_event_impl.cc:157)
libbase.so!base::trace_event::TraceLog::ConvertTraceEventsToTraceFormat(std::__Cr::unique_ptr<base::trace_event::TraceBuffer, std::__Cr::default_delete<base::trace_event::TraceBuffer> >, base::RepeatingCallback<void (scoped_refptr<base::RefCountedString> const&, bool)> const&, base::RepeatingCallback<bool (char const*, char const*, base::RepeatingCallback<bool (char const*)>*)> const&)(std::__Cr::unique_ptr<base::trace_event::TraceBuffer, std::__Cr::default_delete<base::trace_event::TraceBuffer> > logged_events, const base::trace_event::TraceLog::OutputCallback & flush_output_callback, const base::trace_event::ArgumentFilterPredicate & argument_filter_predicate) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_log.cc:960)
libbase.so!base::trace_event::TraceLog::FinishFlush(base::trace_event::TraceLog * this, int generation, bool discard_events) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_log.cc:1016)
libbase.so!base::trace_event::TraceLog::FlushInternal(base::RepeatingCallback<void (scoped_refptr<base::RefCountedString> const&, bool)> const&, bool, bool)(base::trace_event::TraceLog * this, const base::trace_event::TraceLog::OutputCallback & cb, bool use_worker_thread, bool discard_events) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_log.cc:933)
libbase.so!base::trace_event::TraceLog::Flush(base::RepeatingCallback<void (scoped_refptr<base::RefCountedString> const&, bool)> const&, bool)(base::trace_event::TraceLog * this, const base::trace_event::TraceLog::OutputCallback & cb, bool use_worker_thread) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_log.cc:874)
main(int argc, char ** argv) (\media\keyou\dev2\chromium64\src\demo\demo_tracing\demo_tracing_console.cc:112)
```

直接使用 TraceLog 的 demo 见 `demo_tracing_console.cc` .

> 注意（2020.7.20）:  
> 创建 TraceConfig 对象时，使用1个参数的构造函数和2个参数的构造函数是不一样的，特别是第一个参数。
> 1个参数的构造函数，参数是json字符串格式的，2个参数的构造函数第一个参数表示categories，不是json格式的。
> 1个参数的构造函数如果传递了非json格式的字符串，将会启用所有的categories。

## TracingService/Perfetto

chromium 使用 TraceLog 的 hook 机制扩展了 TraceLog，
如果程序启动时添加了 `--trace-startup=...` 参数或者通过 `chrome://tracing` 来启动 trace，则会向 TraceLog 注册 3 个 TraceLog 的回调，
TraceEventDataSource::OnAddTraceEvent, TraceEventDataSource::FlushCurrentThread, TraceEventDataSource::OnUpdateDuration，流程如下：

```c++
libbase.so!base::trace_event::TraceLog::SetAddTraceEventOverrides(base::trace_event::TraceLog * this, const base::trace_event::TraceLog::AddTraceEventOverrideFunction & add_event_override, const base::trace_event::TraceLog::OnFlushFunction & on_flush_override, const base::trace_event::TraceLog::UpdateDurationFunction & update_duration_override) (/media/keyou/dev2/chromium64/src/base/trace_event/trace_log.cc:340)
libtracing_cpp.so!tracing::TraceEventDataSource::RegisterWithTraceLog(tracing::TraceEventDataSource * this) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/trace_event_data_source.cc:458)
libtracing_cpp.so!tracing::TraceEventDataSource::SetupStartupTracing(tracing::TraceEventDataSource * this, bool privacy_filtering_enabled) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/trace_event_data_source.cc:550)
libtracing_cpp.so!tracing::EnableStartupTracingIfNeeded() (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/trace_startup.cc:74)
main(int argc, char ** argv) (/media/keyou/dev2/chromium64/src/demo/demo_tracing/demo_tracing_perfetto.cc:160)
```

注册 hook 后，所有新的 Trace 都会被发送给 hook 函数，也就是 `tracing::TraceEventDataSource` 类，这个类由 `//services/tracing` 服务提供，用来将 TraceLog 对接到 `Perfetto` 。
Perfetto 是一个跨平台的 tracing 组件，包括一个可嵌入到其他程序中的 tracing 库以及用来可视化展示 trace 的工具 (<https://ui.perfetto.dev/#!/>和<chrome://tracing>)。
关于 Perfetto 的详细信息见 <https://perfetto.dev/#/>.

在 chromium 中，Perfetto 被包装进 TracingService，以便支持多进程的 Trace 追踪，TracingService 的代码在 `//services/tracing` 目录中。
直接使用 TracingService 的 demo 见 `demo_tracing_perfetto.cc` 。

下面的类图反应了 TracingService 是如何支持多进程的：

<iframe frameborder="0" style="width:100%;height:543px;" src="https://app.diagrams.net/?lightbox=1&highlight=0000ff&edit=_blank&layers=1&nav=1&title=tracing.drawio#Uhttps%3A%2F%2Fdrive.google.com%2Fuc%3Fid%3D19S-zR6bhDN24Hbmer4c9pWYvV_n2mbb-%26export%3Ddownload"></iframe>

本来 Perfetto 也是支持多进程的，但是是使用自己定义的 IPC 通信，chromium 使用 mojo 替换掉了原本的 IPC 机制，以便让它更贴近 chromium, 并且可以支持 chromium 的沙箱机制。

> 完整的类图可以在这里查看： <https://drive.google.com/file/d/19S-zR6bhDN24Hbmer4c9pWYvV_n2mbb-/view?usp=sharing>

### Trace Viewer

获取到的 Trace 数据可以使用 TraceViewer 进行可视化，在 chromium 中使用 `--trace-startup=...` 或者 `chrome://tracing` 来获得 Trace 文件后可以使用 `chrome://tracing` 或者 <https://ui.perfetto.dev/#!/> 来查看。
这两个 Trace Viewer 不仅支持 json 格式的 Trace 文件也支持 Android systrace 和 ftrace 格式。

如果你想使用 TraceViewer 但是又不想引入 Perfetto 的库，可以自己将数据生成为 chrome json trace event 格式，在 base 库中以及 swiftshader 中都采用了这种方式，因为这种 trace 数据格式并不复杂，例如以下格式就是合法的 Trace 格式：

```json
[{"pid":11988,"tid":11988,"ts":2194525525,"ph":"X","cat":"test","name":"main","args":{},"tdur":0,"tts":2922},
{"pid":11988,"tid":11988,"ts":2194525703,"ph":"X","cat":"testxxx,","name":"main","args":{},"tdur":0,"tts":3073}]
```

或者下面这种格式：

``` json
{"traceEvents":[
{"pid":11988,"tid":11988,"ts":2194525525,"ph":"X","cat":"test","name":"main","args":{},"tdur":0,"tts":2922},
{"pid":11988,"tid":11988,"ts":2194525703,"ph":"X","cat":"testxxx,","name":"main","args":{},"tdur":0,"tts":3073}]
```

TraceViewer 的一个例子：

![TraceViewer](2020-05-22-17-09-02.png)

完整的 Trace 格式介绍见： <https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit#>

### TracingService 的执行流程追踪

启动 TracingService 的时候会同时启动 TraceLog，如下：

```c++
libbase.so!base::trace_event::TraceLog::SetEnabled(base::trace_event::TraceLog * this, const base::trace_event::TraceConfig & trace_config, uint8_t modes_to_enable) (\media\keyou\dev2\chromium64\src\base\trace_event\trace_log.cc:571)
libtracing_cpp.so!tracing::TraceEventDataSource::StartTracingInternal(tracing::TraceEventDataSource * this, tracing::ProducerClient * producer, const perfetto::protos::gen::DataSourceConfig & data_source_config) (\media\keyou\dev2\chromium64\src\services\tracing\public\cpp\perfetto\trace_event_data_source.cc:719)
libtracing_cpp.so!tracing::TraceEventDataSource::StartTracing(tracing::TraceEventDataSource * this, tracing::ProducerClient * producer, const perfetto::protos::gen::DataSourceConfig & data_source_config) (\media\keyou\dev2\chromium64\src\services\tracing\public\cpp\perfetto\trace_event_data_source.cc:674)
libtracing_cpp.so!tracing::PerfettoTracedProcess::DataSourceBase::StartTracingWithID(tracing::TraceEventDataSource * this, uint64_t data_source_id, tracing::ProducerClient * producer, const perfetto::protos::gen::DataSourceConfig & data_source_config) (\media\keyou\dev2\chromium64\src\services\tracing\public\cpp\perfetto\perfetto_traced_process.cc:67)
libtracing_cpp.so!tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1::operator()(base::WeakPtr<tracing::ProducerClient>, tracing::PerfettoTracedProcess::DataSourceBase*, unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>) const(const class {...} * this, base::WeakPtr<tracing::ProducerClient> weak_ptr, tracing::TraceEventDataSource * data_source, perfetto::DataSourceInstanceID id, const perfetto::protos::gen::DataSourceConfig & data_source_config, tracing::mojom::ProducerClient::StartDataSourceCallback callback) (\media\keyou\dev2\chromium64\src\services\tracing\public\cpp\perfetto\producer_client.cc:186)
libtracing_cpp.so!base::internal::FunctorTraits<tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1, void>::Invoke<tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1, base::WeakPtr<tracing::ProducerClient>, tracing::PerfettoTracedProcess::DataSourceBase*, unsigned long, perfetto::protos::gen::DataSourceConfig, base::OnceCallback<void ()> >(tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1&&, base::WeakPtr<tracing::ProducerClient>&&, tracing::PerfettoTracedProcess::DataSourceBase*&&, unsigned long&&, perfetto::protos::gen::DataSourceConfig&&, base::OnceCallback<void ()>&&)(class {...} && functor, base::OnceCallback<void ()> && args, base::OnceCallback<void ()> && args, base::OnceCallback<void ()> && args, base::OnceCallback<void ()> && args, base::OnceCallback<void ()> && args) (\media\keyou\dev2\chromium64\src\base\bind_internal.h:385)
libtracing_cpp.so!base::internal::InvokeHelper<false, void>::MakeItSo<tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1, base::WeakPtr<tracing::ProducerClient>, tracing::PerfettoTracedProcess::DataSourceBase*, unsigned long, perfetto::protos::gen::DataSourceConfig, base::OnceCallback<void ()> >(tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1&&, base::WeakPtr<tracing::ProducerClient>&&, tracing::PerfettoTracedProcess::DataSourceBase*&&, unsigned long&&, perfetto::protos::gen::DataSourceConfig&&, base::OnceCallback<void ()>&&)(class {...} && functor, base::OnceCallback<void ()> && args, base::OnceCallback<void ()> && args, base::OnceCallback<void ()> && args, base::OnceCallback<void ()> && args, base::OnceCallback<void ()> && args) (\media\keyou\dev2\chromium64\src\base\bind_internal.h:598)
libtracing_cpp.so!base::internal::Invoker<base::internal::BindState<tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1, base::WeakPtr<tracing::ProducerClient>, tracing::PerfettoTracedProcess::DataSourceBase*, unsigned long, perfetto::protos::gen::DataSourceConfig, base::OnceCallback<void ()> >, void ()>::RunImpl<tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1, std::__Cr::tuple<base::WeakPtr<tracing::ProducerClient>, tracing::PerfettoTracedProcess::DataSourceBase*, unsigned long, perfetto::protos::gen::DataSourceConfig, base::OnceCallback<void ()> >, 0ul, 1ul, 2ul, 3ul, 4ul>(tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1&&, std::__Cr::tuple<base::WeakPtr<tracing::ProducerClient>, tracing::PerfettoTracedProcess::DataSourceBase*, unsigned long, perfetto::protos::gen::DataSourceConfig, base::OnceCallback<void ()> >&&, std::__Cr::integer_sequence<unsigned long, 0ul, 1ul, 2ul, 3ul, 4ul>)(class {...} && functor, std::__Cr::tuple<base::WeakPtr<tracing::ProducerClient>, tracing::PerfettoTracedProcess::DataSourceBase *, unsigned long, perfetto::protos::gen::DataSourceConfig, base::OnceCallback<void ()> > && bound) (\media\keyou\dev2\chromium64\src\base\bind_internal.h:671)
libtracing_cpp.so!base::internal::Invoker<base::internal::BindState<tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)::$_1, base::WeakPtr<tracing::ProducerClient>, tracing::PerfettoTracedProcess::DataSourceBase*, unsigned long, perfetto::protos::gen::DataSourceConfig, base::OnceCallback<void ()> >, void ()>::RunOnce(base::internal::BindStateBase*)(base::internal::BindStateBase * base) (\media\keyou\dev2\chromium64\src\base\bind_internal.h:640)
libtracing_cpp.so!base::OnceCallback<void ()>::Run() &&(base::OnceCallback<void ()> * this) (\media\keyou\dev2\chromium64\src\base\callback.h:98)
libtracing_cpp.so!tracing::PerfettoTracedProcess::CanStartTracing(tracing::PerfettoProducer*, base::OnceCallback<void ()>)(tracing::PerfettoTracedProcess * this, tracing::ProducerClient * producer, base::OnceCallback<void ()> start_tracing) (\media\keyou\dev2\chromium64\src\services\tracing\public\cpp\perfetto\perfetto_traced_process.cc:227)
libtracing_cpp.so!tracing::ProducerClient::StartDataSource(unsigned long, perfetto::protos::gen::DataSourceConfig const&, base::OnceCallback<void ()>)(tracing::ProducerClient * this, uint64_t id, const perfetto::protos::gen::DataSourceConfig & data_source_config, tracing::mojom::ProducerClient::StartDataSourceCallback callback) (\media\keyou\dev2\chromium64\src\services\tracing\public\cpp\perfetto\producer_client.cc:174)
libtracing_mojom.so!tracing::mojom::ProducerClientStubDispatch::AcceptWithResponder(tracing::ProducerClient * impl, mojo::Message * message, std::__Cr::unique_ptr<mojo::MessageReceiverWithStatus, std::__Cr::default_delete<mojo::MessageReceiverWithStatus> > responder) (\media\keyou\dev2\chromium64\src\out\debug\gen\services\tracing\public\mojom\perfetto_service.mojom.cc:1462)
libtracing_cpp.so!tracing::mojom::ProducerClientStub<mojo::RawPtrImplRefTraits<tracing::mojom::ProducerClient> >::AcceptWithResponder(tracing::mojom::ProducerClientStub<mojo::RawPtrImplRefTraits<tracing::mojom::ProducerClient> > * this, mojo::Message * message, std::__Cr::unique_ptr<mojo::MessageReceiverWithStatus, std::__Cr::default_delete<mojo::MessageReceiverWithStatus> > responder) (\media\keyou\dev2\chromium64\src\out\debug\gen\services\tracing\public\mojom\perfetto_service.mojom.h:507)
libbindings.so!mojo::InterfaceEndpointClient::HandleValidatedMessage(mojo::InterfaceEndpointClient * this, mojo::Message * message) (\media\keyou\dev2\chromium64\src\mojo\public\cpp\bindings\lib\interface_endpoint_client.cc:528)
libbindings.so!mojo::InterfaceEndpointClient::HandleIncomingMessageThunk::Accept(mojo::InterfaceEndpointClient::HandleIncomingMessageThunk * this, mojo::Message * message) (\media\keyou\dev2\chromium64\src\mojo\public\cpp\bindings\lib\interface_endpoint_client.cc:140)
libbindings.so!mojo::MessageDispatcher::Accept(mojo::MessageDispatcher * this, mojo::Message * message) (\media\keyou\dev2\chromium64\src\mojo\public\cpp\bindings\lib\message_dispatcher.cc:41)
libbindings.so!mojo::InterfaceEndpointClient::HandleIncomingMessage(mojo::InterfaceEndpointClient * this, mojo::Message * message) (\media\keyou\dev2\chromium64\src\mojo\public\cpp\bindings\lib\interface_endpoint_client.cc:356)
libbindings.so!mojo::internal::MultiplexRouter::ProcessIncomingMessage(mojo::internal::MultiplexRouter * this, mojo::internal::MultiplexRouter::MessageWrapper * message_wrapper, mojo::internal::MultiplexRouter::ClientCallBehavior client_call_behavior, base::internal::PooledSequencedTaskRunner * current_task_runner) (\media\keyou\dev2\chromium64\src\mojo\public\cpp\bindings\lib\multiplex_router.cc:883)
libbindings.so!mojo::internal::MultiplexRouter::Accept(mojo::internal::MultiplexRouter * this, mojo::Message * message) (\media\keyou\dev2\chromium64\src\mojo\public\cpp\bindings\lib\multiplex_router.cc:604)
```

首次收到 Trace，初始化 TraceWriter 用于记录 Trace:

```c++
libperfetto.so!perfetto::StartupTraceWriter::StartupTraceWriter(perfetto::StartupTraceWriter * this, std::__Cr::shared_ptr<perfetto::StartupTraceWriterRegistryHandle> registry_handle, perfetto::BufferExhaustedPolicy buffer_exhausted_policy, size_t max_buffer_size_bytes) (/media/keyou/dev2/chromium64/src/third_party/perfetto/src/tracing/core/startup_trace_writer.cc:364)
libperfetto.so!perfetto::StartupTraceWriterRegistry::CreateUnboundTraceWriter(perfetto::StartupTraceWriterRegistry * this, perfetto::BufferExhaustedPolicy buffer_exhausted_policy, size_t max_buffer_size_bytes) (/media/keyou/dev2/chromium64/src/third_party/perfetto/src/tracing/core/startup_trace_writer_registry.cc:64)
libtracing_cpp.so!tracing::TraceEventDataSource::CreateTraceWriterLocked(tracing::TraceEventDataSource * this) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/trace_event_data_source.cc:864)
libtracing_cpp.so!tracing::TraceEventDataSource::CreateThreadLocalEventSink(tracing::TraceEventDataSource * this, bool thread_will_flush) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/trace_event_data_source.cc:879)
libtracing_cpp.so!tracing::TraceEventDataSource::GetOrPrepareEventSink(bool thread_will_flush) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/trace_event_data_source.cc:514)
libtracing_cpp.so!tracing::TraceEventDataSource::OnAddTraceEvent<tracing::TraceEventDataSource::OnAddTraceEvent(base::trace_event::TraceEvent*, bool, base::trace_event::TraceEventHandle*)::$_3>(base::trace_event::TraceEvent*, bool, base::trace_event::TraceEventHandle*, tracing::TraceEventDataSource::OnAddTraceEvent(base::trace_event::TraceEvent*, bool, base::trace_event::TraceEventHandle*)::$_3)(base::trace_event::TraceEvent * trace_event, bool thread_will_flush, base::trace_event::TraceEventHandle * handle, class {...} func) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/trace_event_data_source.h:201)
libtracing_cpp.so!tracing::TraceEventDataSource::OnAddTraceEvent(TraceEvent * trace_event, bool thread_will_flush, base::trace_event::TraceEventHandle * handle) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/trace_event_data_source.cc:894)
libbase.so!base::trace_event::TraceLog::AddTraceEventWithThreadIdAndTimestamp(base::trace_event::TraceLog * this, char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned long long bind_id, int thread_id, const base::TimeTicks & timestamp, base::trace_event::TraceArguments * args, unsigned int flags) (/media/keyou/dev2/chromium64/src/base/trace_event/trace_log.cc:1260)
libbase.so!trace_event_internal::AddTraceEventWithThreadIdAndTimestamp(char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned long long bind_id, int thread_id, const base::TimeTicks & timestamp, base::trace_event::TraceArguments * args, unsigned int flags) (/media/keyou/dev2/chromium64/src/base/trace_event/trace_log.cc:1807)
trace_event_internal::AddTraceEventWithThreadIdAndTimestamp(char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, int thread_id, const base::TimeTicks & timestamp, unsigned int flags, unsigned long long bind_id) (/media/keyou/dev2/chromium64/src/base/trace_event/trace_event.h:792)
trace_event_internal::AddTraceEvent(char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned int flags, unsigned long long bind_id) (/media/keyou/dev2/chromium64/src/base/trace_event/trace_event.h:807)
main(int argc, char ** argv) (/media/keyou/dev2/chromium64/src/demo/demo_tracing/demo_tracing_perfetto_custom.cc:146)
```

将 Trace 保存到 TraceWriter:

```c++
libperfetto.so!perfetto::StartupTraceWriter::NewTracePacket(perfetto::StartupTraceWriter * this) (/media/keyou/dev2/chromium64/src/third_party/perfetto/src/tracing/core/startup_trace_writer.cc:457)
libtracing_cpp.so!tracing::TrackEventThreadLocalEventSink::DoResetIncrementalState(tracing::TrackEventThreadLocalEventSink * this, base::trace_event::TraceEvent * trace_event, bool explicit_timestamp) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/track_event_thread_local_event_sink.cc:719)
libtracing_cpp.so!tracing::TrackEventThreadLocalEventSink::ResetIncrementalStateIfNeeded(tracing::TrackEventThreadLocalEventSink * this, base::trace_event::TraceEvent * trace_event) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/track_event_thread_local_event_sink.cc:228)
libtracing_cpp.so!tracing::TrackEventThreadLocalEventSink::AddTraceEvent<tracing::TraceEventDataSource::OnAddTraceEvent(base::trace_event::TraceEvent*, bool, base::trace_event::TraceEventHandle*)::$_3>(base::trace_event::TraceEvent*, base::trace_event::TraceEventHandle*, tracing::TraceEventDataSource::OnAddTraceEvent(base::trace_event::TraceEvent*, bool, base::trace_event::TraceEventHandle*)::$_3)(tracing::TrackEventThreadLocalEventSink * this, base::trace_event::TraceEvent * trace_event, base::trace_event::TraceEventHandle * handle, class {...} arg_func) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/track_event_thread_local_event_sink.h:96)
libtracing_cpp.so!tracing::TraceEventDataSource::OnAddTraceEvent<tracing::TraceEventDataSource::OnAddTraceEvent(base::trace_event::TraceEvent*, bool, base::trace_event::TraceEventHandle*)::$_3>(base::trace_event::TraceEvent*, bool, base::trace_event::TraceEventHandle*, tracing::TraceEventDataSource::OnAddTraceEvent(base::trace_event::TraceEvent*, bool, base::trace_event::TraceEventHandle*)::$_3)(base::trace_event::TraceEvent * trace_event, bool thread_will_flush, base::trace_event::TraceEventHandle * handle, class {...} func) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/trace_event_data_source.h:205)
libtracing_cpp.so!tracing::TraceEventDataSource::OnAddTraceEvent(TraceEvent * trace_event, bool thread_will_flush, base::trace_event::TraceEventHandle * handle) (/media/keyou/dev2/chromium64/src/services/tracing/public/cpp/perfetto/trace_event_data_source.cc:894)
libbase.so!base::trace_event::TraceLog::AddTraceEventWithThreadIdAndTimestamp(base::trace_event::TraceLog * this, char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned long long bind_id, int thread_id, const base::TimeTicks & timestamp, base::trace_event::TraceArguments * args, unsigned int flags) (/media/keyou/dev2/chromium64/src/base/trace_event/trace_log.cc:1260)
libbase.so!trace_event_internal::AddTraceEventWithThreadIdAndTimestamp(char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned long long bind_id, int thread_id, const base::TimeTicks & timestamp, base::trace_event::TraceArguments * args, unsigned int flags) (/media/keyou/dev2/chromium64/src/base/trace_event/trace_log.cc:1807)
trace_event_internal::AddTraceEventWithThreadIdAndTimestamp(char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, int thread_id, const base::TimeTicks & timestamp, unsigned int flags, unsigned long long bind_id) (/media/keyou/dev2/chromium64/src/base/trace_event/trace_event.h:792)
trace_event_internal::AddTraceEvent(char phase, const unsigned char * category_group_enabled, const char * name, const char * scope, unsigned long long id, unsigned int flags, unsigned long long bind_id) (/media/keyou/dev2/chromium64/src/base/trace_event/trace_event.h:807)
main(int argc, char ** argv) (/media/keyou/dev2/chromium64/src/demo/demo_tracing/demo_tracing_perfetto_custom.cc:146)
```

结束 TracingSerivce 的时候会将 Trace 的结果转换为 Json 格式：

```c++
libperfetto.so!perfetto::trace_processor::json::(anonymous namespace)::TraceFormatWriter::WriteHeader(perfetto::trace_processor::json::(anonymous namespace)::TraceFormatWriter * this) (\media\keyou\dev2\chromium64\src\third_party\perfetto\src\trace_processor\export_json.cc:213)
libperfetto.so!perfetto::trace_processor::json::(anonymous namespace)::TraceFormatWriter::TraceFormatWriter(perfetto::trace_processor::json::OutputWriter*, std::__Cr::function<bool (char const*, char const*, std::__Cr::function<bool (char const*)>*)>, std::__Cr::function<bool (char const*)>, std::__Cr::function<bool (char const*)>)(perfetto::trace_processor::json::(anonymous namespace)::TraceFormatWriter * this, tracing::(anonymous namespace)::JsonStringOutputWriter * output, perfetto::trace_processor::json::ArgumentFilterPredicate argument_filter, perfetto::trace_processor::json::MetadataFilterPredicate metadata_filter, perfetto::trace_processor::json::LabelFilterPredicate label_filter) (\media\keyou\dev2\chromium64\src\third_party\perfetto\src\trace_processor\export_json.cc:101)
libperfetto.so!perfetto::trace_processor::json::ExportJson(perfetto::trace_processor::TraceStorage const*, perfetto::trace_processor::json::OutputWriter*, std::__Cr::function<bool (char const*, char const*, std::__Cr::function<bool (char const*)>*)>, std::__Cr::function<bool (char const*)>, std::__Cr::function<bool (char const*)>)(const perfetto::trace_processor::TraceStorage * storage, tracing::(anonymous namespace)::JsonStringOutputWriter * output, perfetto::trace_processor::json::ArgumentFilterPredicate argument_filter, perfetto::trace_processor::json::MetadataFilterPredicate metadata_filter, perfetto::trace_processor::json::LabelFilterPredicate label_filter) (\media\keyou\dev2\chromium64\src\third_party\perfetto\src\trace_processor\export_json.cc:1028)
libperfetto.so!perfetto::trace_processor::json::ExportJson(perfetto::trace_processor::TraceProcessorStorage*, perfetto::trace_processor::json::OutputWriter*, std::__Cr::function<bool (char const*, char const*, std::__Cr::function<bool (char const*)>*)>, std::__Cr::function<bool (char const*)>, std::__Cr::function<bool (char const*)>)(perfetto::trace_processor::TraceProcessorStorageImpl * tp, tracing::(anonymous namespace)::JsonStringOutputWriter * output, perfetto::trace_processor::json::ArgumentFilterPredicate argument_filter, perfetto::trace_processor::json::MetadataFilterPredicate metadata_filter, perfetto::trace_processor::json::LabelFilterPredicate label_filter) (\media\keyou\dev2\chromium64\src\third_party\perfetto\src\trace_processor\export_json.cc:1071)
tracing::ConsumerHost::TracingSession::ExportJson(tracing::ConsumerHost::TracingSession * this) (\media\keyou\dev2\chromium64\src\services\tracing\perfetto\consumer_host.cc:470)
tracing::ConsumerHost::TracingSession::OnTraceData(tracing::ConsumerHost::TracingSession * this, std::__Cr::vector<perfetto::TracePacket, std::__Cr::allocator<perfetto::TracePacket> > packets, bool has_more) (\media\keyou\dev2\chromium64\src\services\tracing\perfetto\consumer_host.cc:526)
tracing::ConsumerHost::OnTraceData(tracing::ConsumerHost * this, std::__Cr::vector<perfetto::TracePacket, std::__Cr::allocator<perfetto::TracePacket> > packets, bool has_more) (\media\keyou\dev2\chromium64\src\services\tracing\perfetto\consumer_host.cc:667)
libperfetto.so!perfetto::TracingServiceImpl::ReadBuffers(perfetto::TracingServiceImpl * this, perfetto::TracingSessionID tsid, perfetto::TracingServiceImpl::ConsumerEndpointImpl * consumer) (\media\keyou\dev2\chromium64\src\third_party\perfetto\src\tracing\core\tracing_service_impl.cc:1687)
libperfetto.so!perfetto::TracingServiceImpl::ConsumerEndpointImpl::ReadBuffers(perfetto::TracingServiceImpl::ConsumerEndpointImpl * this) (\media\keyou\dev2\chromium64\src\third_party\perfetto\src\tracing\core\tracing_service_impl.cc:2438)
tracing::ConsumerHost::TracingSession::OnTracingDisabled(tracing::ConsumerHost::TracingSession * this) (\media\keyou\dev2\chromium64\src\services\tracing\perfetto\consumer_host.cc:337)
tracing::ConsumerHost::OnTracingDisabled(tracing::ConsumerHost * this) (\media\keyou\dev2\chromium64\src\services\tracing\perfetto\consumer_host.cc:659)
libperfetto.so!perfetto::TracingServiceImpl::ConsumerEndpointImpl::NotifyOnTracingDisabled()::$_17::operator()() const(const class {...} * this) (\media\keyou\dev2\chromium64\src\third_party\perfetto\src\tracing\core\tracing_service_impl.cc:2390)
libperfetto.so!std::__Cr::__invoke<perfetto::TracingServiceImpl::ConsumerEndpointImpl::NotifyOnTracingDisabled()::$_17&>(class {...} & __f) (\media\keyou\dev2\chromium64\src\buildtools\third_party\libc++\trunk\include\type_traits:3529)
libperfetto.so!std::__Cr::__invoke_void_return_wrapper<void>::__call<perfetto::TracingServiceImpl::ConsumerEndpointImpl::NotifyOnTracingDisabled()::$_17&>(perfetto::TracingServiceImpl::ConsumerEndpointImpl::NotifyOnTracingDisabled()::$_17&)(class {...} & __args) (\media\keyou\dev2\chromium64\src\buildtools\third_party\libc++\trunk\include\__functional_base:348)
libperfetto.so!std::__Cr::__function::__default_alloc_func<perfetto::TracingServiceImpl::ConsumerEndpointImpl::NotifyOnTracingDisabled()::$_17, void ()>::operator()()(std::__Cr::__function::__default_alloc_func<(lambda at ../../third_party/perfetto/src/tracing/core/tracing_service_impl.cc:2388:26), void ()> * this) (\media\keyou\dev2\chromium64\src\buildtools\third_party\libc++\trunk\include\functional:1590)
libperfetto.so!std::__Cr::__function::__policy_invoker<void ()>::__call_impl<std::__Cr::__function::__default_alloc_func<perfetto::TracingServiceImpl::ConsumerEndpointImpl::NotifyOnTracingDisabled()::$_17, void ()> >(std::__Cr::__function::__policy_storage const*)(const std::__Cr::__function::__policy_storage * __buf) (\media\keyou\dev2\chromium64\src\buildtools\third_party\libc++\trunk\include\functional:2071)
libtracing_cpp.so!std::__Cr::__function::__policy_func<void ()>::operator()() const(const std::__Cr::__function::__policy_func<void ()> * this) (\media\keyou\dev2\chromium64\src\buildtools\third_party\libc++\trunk\include\functional:2203)
libtracing_cpp.so!std::__Cr::function<void ()>::operator()() const(const std::__Cr::function<void ()> * this) (\media\keyou\dev2\chromium64\src\buildtools\third_party\libc++\trunk\include\functional:2473)
libtracing_cpp.so!tracing::PerfettoTaskRunner::PostTask(std::__Cr::function<void ()>)::$_0::operator()(std::__Cr::function<void ()>) const(const class {...} * this, std::__Cr::function<void ()> task) (\media\keyou\dev2\chromium64\src\services\tracing\public\cpp\perfetto\task_runner.cc:52)
libtracing_cpp.so!base::internal::FunctorTraits<tracing::PerfettoTaskRunner::PostTask(std::__Cr::function<void ()>)::$_0, void>::Invoke<tracing::PerfettoTaskRunner::PostTask(std::__Cr::function<void ()>)::$_0, std::__Cr::function<void ()> >(tracing::PerfettoTaskRunner::PostTask(std::__Cr::function<void ()>)::$_0&&, std::__Cr::function<void ()>&&)(class {...} && functor, std::__Cr::function<void ()> && args) (\media\keyou\dev2\chromium64\src\base\bind_internal.h:385)
```

## 其他

tracing 不仅提供了记录 Trace 的功能，也支持 memory dump, 它可以用来分析内存相关问题，关于 memory dump 的更多细节留待以后补充。
如果大家感兴趣可以从 `base/trace_event/memory_dump_manager.h` 入手。

参考资料：

* [Perfetto - Performance instrumentation and tracing](https://android.googlesource.com/platform/external/perfetto/+/master/README.md)
* [Perfetto - Docs](https://perfetto.dev/#/?id=perfetto-performance-instrumentation-and-tracing)
* [Catapult - Contributing, quick version](https://chromium.googlesource.com/catapult/+/HEAD/tracing/README.md)
* [Trace Event Format - Google Docs](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit#)
* [Service-based model - Perfetto Tracing Docs](https://www.perfetto.dev/docs/concepts/service-model)
* [Life of a Perfetto tracing session - Perfetto Tracing Docs](https://www.perfetto.dev/docs/design-docs/life-of-a-tracing-session)
