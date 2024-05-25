#include "uilt.h"

//不同版本的系统offset不同
#define PROCESS_ACTIVE_PROCESS_LINKS_OFFSET 0x2e8
//#define PROCESS_Unique_ProcessId 0x2e0

#define Fake_PID 4 //设置假PID为4


/*
nt!_EPROCESS
   +0x000 Pcb              : _KPROCESS
   +0x2d8 ProcessLock      : _EX_PUSH_LOCK
   +0x2e0 UniqueProcessId  : 0x00000000`00001994 Void
   +0x2e8 ActiveProcessLinks : _LIST_ENTRY [ 0xfffff804`80ac73c0 - 0xffffe389`c9c0c368 ]
   +0x2f8 RundownProtect   : _EX_RUNDOWN_REF
   +0x300 Flags2           : 0xd080
   +0x300 JobNotReallyActive : 0y0
*/


ULONG64 ControlProcessHide(PEPROCESS EProc, ULONG Save_PID);