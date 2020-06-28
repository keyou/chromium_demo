# cc (Chrome Compositor)

> Viz 和 cc 相关的类图见： <https://app.diagrams.net/?lightbox=1&highlight=0000ff&edit=_blank&layers=1&nav=1&title=viz%2Bcc.drawio#Uhttps%3A%2F%2Fdrive.google.com%2Fuc%3Fid%3D1LW5d0GzlksSVtncDO1kpeHCVs3YD3rl0%26export%3Ddownload>  

TODO: 完善文档

cc 的核心调度入口在 `cc::Scheduler::ProcessScheduledActions()`，主要代码如下：

```C++
// 2020年1月(v80.0.3987.158),有删减
void Scheduler::ProcessScheduledActions() {
  SchedulerStateMachine::Action action;
  do {
    action = state_machine_.NextAction();
    switch (action) {
      case SchedulerStateMachine::Action::NONE:
        break;
      case SchedulerStateMachine::Action::SEND_BEGIN_MAIN_FRAME:
        // 绘制新帧，这会触发cc embedder的绘制(Paint)，比如views::View::Paint()或者blink::GraphicsLayer::Paint()
        client_->ScheduledActionSendBeginMainFrame(begin_main_frame_args_);
        break;
      case SchedulerStateMachine::Action::COMMIT: {
        // 执行提交，也就是将cc::Layer的内容复制到cc::LayerImpl中
        client_->ScheduledActionCommit();
        break;
      }
      case SchedulerStateMachine::Action::ACTIVATE_SYNC_TREE:
        // 执行同步，也就是将pending_tree的内容同步到active_tree
        client_->ScheduledActionActivateSyncTree();
        break;
      case SchedulerStateMachine::Action::DRAW_IF_POSSIBLE:
        // 执行submit，创建CompositorFrame并提交到DisplayCompsitor
        DrawIfPossible();
        break;
      case SchedulerStateMachine::Action::BEGIN_LAYER_TREE_FRAME_SINK_CREATION:
        // 初始化，请求创建LayerTreeFrameSink
        client_->ScheduledActionBeginLayerTreeFrameSinkCreation();
        break;
      case SchedulerStateMachine::Action::PREPARE_TILES:
        // 执行Raster，并且准备Tiles
        client_->ScheduledActionPrepareTiles();
        break;
      }
    }
  } while (action != SchedulerStateMachine::Action::NONE);

  // 用于设定一个Deadline，在该Deadline之前都允许触发SEND_BEGIN_MAIN_FRAME事件，在Deadline触发后就不再允许触发该事件。它会Post一个延迟的OnBeginImplFrameDeadline事件，如果该事件执行了表示已经到了Deadline（状态机进入INSIDE_DEADLINE状态），它会设置一些状态禁止触发BeginMainFrame，然后进行下一次调度，这些调度都是需要在Deadline状态才能执行的操作，可能调度到DRAW_IF_POSSIBLE，这会触发client提交CF到viz，也可能调度到其他状态比如PREPARE_TILES等，当然也可能调度到NONE状态，在进入Deadline之后的所有操作都完成之后，调度器会进入空闲状态IDLE，此时就算退出Deadline了，后续就允许触发SEND_BEGIN_MAIN_FRAME事件了。
  ScheduleBeginImplFrameDeadline();

  // 用于检测是否需要触发SEND_BEGIN_MAIN_FRAME事件，如果需要它会Post HandlePendingBeginFrame事件，该事件执行后会使状态机进入INSIDE_BEGIN_FRAME状态，随后调度器会在合适的时机触发SEND_BEGIN_MAIN_FRAME事件，这会调用client进行绘制，在client绘制完成之后会请求调度器执行提交COMMIT以及激活ACTIVATE_SYNC_TREE。
  PostPendingBeginFrameTask();

  // 用于检测当前是否处于IDLE状态，如果是则将自己从 BFS 移除，否则则将自己 加入BFS。如果调度器将自己从BFS上移除，则会进入”休眠状态“，此时不会调度器不会占用CPU，如果后续外部需要更新显示的画面，可以通过cc::Scheduler::SetNeedsBeginFrame()来激活调度器（注意cc模块外可以通过cc::LayerTreeHost::SetNeedsCommit()来间接调用它，如果在cc::Layer中，则可以直接调用它的SetNeedsCommit成员函数，如果cc::Layer没有任何变化，只调用SetNeddsCommit()则不会激活调度器，必须有实际的变更或者标记区域demaged）。
  StartOrStopBeginFrames();
}
```
