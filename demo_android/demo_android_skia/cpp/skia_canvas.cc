
#include "demo/demo_android/demo_android_skia/cpp/skia_canvas.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/bind.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/memory/ref_counted_memory.h"
#include "base/message_loop/message_loop.h"
#include "base/task/post_task.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/trace_event/common/trace_event_common.h"
#include "base/trace_event/trace_event.h"
#include "base/trace_event/trace_log.h"
#include "demo/demo_android/demo_android_skia/cpp/skia_canvas_gl.h"
#include "demo/demo_android/demo_android_skia/cpp/skia_canvas_software.h"
#include "demo/demo_android/demo_android_skia/demo_android_skia_jni_headers/SkiaCanvas_jni.h"
#include "demo/demo_android/demo_android_skia/cpp/trace_android.h"

namespace demo_jni {

std::unique_ptr<base::MessageLoop> g_message_loop;
// 注意该程序会向以下文件写数据，请只在测试机上使用该程序
const base::FilePath::CharType kTraceFileName[] =
    FILE_PATH_LITERAL("/sdcard/trace_skia.json");
std::unique_ptr<base::File> g_trace_file;
int g_write_offset = 0;

void StartTrace();
void DemoMain() {
  // 初始化UI消息循环，在 Android 上它使用 ALooper_forThread 实现
  g_message_loop =
      std::make_unique<base::MessageLoop>(base::MessagePumpType::UI);
  g_trace_file = std::make_unique<base::File>(
      base::FilePath(kTraceFileName),
      base::File::FLAG_OPEN_ALWAYS | base::File::FLAG_WRITE | base::File::FLAG_OPEN_TRUNCATED);
  DCHECK(g_trace_file->IsValid());
  g_write_offset += g_trace_file->Write(g_write_offset, "[", 1);
  StartTrace();
  DCHECK(ScopedTrace::Initialize());
}

void StartTrace() {
  // 配置及启动 Trace
  base::trace_event::TraceConfig trace_config =
      base::trace_event::TraceConfig("shell");
  base::trace_event::TraceLog::GetInstance()->SetEnabled(
      trace_config, base::trace_event::TraceLog::RECORDING_MODE);
}

bool is_flushing_ = false;
void FlushTrace() {
  if (is_flushing_)
    return;
  is_flushing_ = true;
  DLOG(INFO) << "Flush trace start.";

  base::trace_event::TraceLog::GetInstance()->SetDisabled();
  base::trace_event::TraceLog::GetInstance()->Flush(base::BindRepeating(
      [](const scoped_refptr<base::RefCountedString>& events_str,
         bool has_more_events) {
        LOG(INFO) << std::endl << events_str->data();
        g_write_offset += g_trace_file->Write(
            g_write_offset, events_str->data().c_str(), events_str->size());
        g_write_offset += g_trace_file->Write(g_write_offset, ",\n", 2);
        if (!has_more_events) {
          StartTrace();
          g_write_offset += g_trace_file->Write(g_write_offset, "\n", 1);
          is_flushing_ = false;
          g_trace_file->Flush();
          DLOG(INFO) << "Flush trace stop.";
        }
      }));
}

static jlong JNI_SkiaCanvas_Init(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    const base::android::JavaParamRef<jobject>& surface,
    jboolean useGL) {
  DLOG(INFO) << "[demo_android_skia] JNI_SkiaCanvas_Init";
  if (!g_message_loop) {
    DemoMain();
  }
  if (useGL)
    return reinterpret_cast<intptr_t>(new SkiaCanvasGL(env, caller, surface));
  return reinterpret_cast<intptr_t>(
      new SkiaCanvasSoftware(env, caller, surface));
}

SkiaCanvas::SkiaCanvas(JNIEnv* env,
                       const base::android::JavaParamRef<jobject>& caller,
                       const base::android::JavaParamRef<jobject>& surface)
    : caller_(caller),
      nativeWindow_(ANativeWindow_fromSurface(env, surface.obj())),
      width_(ANativeWindow_getWidth(nativeWindow_)),
      height_(ANativeWindow_getHeight(nativeWindow_)),
      render_thread_("DemoRender") {
  DCHECK(nativeWindow_);
  circlePaint_.setAntiAlias(false);
  circlePaint_.setColor(SK_ColorRED);
  pathPaint_.setAntiAlias(false);
  pathPaint_.setColor(SK_ColorWHITE);
  pathPaint_.setStyle(SkPaint::kStroke_Style);
  pathPaint_.setStrokeWidth(strokeWidth_);
  DCHECK(render_thread_.Start());
  render_task_runner_ = render_thread_.task_runner();
  render_closure_ = base::BindRepeating(&SkiaCanvas::OnRenderOnRenderThread,
                                        base::Unretained(this));
  render_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&SkiaCanvas::InitializeOnRenderThread,
                                base::Unretained(this)));
}

void SkiaCanvas::InitializeOnRenderThread() {}

// Android 系统会控制触摸事件的频率在 60 pps
void SkiaCanvas::OnTouch(JNIEnv* env, int action, jfloat x, jfloat y) {
  TRACE_EVENT0("shell", "SkiaCanvas::OnTouch");
  ATRACE_CALL();
  std::stringstream ss;
  ss << "[" << tag_ << "] OnTouch: action,x,y=" << action << ", " << x << ", "
     << y;
  DLOG(INFO) << ss.str();
  render_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&SkiaCanvas::OnTouchOnRenderThread,
                                base::Unretained(this), action, x, y));
}

void SkiaCanvas::OnTouchOnRenderThread(int action, jfloat x, jfloat y) {
  TRACE_EVENT0("shell", "SkiaCanvas::OnTouchOnRenderThread");
  ATRACE_CALL();
  if (action == 0) {  // down
    skPath_.rewind();
    skPath_.moveTo(x, y);
    frame_count_ = 0;
    last_frame_time_ = base::TimeTicks();
    total_frame_time_ = base::TimeDelta();
    total_paint_time_ = base::TimeDelta();
    total_swap_time_ = base::TimeDelta();
    touch_count_ = 1;
    touch_start_time_ = base::TimeTicks::Now();
    total_touch_time_ = base::TimeDelta();
    SetNeedsRedraw(true);
  } else if (action == 2 || action == 1) {  // move or up
    skPath_.lineTo(x, y);
    total_touch_time_ = base::TimeTicks::Now() - touch_start_time_;
    touch_count_++;
    if (action == 1) {
      SetNeedsRedraw(false);
      ShowFrameRateOnRenderThread();
      render_task_runner_->PostDelayedTask(FROM_HERE,
                                           base::BindOnce(FlushTrace),
                                           base::TimeDelta::FromSeconds(1));
    }
  }
}

void SkiaCanvas::SetNeedsRedraw(bool need_redraw) {
  need_redraw_ = need_redraw;
  if (need_redraw && !is_drawing_) {
    if (render_task_runner_->RunsTasksInCurrentSequence())
      OnRenderOnRenderThread();
    else
      render_task_runner_->PostTask(FROM_HERE, render_closure_);
  }
}

void SkiaCanvas::OnRenderOnRenderThread() {
  TRACE_EVENT0("shell", "SkiaCanvas::OnRenderOnRenderThread");
  ATRACE_CALL();
  if (!need_redraw_) {
    is_drawing_ = false;
    frame_count_ = 0;
    return;
  }
  // 一旦开始就一直保持以16ms的间隔进行刷新
  //need_redraw_ = false;
  is_drawing_ = true;
  // 如果要测试 VSYNC 可以不添加延时
  render_task_runner_->PostDelayedTask(FROM_HERE, render_closure_,
                                       base::TimeDelta::FromMilliseconds(16));

  auto now = base::TimeTicks::Now();
  if (frame_count_>0) {
    total_frame_time_ += now - last_frame_time_;
  }
  frame_count_++;
  last_frame_time_ = now;

  {
    TRACE_EVENT0("shell", "paint");
    ATRACE_NAME("shell.paint");
    auto paint_start_time = base::TimeTicks::Now();
    auto canvas = BeginPaint();
    canvas->clear(background_);
    canvas->drawPath(skPath_, pathPaint_);
    OnPaint(canvas);
    total_paint_time_ += base::TimeTicks::Now() - paint_start_time;
  }

  {
    TRACE_EVENT0("shell", "swapbuffer");
    ATRACE_NAME("shell.swapbuffer");
    auto swap_start_time = base::TimeTicks::Now();
    SwapBuffer();
    total_swap_time_ += base::TimeTicks::Now() - swap_start_time;
  }
}

void SkiaCanvas::ShowFrameRateOnRenderThread() {
  std::stringstream ss;
  ss << tag_
     << " frame= " << (total_frame_time_ / frame_count_).InMilliseconds()
     << " ms,"
     << " paint= " << (total_paint_time_ / frame_count_).InMilliseconds()
     << " ms,"
     << " swap= " << (total_swap_time_ / frame_count_).InMilliseconds() << " ms"
     << " touch= " << (total_touch_time_ / touch_count_).InMilliseconds()
     << " ms";
  g_message_loop->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&SkiaCanvas::ShowInfo, base::Unretained(this), ss.str()));
}

void SkiaCanvas::ShowInfo(std::string info) {
  TRACE_EVENT0("shell", "SkiaCanvas::ShowInfo");
  ATRACE_CALL();
  DLOG(INFO) << "[demo_android_skia] Info: " << info;
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_SkiaCanvas_showInfo(env, caller_,
                           base::android::ConvertUTF8ToJavaString(env, info));
}

}  // namespace demo_jni