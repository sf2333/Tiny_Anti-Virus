#include "allocVM.h"
#include "nativeApi.h"
#include "globals.h"



PVOID AllocateInjectMemory(IN HANDLE ProcessHandle, IN PVOID DesiredAddress, IN SIZE_T DesiredSize)
{
	MEMORY_BASIC_INFORMATION mbi;
	SIZE_T AllocateSize = DesiredSize;
	ZWQUERYVIRTUALMEMORY ZwQueryVirtualMemory = (ZWQUERYVIRTUALMEMORY)g_undocument_data.NtQueryVirtualMemory ;

	if ((ULONG_PTR)DesiredAddress >= 0x70000000 && (ULONG_PTR)DesiredAddress < 0x80000000)
		DesiredAddress = (PVOID)0x70000000;

	while (1)
	{
		if (!NT_SUCCESS(ZwQueryVirtualMemory(ProcessHandle, DesiredAddress, MemoryBasicInformation, &mbi, sizeof(mbi), NULL)))
			return NULL;

		if (DesiredAddress != (PVOID)mbi.AllocationBase)
		{
			DesiredAddress = (PVOID)mbi.AllocationBase;
		}
		else
		{
			DesiredAddress = (PVOID)((ULONG_PTR)mbi.AllocationBase - 0x10000);
		}

		if (mbi.State == MEM_FREE)
		{
			if (NT_SUCCESS(ZwAllocateVirtualMemory(ProcessHandle, (PVOID *)&mbi.BaseAddress, 0, &AllocateSize, MEM_RESERVE, PAGE_EXECUTE_READWRITE)))
			{
				if (NT_SUCCESS(ZwAllocateVirtualMemory(ProcessHandle, (PVOID *)&mbi.BaseAddress, 0, &AllocateSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE)))
				{
					return (PVOID)mbi.BaseAddress;
				}
			}
		}
	}
	return NULL;
}