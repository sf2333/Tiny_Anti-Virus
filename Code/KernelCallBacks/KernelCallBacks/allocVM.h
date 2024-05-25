#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include <ntifs.h>
#ifdef __cplusplus
}
#endif 
#include <windef.h>


typedef struct _MEMORY_BASIC_INFORMATION32 {
    DWORD BaseAddress;
    DWORD AllocationBase;
    DWORD AllocationProtect;
    DWORD RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
} MEMORY_BASIC_INFORMATION32, *PMEMORY_BASIC_INFORMATION32;

typedef struct DECLSPEC_ALIGN(16) _MEMORY_BASIC_INFORMATION64 {
    ULONGLONG BaseAddress;
    ULONGLONG AllocationBase;
    DWORD     AllocationProtect;
    DWORD     __alignment1;
    ULONGLONG RegionSize;
    DWORD     State;
    DWORD     Protect;
    DWORD     Type;
    DWORD     __alignment2;
} MEMORY_BASIC_INFORMATION64, *PMEMORY_BASIC_INFORMATION64;

#ifdef _WIN64
#define MEMORY_BASIC_INFORMATION MEMORY_BASIC_INFORMATION64
#define PMEMORY_BASIC_INFORMATION PMEMORY_BASIC_INFORMATION64
#else
#define MEMORY_BASIC_INFORMATION MEMORY_BASIC_INFORMATION32
#define PMEMORY_BASIC_INFORMATION PMEMORY_BASIC_INFORMATION32
#endif

/*
typedef enum _MEMORY_INFORMATION_CLASS
{
    MemoryBasicInformation,
    MemoryWorkingSetList,
    MemorySectionName
} MEMORY_INFORMATION_CLASS;
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

// º¯ÊýÉùÃ÷
extern "C"
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

PVOID AllocateInjectMemory(IN HANDLE ProcessHandle, IN PVOID DesiredAddress, IN SIZE_T DesiredSize);
