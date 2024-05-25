#include "public.h"
#include "functions.h"


NTSTATUS readFile(PCWSTR filename,char *recvBuffer,ULONG bufferSize,PULONG readSize)
{
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK iostatus;
	HANDLE hfile;
	UNICODE_STRING logFileName;

	RtlUnicodeStringInit(&logFileName,filename);

	//初始化objectAttributes
	InitializeObjectAttributes(&objectAttributes,&logFileName,OBJ_CASE_INSENSITIVE,NULL,NULL);

	NTSTATUS status = ZwCreateFile(&hfile,
		GENERIC_READ,
		&objectAttributes,
		&iostatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);
	if (NT_SUCCESS(status))
	{
		status = ZwReadFile(hfile,NULL,NULL,NULL,&iostatus,recvBuffer,bufferSize,NULL,NULL);
		if (NT_SUCCESS(status))
		{
			*readSize = (ULONG)iostatus.Information;
		}
	}
	ZwClose(hfile);
	return status;
}

BOOL isNameInLogFile(PUNICODE_STRING fullName)
{
	NTSTATUS status;
	char buffer[1000];
	char szFullName[MAX_PATH];
	char *catName;
	ULONG size;
	UnicodeToChar(fullName,szFullName,MAX_PATH);

	//通过路径获取进程名字位置
	for (catName = szFullName+strlen(szFullName);catName>szFullName;catName--)
	{
		if (*catName == '\\')
		{
			catName++;
			break;
		}
	}
	status = readFile(L"\\??\\C:\\Program Files\\injectConfig.txt",buffer,sizeof(buffer),&size);

	if (NT_SUCCESS(status))
	{
		return (0!=strstr(buffer,catName));
	}
	return FALSE;
}
