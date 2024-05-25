#include "public.h"
#include "functions.h"
#include "globals.h"

UMDOCUMENT_DATA g_undocument_data;

PVOID getAddressByName(char *name)
{
	int index = 0;
	PVOID address = NULL;
	index = GetExportSsdtIndex(name);
	if (0!=index)
		address = GetSSDTEntry(index);
	if (NULL != address)
		return address;
	return NULL;
}
NTSTATUS initUndocumentData()
{
	PVOID address = NULL;
	g_undocument_data.PreviousMode = getPreviousModeOffset();
	if (0 == g_undocument_data.PreviousMode)
		return -1;

	Initialize();
	//get NtQueryVirtualMemory address
	address = getAddressByName("NtQueryVirtualMemory");
	if (NULL == address)
		return -1;
	g_undocument_data.NtQueryVirtualMemory = address;

	//get NtProtectVirtualMemory address
	address = getAddressByName("NtProtectVirtualMemory");
	if (NULL == address)
		return -1;
	g_undocument_data.NtProtectVirtualMemory = address;

	//finish
	Deinitialize();
	return STATUS_SUCCESS;
}