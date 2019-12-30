#ifndef DEMO_IPC_MESSAGES_H_
#define DEMO_IPC_MESSAGES_H_

#include "ipc/ipc_message.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_param_traits.h"

#define IPC_MESSAGE_START IPCTestMsgStart

IPC_MESSAGE_CONTROL1(IPCTestMsg_Hello,std::string)
IPC_MESSAGE_CONTROL1(IPCTestMsg_Hi,std::string)

#endif //DEMO_IPC_MESSAGES_H_