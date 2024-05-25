
#ifdef __cplusplus
extern "C" {
#endif
#include "public.h"
#include <Ntstrsafe.h>
#include <ntimage.h>

#ifdef __cplusplus
}; // extern "C"
#endif
#include "functions.h"

#include "getNtFunFromIndex.h"

#define  MAX_PATH 260

//=====================struct=================
typedef struct _SYSTEM_SERVICE_DESCRIPTOR_TABLE 
{
	PULONG_PTR ServiceTableBase;
	PULONG ServiceCounterTableBase;
	ULONG_PTR NumberOfServices;
	PUCHAR ParamTableBase;
} SYSTEM_SERVICE_DESCRIPTOR_TABLE, *PSYSTEM_SERVICE_DESCRIPTOR_TABLE;
typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;         // Not filled in
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

//=====================struct=================


//////////////////////////////////////////////////////////////////////////////////
////////////////////////////import////////////////////////////////////////////////

extern "C"
{

	NTSYSAPI
		PIMAGE_NT_HEADERS
		NTAPI
		RtlImageNtHeader( PVOID Base );



	NTSYSAPI
		NTSTATUS
		NTAPI
		ZwQuerySystemInformation(
		IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
		OUT PVOID SystemInformation,
		IN ULONG SystemInformationLength,
		OUT PULONG ReturnLength OPTIONAL 
		);
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////import////////////////////////////////////////////////
PVOID g_KernelBase = NULL;
ULONG g_KernelSize = 0;
PSYSTEM_SERVICE_DESCRIPTOR_TABLE g_SSDT = NULL;






/// <summary>
/// Get ntoskrnl base address
/// </summary>
/// <param name="pSize">Size of module</param>
/// <returns>Found address, NULL if not found</returns>
PVOID GetKernelBase( OUT PULONG pSize )
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG bytes = 0;
	PRTL_PROCESS_MODULES pMods = NULL;
	PVOID checkPtr = NULL;
	UNICODE_STRING routineName;

	// Already found
	if (g_KernelBase != NULL)
	{
		if (pSize)
			*pSize = g_KernelSize;
		return g_KernelBase;
	}

	RtlUnicodeStringInit( &routineName, L"NtOpenFile" );

	checkPtr = MmGetSystemRoutineAddress( &routineName );
	if (checkPtr == NULL)
		return NULL;

	// Protect from UserMode AV
	status = ZwQuerySystemInformation( SystemModuleInformation, 0, bytes, &bytes );
	if (bytes == 0)
	{
		DPRINT( "BlackBone: %s: Invalid SystemModuleInformation size\n", __FUNCTION__ );
		return NULL;
	}

	pMods = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag( NonPagedPool, bytes, BB_POOL_TAG );
	RtlZeroMemory( pMods, bytes );

	status = ZwQuerySystemInformation( SystemModuleInformation, pMods, bytes, &bytes );

	if (NT_SUCCESS( status ))
	{
		PRTL_PROCESS_MODULE_INFORMATION pMod = pMods->Modules;

		for (ULONG i = 0; i < pMods->NumberOfModules; i++)
		{
			// System routine is inside module
			if (checkPtr >= pMod[i].ImageBase &&
				checkPtr < (PVOID)((PUCHAR)pMod[i].ImageBase + pMod[i].ImageSize))
			{
				g_KernelBase = pMod[i].ImageBase;
				g_KernelSize = pMod[i].ImageSize;
				if (pSize)
					*pSize = g_KernelSize;
				break;
			}
		}
	}

	if (pMods)
		ExFreePoolWithTag( pMods, BB_POOL_TAG );

	return g_KernelBase;
}

/// <summary>
/// Gets SSDT base - KiServiceTable
/// </summary>
/// <returns>SSDT base, NULL if not found</returns>
PSYSTEM_SERVICE_DESCRIPTOR_TABLE GetSSDTBase()
{
	PUCHAR ntosBase = (PUCHAR)GetKernelBase( NULL );

	// Already found
	if (g_SSDT != NULL)
		return g_SSDT;

	if (!ntosBase)
		return NULL;

	PIMAGE_NT_HEADERS pHdr = RtlImageNtHeader( ntosBase );
	PIMAGE_SECTION_HEADER pFirstSec = (PIMAGE_SECTION_HEADER)(pHdr + 1);
	for (PIMAGE_SECTION_HEADER pSec = pFirstSec; pSec < pFirstSec + pHdr->FileHeader.NumberOfSections; pSec++)
	{
		// Non-paged, non-discardable, readable sections
		// Probably still not fool-proof enough...
		if (pSec->Characteristics & IMAGE_SCN_MEM_NOT_PAGED &&
			pSec->Characteristics & IMAGE_SCN_MEM_EXECUTE &&
			!(pSec->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) &&
			(*(PULONG)pSec->Name != 'TINI') &&
			(*(PULONG)pSec->Name != 'EGAP'))
		{
			PVOID pFound = NULL;

			// KiSystemServiceRepeat pattern
			// 			nt!KiSystemServiceRepeat:
			// 			fffff800`03edb772 4c8d15c7202300  lea     r10,[nt!KeServiceDescriptorTable (fffff800`0410d840)]
			// 			fffff800`03edb779 4c8d1d00212300  lea     r11,[nt!KeServiceDescriptorTableShadow (fffff800`0410d880)]

			UCHAR pattern[] = "\x4c\x8d\x15\xcc\xcc\xcc\xcc\x4c\x8d\x1d\xcc\xcc\xcc\xcc\xf7";
			NTSTATUS status = BBSearchPattern( pattern, 0xCC, sizeof( pattern ) - 1, ntosBase + pSec->VirtualAddress, pSec->Misc.VirtualSize, &pFound );
			if (NT_SUCCESS( status ))
			{
				g_SSDT = (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)((PUCHAR)pFound + *(PULONG)((PUCHAR)pFound + 3) + 7);//指令占7个字节。
				//DPRINT( "BlackBone: %s: KeSystemServiceDescriptorTable = 0x%p\n", __FUNCTION__, g_SSDT );
				return g_SSDT;
			}
		}
	}

	return NULL;
}

/// <summary>
/// Gets the SSDT entry address by index.
/// </summary>
/// <param name="index">Service index</param>
/// <returns>Found service address, NULL if not found</returns>
PVOID GetSSDTEntry( IN ULONG index )
{
	static PSYSTEM_SERVICE_DESCRIPTOR_TABLE pSSDT = NULL;
	UNICODE_STRING routineName;

	//get ssdt address
	if(!pSSDT)
	{
#ifndef _WIN64
		RtlInitUnicodeString(&routineName, L"KeServiceDescriptorTable");
		pSSDT = (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)MmGetSystemRoutineAddress(&routineName);
#else
		pSSDT = GetSSDTBase();
#endif
	}

	if (!pSSDT)
		return NULL;
	if (index > pSSDT->NumberOfServices)
		return NULL;
#ifndef _WIN64
	return (PVOID)*((PULONG_PTR)(index*4 + (ULONG_PTR)(pSSDT->ServiceTableBase)));
#else
	return (PUCHAR)pSSDT->ServiceTableBase + (((PLONG)pSSDT->ServiceTableBase)[index] >> 4);
#endif
}