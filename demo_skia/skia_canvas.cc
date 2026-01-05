
#include "demo/demo_skia/skia_canvas.h"

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/ref_counted_memory.h"
#include "base/task/single_thread_task_runner.h"
#include "base/trace_event/trace_log.h"

namespace demo {

SkiaCanvas::SkiaCanvas(gfx::AcceleratedWidget widget,int width,int height)
    : nativeWindow_(widget),
      width_(width),
      height_(height),
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

SkiaCanvas::~SkiaCanvas() = default;

void SkiaCanvas::InitializeOnRenderThread() {}

// Android 系统会控制触摸事件的频率在 60 pps
void SkiaCanvas::OnTouch(int action, float x, float y) {
  TRACE_EVENT0("shell", "SkiaCanvas::OnTouch");
  // std::stringstream ss;
  // ss << "[" << tag_ << "] OnTouch: action,x,y=" << action << ", " << x << ", "
  //    << y;
  // DLOG(INFO) << ss.str();
  render_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&SkiaCanvas::OnTouchOnRenderThread,
                                base::Unretained(this), action, x, y));
}

void SkiaCanvas::OnTouchOnRenderThread(int action, float x, float y) {
  TRACE_EVENT2("shell", "SkiaCanvas::OnTouchOnRenderThread","action",action,"x",x);
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
  // 用于在 chrome:://tracing 中显示 Vsync
  TRACE_EVENT0("shell", "VSYNC");
  TRACE_EVENT0("shell", "SkiaCanvas::OnRenderOnRenderThread");
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
                                       base::Milliseconds(16));

  auto now = base::TimeTicks::Now();
  if (frame_count_>0) {
    total_frame_time_ += now - last_frame_time_;
  }
  frame_count_++;
  last_frame_time_ = now;

  {
    TRACE_EVENT0("shell", "paint");
    auto paint_start_time = base::TimeTicks::Now();
    TRACE_EVENT0("shell", "BeginPaint");
    auto* canvas = BeginPaint();
    TRACE_EVENT0("shell", "draw");
    canvas->clear(background_);
    canvas->drawPath(skPath_, pathPaint_);
    auto point_count = skPath_.countPoints();
    SkPoint p;
    for(int i = 0;i < point_count;i++) {
      p = skPath_.getPoint(i);
      canvas->drawCircle(p,3,circlePaint_);
    }
    TRACE_EVENT0("shell", "OnPaint");
    OnPaint(canvas);
    total_paint_time_ += base::TimeTicks::Now() - paint_start_time;
  }

  {
    TRACE_EVENT0("shell", "swapbuffer");
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
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(&SkiaCanvas::ShowInfo, base::Unretained(this), ss.str()));
}

void SkiaCanvas::ShowInfo(std::string info) {
  TRACE_EVENT0("shell", "SkiaCanvas::ShowInfo");
  DLOG(INFO) << "[demo_android_skia] Info: " << info;
}

}  // namespace demo
