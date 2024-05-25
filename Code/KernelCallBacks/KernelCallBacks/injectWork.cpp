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

//ȫ�ֱ���
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

	//��ȡδ����������ַ������
	NTSTATUS status = initUndocumentData();

	if(STATUS_SUCCESS!=status)
	{
		KdPrint(("init fail! \n"));
		return -1;
	}
	DbgPrint("����ע��\r\n");
	return PsSetLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)NotifyRoutine);
}

VOID stopInject()
{
	PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)NotifyRoutine);
	DbgPrint("�ر�ע��\r\n"); 
}
template <class T>
BOOL isExeFile(T pNTHeader)
{
	//����dllע��
	if (0 != (pNTHeader->FileHeader.Characteristics&IMAGE_FILE_DLL))
		return FALSE;
	//�ж�exe�ļ�
	if ((pNTHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)||(pNTHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI))
		return TRUE;
	//����SYS�ļ�ע��

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
	IN PUNICODE_STRING    FullImageName, // ӳ���ȫ��
	IN HANDLE    ProcessId,  //ӳ��ӳ��Ľ��̵Ľ��� ID��������¼��ص�ӳ��������������˾��Ϊ�㡣
	IN PIMAGE_INFO    ImageInfo //ӳ��Ļ�����Ϣ
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

	//�ж��Ƿ�ΪPE�ļ�
	pDos = (PIMAGE_DOS_HEADER)BaseImage;
	if (0x5a4d != pDos->e_magic)
		return;

	//����32��64
	pHeader = (PIMAGE_NT_HEADERS32)((ULONG_PTR)BaseImage + pDos->e_lfanew);
	pHeader64 = (PIMAGE_NT_HEADERS64)pHeader;

	//�ж��Ƿ�Ϊ64λ����
	bIs64Bit = (pHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);

	//�ж��Ƿ�Ϊexe�ļ�
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

	//ֻ��ҪDevice��ͷ��·��ע��
	SIZE_T sameBytes = RtlCompareMemory(FullImageName->Buffer, L"\\Device\\", 16);
	if (16 != sameBytes) {
		return;
	}

	//unicode�ַ���תchar
	//UnicodeToChar(FullImageName, szFullImageName);

	//�ж�ProcessId�õ���·���ͻص������·���Ƿ���ͬ ��ֹ�޸ĵ�����ӳ��
	PUNICODE_STRING currentPath;
	ULONG Tag = 'pgaT';
	GetProcessPathById(ProcessId, &currentPath);
	ULONG uRe = RtlCompareUnicodeString(currentPath, FullImageName, TRUE);
	if (0 == uRe) {
		DbgPrint("ע�������: %wZ\n", FullImageName);
		addIAT(ProcessId, ImageInfo->ImageBase, g_dllName, g_apiName);
	}
	else
	{
		DbgPrint("[E0]����·������ͬ���: %wZ %wZ\n", currentPath, FullImageName);
	}
	ExFreePoolWithTag((PVOID)currentPath, Tag);
	return;

}

