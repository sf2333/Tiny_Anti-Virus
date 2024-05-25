#ifdef __cplusplus
extern "C" {
#endif

#include <ntifs.h>
#include <ntimage.h>
#include <string.h>

#ifdef __cplusplus
}; // extern "C"
#endif

#include "nativeApi.h"
#include "injectDLL.h"
#include "injectWork.h"
#include "functions.h"
#ifdef __cplusplus
namespace { // anonymous namespace to limit the scope of this global variable!
#endif
#ifdef __cplusplus
}; // anonymous namespace
#endif

//全局变量
char g_dllName[MAX_PATH];
char g_apiName[MAX_PATH];

NTSTATUS startInject(char *dllName,char *apiName)
{
	int iName =strlen(dllName);
	int iApiName = strlen(apiName);

	if ((iName>MAX_PATH-1)||(iApiName>MAX_PATH-1))
	{
		return -1;
	}
	RtlCopyBytes(g_dllName,dllName,iName+1);
	RtlCopyBytes(g_apiName,apiName,iApiName+1);

	//获取未导出函数地址等数据
	NTSTATUS status = initUndocumentData();

	if(STATUS_SUCCESS!=status)
	{
		KdPrint(("init fail! \n"));
		return -1;
	}
	DbgPrint("启动注入\r\n");
	return PsSetLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)NotifyRoutine);
}

VOID stopInject()
{
	PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)NotifyRoutine);
	DbgPrint("关闭注入\r\n"); 
}
template <class T>
BOOL isExeFile(T pNTHeader)
{
	//不对dll注入
	if (0 != (pNTHeader->FileHeader.Characteristics&IMAGE_FILE_DLL))
		return FALSE;
	//判断exe文件
	if ((pNTHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)||(pNTHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI))
		return TRUE;
	//不对SYS文件注入

	return FALSE;
}

VOID UnicodeToChar(PUNICODE_STRING uniSource, CHAR* szDest)
{
	ANSI_STRING ansiTemp;
	RtlUnicodeStringToAnsiString(&ansiTemp, uniSource, TRUE);

	strcpy(szDest, ansiTemp.Buffer);
	RtlFreeAnsiString(&ansiTemp);
}

VOID NotifyRoutine(
	IN PUNICODE_STRING    FullImageName, // 映像的全名
	IN HANDLE    ProcessId,  //映射映像的进程的进程 ID，但如果新加载的映像是驱动程序，则此句柄为零。
	IN PIMAGE_INFO    ImageInfo //映像的基本信息
)
{
	char szFullImageName[MAX_PATH] = { 0 };
	UCHAR* pFileName;
	PVOID BaseImage = NULL;
	PEPROCESS Process;
	NTSTATUS status;

	PIMAGE_DOS_HEADER pDos;
	PIMAGE_NT_HEADERS32 pHeader;
	PIMAGE_NT_HEADERS64 pHeader64;
	BOOL bIs64Bit;

	BaseImage = ImageInfo->ImageBase;

	//判断是否为PE文件
	pDos = (PIMAGE_DOS_HEADER)BaseImage;
	if (0x5a4d != pDos->e_magic)
		return;

	//区分32和64
	pHeader = (PIMAGE_NT_HEADERS32)((ULONG_PTR)BaseImage + pDos->e_lfanew);
	pHeader64 = (PIMAGE_NT_HEADERS64)pHeader;

	//判断是否为64位程序
	bIs64Bit = (pHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);

	//判断是否为exe文件
	BOOL bIsExeFile = FALSE;
	if (bIs64Bit) {
		bIsExeFile = isExeFile(pHeader64);
	}
	else {
		bIsExeFile = isExeFile(pHeader);
	}
	if (!bIsExeFile) {
		return;
	}

	//只对要Device开头的路径注入
	SIZE_T sameBytes = RtlCompareMemory(FullImageName->Buffer, L"\\Device\\", 16);
	if (16 != sameBytes) {
		return;
	}

	//unicode字符串转char
	//UnicodeToChar(FullImageName, szFullImageName);

	//判断ProcessId得到的路径和回调传入的路径是否相同 防止修改到错误映像
	PUNICODE_STRING currentPath;
	ULONG Tag = 'pgaT';
	GetProcessPathById(ProcessId, &currentPath);
	ULONG uRe = RtlCompareUnicodeString(currentPath, FullImageName, TRUE);
	if (0 == uRe) {
		DbgPrint("注入程序名: %wZ\n", FullImageName);
		addIAT(ProcessId, ImageInfo->ImageBase, g_dllName, g_apiName);
	}
	else
	{
		DbgPrint("[E0]出现路径不相同情况: %wZ %wZ\n", currentPath, FullImageName);
	}
	ExFreePoolWithTag((PVOID)currentPath, Tag);
	return;

}

