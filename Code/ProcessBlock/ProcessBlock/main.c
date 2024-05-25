#include <ntddk.h>

typedef NTSTATUS(*PPsSetCreateProcessNotifyRoutineEx)(
	_In_ PCREATE_PROCESS_NOTIFY_ROUTINE_EX NotifyRoutine,
	_In_ BOOLEAN Remove
	);

PPsSetCreateProcessNotifyRoutineEx pPsSetCreateProcessNotifyRoutineEx = NULL;
BOOLEAN	bRegister = FALSE;
UCHAR* PsGetProcessImageFileName(PEPROCESS EProcess);

VOID CreateProcessNotifyEx(
	_Inout_  PEPROCESS              Process,
	_In_     HANDLE                 ProcessId,
	_In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
	UNREFERENCED_PARAMETER(Process);
	UNREFERENCED_PARAMETER(ProcessId);

	HANDLE	hParentId = NULL;
	HANDLE	hParentThreadId = NULL;
	HANDLE	hCurrentThreadId = NULL;
	hCurrentThreadId = PsGetCurrentThreadId();

	//��ʾ���������˳�
	if (CreateInfo == NULL) {
		DbgPrint("[destory]Process[%d] is destoried by Process[%d] Thread[%d]", ProcessId, hParentId, hCurrentThreadId);
		return;
	}

	//��ʾ���̴���
	else
	{
		hParentId = CreateInfo->CreatingThreadId.UniqueProcess; //�����½��̵Ľ���id
		hParentThreadId = CreateInfo->CreatingThreadId.UniqueThread;  //�����½��̵��߳�id
		DbgPrint("[destory]Process[%d] is Created by Process[%d]", ProcessId, hParentId);
		DbgPrint("[destory]����·��:%wZ\r\n", CreateInfo->ImageFileName);

		UCHAR* pszImageFileName = PsGetProcessImageFileName(Process);  //ֻ�ܲ�ѯ16���ֽڲ����޷���ʾ����
		if (pszImageFileName)
			DbgPrint("[destory]������:%s\r\n", pszImageFileName);

		//�ж��Ƿ��ǰ�����

		if (strcmp((const char*)pszImageFileName, "TinyAnti-virus.exe") == 0)
		{
			DbgPrint("[destory]���������̷���:%s\r\n", pszImageFileName);
		}
		else
		{
			//ֱ����������ʧ��
			CreateInfo->CreationStatus = STATUS_UNSUCCESSFUL;
			DbgPrint("[destory-block]�ɹ����ؽ��̣�%s\r\n", pszImageFileName);
		}


	}

	return;

}

void Unload(PDRIVER_OBJECT driver)
{
	UNREFERENCED_PARAMETER(driver);
	DbgPrint("[�ϸ�ģʽ]�رճɹ����ָ�һ��ģʽ!");
	if (bRegister && pPsSetCreateProcessNotifyRoutineEx) {
		pPsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyEx, TRUE);
		bRegister = FALSE;
	}

}

BOOLEAN bypass_signcheck(PDRIVER_OBJECT pDriverObject)
{
#ifdef _WIN64
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG64 __Undefined1;
		ULONG64 __Undefined2;
		ULONG64 __Undefined3;
		ULONG64 NonPagedDebugInfo;
		ULONG64 DllBase;
		ULONG64 EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
		USHORT  LoadCount;
		USHORT  __Undefined5;
		ULONG64 __Undefined6;
		ULONG   CheckSum;
		ULONG   __padding1;
		ULONG   TimeDateStamp;
		ULONG   __padding2;
	} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#else
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG unknown1;
		ULONG unknown2;
		ULONG unknown3;
		ULONG unknown4;
		ULONG unknown5;
		ULONG unknown6;
		ULONG unknown7;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
	} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#endif
	PKLDR_DATA_TABLE_ENTRY pLdrData = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
	pLdrData->Flags = pLdrData->Flags | 0x20;

	return TRUE;

}


NTSTATUS DriverEntry(PDRIVER_OBJECT	driver, PUNICODE_STRING	RegPath)
{
	UNREFERENCED_PARAMETER(driver);
	UNREFERENCED_PARAMETER(RegPath);

	// �ƹ�ǩ�����
	// LINKER_FLAGS=/INTEGRITYCHECK
	bypass_signcheck(driver);
	driver->DriverUnload = Unload;

	do {
		UNICODE_STRING	uFunName = { 0 };
		RtlInitUnicodeString(&uFunName, L"PsSetCreateProcessNotifyRoutineEx");

		pPsSetCreateProcessNotifyRoutineEx = (PPsSetCreateProcessNotifyRoutineEx)MmGetSystemRoutineAddress(&uFunName);
		if (pPsSetCreateProcessNotifyRoutineEx == NULL) {
			DbgPrint("[�ϸ�ģʽ]GetSetCreateProcessNotif Failed");
			break;
		}
		if (STATUS_SUCCESS != pPsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyEx, FALSE)) {
			DbgPrint("[�ϸ�ģʽ]Register Process Notify Failed");
			break;
		}
		bRegister = TRUE;
		DbgPrint("[�ϸ�ģʽ]�����ɹ�����ֹ���н�������!");

	} while (FALSE);
	return STATUS_SUCCESS;
}

