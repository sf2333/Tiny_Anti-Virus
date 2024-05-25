#pragma once

#include "public.h"
#include <WinDef.h>

/*
typedef enum _MEMORY_INFORMATION_CLASS 
{ 
   MemoryBasicInformation, 
   MemoryWorkingSetList, 
   MemorySectionName 
}MEMORY_INFORMATION_CLASS; 

typedef struct _MEMORY_BASIC_INFORMATION {  
	PVOID       BaseAddress;  
	PVOID       AllocationBase;  
	DWORD       AllocationProtect;  
	SIZE_T      RegionSize;  
	DWORD       State;  
	DWORD       Protect;  
	DWORD       Type;  
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;  
*/

typedef 
NTSTATUS 
(WINAPI *ZWQUERYVIRTUALMEMORY) ( 
								IN HANDLE ProcessHandle, 
								IN PVOID BaseAddress, 
								IN MEMORY_INFORMATION_CLASS MemoryInformationClass, 
								OUT PVOID MemoryInformation, 
								IN ULONG MemoryInformationLength, 
								OUT PULONG ReturnLength OPTIONAL 
								);  

extern "C"
{


	NTKERNELAPI
		UCHAR *
		PsGetProcessImageFileName(
		__in PEPROCESS Process
		); 

	__declspec(dllimport)
	NTSTATUS 
		ZwAllocateVirtualMemory(
		__in HANDLE  ProcessHandle,
		__inout PVOID  *BaseAddress,
		__in ULONG_PTR  ZeroBits,
		__inout PSIZE_T  RegionSize,
		__in ULONG  AllocationType,
		__in ULONG  Protect
		); 


/*
	NTSTATUS
		ZwWriteVirtualMemory(
		IN HANDLE 
		ProcessHandle,
		IN PVOID 
		BaseAddress,
		IN PVOID 
		Buffer,
		IN ULONG 
		BufferLength,
		OUT PULONG 
		ReturnLength OPTIONAL
		);



	NTSTATUS 
		ZwFreeVirtualMemory(
		__in HANDLE  ProcessHandle,
		__inout PVOID  *BaseAddress,
		__inout PSIZE_T  RegionSize,
		__in ULONG  FreeType
		); 
*/

};

VOID UnicodeToChar(PUNICODE_STRING uniSource, CHAR *szDest,size_t size);
NTSTATUS BBSearchPattern( IN PUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound );
int getPreviousModeOffset();
int GetExportSsdtIndex(const char* ExportName);
PVOID GetSSDTEntry( IN ULONG index );
void Deinitialize();
NTSTATUS Initialize();
NTSTATUS initUndocumentData();
BOOL isNameInLogFile(PUNICODE_STRING fullName);
NTSTATUS GetProcessPathById(HANDLE ProcessId, 
				   PUNICODE_STRING *pProcessPath);