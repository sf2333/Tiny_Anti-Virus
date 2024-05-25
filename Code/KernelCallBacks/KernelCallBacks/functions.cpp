#include "public.h"
#include "functions.h"


#define DPRINT(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)



VOID UnicodeToChar(PUNICODE_STRING uniSource, CHAR *szDest ,size_t size)
{                                                  
	ANSI_STRING ansiTemp;                                
	RtlUnicodeStringToAnsiString(&ansiTemp,uniSource,TRUE);   
	if (size<=ansiTemp.Length)
	{
		RtlFreeAnsiString(&ansiTemp);
		*szDest = '\x00';
		return;
	}
	memcpy(szDest,ansiTemp.Buffer,ansiTemp.Length);
	szDest[ansiTemp.Length] = '\x00';
	RtlFreeAnsiString(&ansiTemp);
	return;
}




NTSTATUS BBSearchPattern( IN PUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound )
{
	ASSERT( ppFound != NULL && pattern != NULL && base != NULL );
	if (ppFound == NULL || pattern == NULL || base == NULL)
		return STATUS_INVALID_PARAMETER;

	for (ULONG_PTR i = 0; i < size - len; i++)
	{
		BOOLEAN found = TRUE;
		for (ULONG_PTR j = 0; j < len; j++)
		{
			if (pattern[j] != wildcard && pattern[j] != ((PUCHAR)base)[i + j])
			{
				found = FALSE;
				break;
			}
		}

		if (found != FALSE)
		{
			*ppFound = (PUCHAR)base + i;
			return STATUS_SUCCESS;
		}
	}

	return STATUS_NOT_FOUND;
}

//获取PreviousMode偏移
int getPreviousModeOffset()
{
	UNICODE_STRING routineName;
	PVOID routineAddress = NULL;
	PVOID pFind = NULL;
	ULONG_PTR findLength = 0x20;
	int offset = 0;
	RtlUnicodeStringInit(&routineName,L"ExGetPreviousMode");
	routineAddress = MmGetSystemRoutineAddress(&routineName);
	if (NULL == routineAddress)
	{
		return 0;
	}

#ifdef _WIN64
/*
win8
nt!PsGetCurrentThreadPreviousMode:
fffff800`872abfac 65488b042588010000 mov   rax,qword ptr gs:[188h]
fffff800`872abfb5 8a8032020000    mov     al,byte ptr [rax+232h]
fffff800`872abfbb c3              ret
win7
nt!ExGetPreviousMode:
fffff800`03efd100 65488b042588010000 mov   rax,qword ptr gs:[188h]
fffff800`03efd109 8a80f6010000    mov     al,byte ptr [rax+1F6h]

*/

	UCHAR pattern[] = "\xcc\xcc\x00\x00\xc3";
	NTSTATUS status = BBSearchPattern(pattern,0xCC,sizeof(pattern)-1,routineAddress,findLength,&pFind);
	if (STATUS_SUCCESS == status)
	{
		offset = *(int *)(pFind);
		return offset;
	}
//32位系统
#else
/*
nt!ExGetPreviousMode:
8087ab0e 64a124010000    mov     eax,dword ptr fs:[00000124h]
8087ab14 8a80d7000000    mov     al,byte ptr [eax+0D7h]
8087ab1a c3              ret

*/

	UCHAR pattern[] = "\xcc\xcc\x00\x00\xc3";
	NTSTATUS status = BBSearchPattern(pattern,0xCC,sizeof(pattern)-1,routineAddress,findLength,&pFind);
	if (STATUS_SUCCESS == status)
	{
		offset = *(int *)((PUCHAR)pFind);
		return offset;
	}
#endif
	
	return 0;
}

