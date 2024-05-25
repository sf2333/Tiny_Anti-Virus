#include <ntddk.h>

typedef NTSTATUS 
(__stdcall *pfn_ZwQueryInformationProcess)(__in HANDLE ProcessHandle,
										   __in PROCESSINFOCLASS ProcessInformationClass,
										   __out_bcount(ProcessInformationLength) PVOID ProcessInformation,
										   __in ULONG ProcessInformationLength,
										   __out_opt PULONG ReturnLength);




VOID VolumeDeviceToDosName(PUNICODE_STRING  DeviceName);

NTSTATUS 
QuerySymbolicLink(IN PUNICODE_STRING SymbolicLinkName,
				  OUT PUNICODE_STRING LinkTarget);