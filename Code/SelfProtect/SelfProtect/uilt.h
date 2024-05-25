#include <ntifs.h>
#include <ntddk.h>


#define PROCESS_Unique_ProcessId 0x2e0

NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);
NTKERNELAPI CHAR* PsGetProcessImageFileName(PEPROCESS Process);

BOOLEAN bypass_signcheck(PDRIVER_OBJECT pDriverObject);

PEPROCESS GetProcessObjectByID(ULONG pid);

NTSTATUS
GetProcessImagePath(
    IN   ULONG    dwProcessId,
    OUT PUNICODE_STRING ProcessImagePath
);

