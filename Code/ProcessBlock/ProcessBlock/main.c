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

	//表示进程正在退出
	if (CreateInfo == NULL) {
		DbgPrint("[destory]Process[%d] is destoried by Process[%d] Thread[%d]", ProcessId, hParentId, hCurrentThreadId);
		return;
	}

	//表示进程创建
	else
	{
		hParentId = CreateInfo->CreatingThreadId.UniqueProcess; //创建新进程的进程id
		hParentThreadId = CreateInfo->CreatingThreadId.UniqueThread;  //创建新进程的线程id
		DbgPrint("[destory]Process[%d] is Created by Process[%d]", ProcessId, hParentId);
		DbgPrint("[destory]进程路径:%wZ\r\n", CreateInfo->ImageFileName);

		UCHAR* pszImageFileName = PsGetProcessImageFileName(Process);  //只能查询16个字节并且无法显示中文
		if (pszImageFileName)
			DbgPrint("[destory]进程名:%s\r\n", pszImageFileName);

		//判断是否是白名单

		if (strcmp((const char*)pszImageFileName, "TinyAnti-virus.exe") == 0)
		{
			DbgPrint("[destory]白名单进程放行:%s\r\n", pszImageFileName);
		}
		else
		{
			//直接设置启动失败
			CreateInfo->CreationStatus = STATUS_UNSUCCESSFUL;
			DbgPrint("[destory-block]成功拦截进程：%s\r\n", pszImageFileName);
		}


	}

	return;

}

void Unload(PDRIVER_OBJECT driver)
{
	UNREFERENCED_PARAMETER(driver);
	DbgPrint("[严格模式]关闭成功，恢复一般模式!");
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

	// 绕过签名检查
	// LINKER_FLAGS=/INTEGRITYCHECK
	bypass_signcheck(driver);
	driver->DriverUnload = Unload;

	do {
		UNICODE_STRING	uFunName = { 0 };
		RtlInitUnicodeString(&uFunName, L"PsSetCreateProcessNotifyRoutineEx");

		pPsSetCreateProcessNotifyRoutineEx = (PPsSetCreateProcessNotifyRoutineEx)MmGetSystemRoutineAddress(&uFunName);
		if (pPsSetCreateProcessNotifyRoutineEx == NULL) {
			DbgPrint("[严格模式]GetSetCreateProcessNotif Failed");
			break;
		}
		if (STATUS_SUCCESS != pPsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyEx, FALSE)) {
			DbgPrint("[严格模式]Register Process Notify Failed");
			break;
		}
		bRegister = TRUE;
		DbgPrint("[严格模式]启动成功，禁止所有进程启动!");

	} while (FALSE);
	return STATUS_SUCCESS;
}

