#ifndef DEMO_IPC_MESSAGES_H_
#define DEMO_IPC_MESSAGES_H_

#include "ipc/ipc_message.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_param_traits.h"

// 使用 IPCTestMsgStart 来测试，它不能随意命名，必须存在于 ipc/ipc_message_start.h 中
// 详情见 ipc/ipc_message_start.h 文件头的解释
// 关于 IPC 的介绍见 http://www.chromium.org/developers/design-documents/inter-process-communication
#define IPC_MESSAGE_START TestMsgStart

IPC_MESSAGE_CONTROL1(IPCTestMsg_Hello,std::string)
IPC_MESSAGE_CONTROL1(IPCTestMsg_Hi,std::string)

IPC_MESSAGE_ROUTED1(IPCTestMsg_RoutedHello,std::string)
IPC_MESSAGE_ROUTED1(IPCTestMsg_RoutedHi,std::string)

#endif //DEMO_IPC_MESSAGES_H_