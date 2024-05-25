#include "hide.h"

// 隐藏进程
ULONG64 ControlProcessHide(PEPROCESS EProc, ULONG Save_PID)
{
    ULONG64 OldPID;
    KIRQL OldIrql;

    /*
    //通过偏移获得额eprocess里的LIST_ENTRY
    PLIST_ENTRY ListEntry = (PLIST_ENTRY)((ULONG64)EProc + PROCESS_ACTIVE_PROCESS_LINKS_OFFSET);

    OldIrql = KeRaiseIrqlToDpcLevel();
    if (ListEntry->Flink != ListEntry && ListEntry->Blink != ListEntry && ListEntry->Blink->Flink == ListEntry && ListEntry->Flink->Blink == ListEntry)
    {
        ListEntry->Flink->Blink = ListEntry->Blink;
        ListEntry->Blink->Flink = ListEntry->Flink;
        ListEntry->Flink = ListEntry;
        ListEntry->Blink = ListEntry;
    }
    */

    //通过偏移获得额eprocess里的PID
    PULONG64 pid_ptr = (PULONG64)((ULONG64)EProc + PROCESS_Unique_ProcessId);
    DbgPrint("pid_ptr: %p pid: %d", pid_ptr, *pid_ptr);

    OldIrql = KeRaiseIrqlToDpcLevel();

    //判断是否已经隐藏了
    if (Save_PID == NULL)
    {
        //隐藏
        OldPID = *pid_ptr;   //保存原本的pid
        DbgPrint("oldpid: %d", OldPID);

        *pid_ptr = Fake_PID; //修改为假PID
        DbgPrint("修改后的pid: %d", *pid_ptr);

        KeLowerIrql(OldIrql);
        return OldPID;
    }
    else
    {
        //恢复
        *pid_ptr = Save_PID;
        DbgPrint("恢复后的pid: %d", *pid_ptr);
        KeLowerIrql(OldIrql);

        return 0;
    }

}
