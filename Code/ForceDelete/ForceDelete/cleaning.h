#include <ntddk.h>
#include <ntimage.h>
#include <ntdef.h>

// -------------------------------------------------------
// 引用微软结构
// -------------------------------------------------------
// 结构体定义
typedef struct _HANDLE_INFO
{
	UCHAR ObjectTypeIndex;
	UCHAR HandleAttributes;
	USHORT  HandleValue;
	ULONG GrantedAccess;
	ULONG64 Object;
	UCHAR Name[256];
} HANDLE_INFO, * PHANDLE_INFO;

HANDLE_INFO HandleInfo[1024];

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
	USHORT  UniqueProcessId;
	USHORT  CreatorBackTraceIndex;
	UCHAR ObjectTypeIndex;
	UCHAR HandleAttributes;
	USHORT  HandleValue;
	PVOID Object;
	ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG64 NumberOfHandles;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef enum _OBJECT_INFORMATION_CLASS
{
	ObjectBasicInformation,
	ObjectNameInformation,
	ObjectTypeInformation,
	ObjectAllInformation,
	ObjectDataInformation
} OBJECT_INFORMATION_CLASS, * POBJECT_INFORMATION_CLASS;

typedef struct _OBJECT_BASIC_INFORMATION
{
	ULONG                   Attributes;
	ACCESS_MASK             DesiredAccess;
	ULONG                   HandleCount;
	ULONG                   ReferenceCount;
	ULONG                   PagedPoolUsage;
	ULONG                   NonPagedPoolUsage;
	ULONG                   Reserved[3];
	ULONG                   NameInformationLength;
	ULONG                   TypeInformationLength;
	ULONG                   SecurityDescriptorLength;
	LARGE_INTEGER           CreationTime;
} OBJECT_BASIC_INFORMATION, * POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION
{
	UNICODE_STRING          TypeName;
	ULONG                   TotalNumberOfHandles;
	ULONG                   TotalNumberOfObjects;
	WCHAR                   Unused1[8];
	ULONG                   HighWaterNumberOfHandles;
	ULONG                   HighWaterNumberOfObjects;
	WCHAR                   Unused2[8];
	ACCESS_MASK             InvalidAttributes;
	GENERIC_MAPPING         GenericMapping;
	ACCESS_MASK             ValidAttributes;
	BOOLEAN                 SecurityRequired;
	BOOLEAN                 MaintainHandleCount;
	USHORT                  MaintainTypeList;
	POOL_TYPE               PoolType;
	ULONG                   DefaultPagedPoolCharge;
	ULONG                   DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;

typedef struct _KAPC_STATE
{
	LIST_ENTRY ApcListHead[2];
	PVOID Process;
	BOOLEAN KernelApcInProgress;
	BOOLEAN KernelApcPending;
	BOOLEAN UserApcPending;
}KAPC_STATE, * PKAPC_STATE;

typedef struct _OBJECT_HANDLE_FLAG_INFORMATION
{
	BOOLEAN Inherit;
	BOOLEAN ProtectFromClose;
}OBJECT_HANDLE_FLAG_INFORMATION, * POBJECT_HANDLE_FLAG_INFORMATION;

typedef struct _LDR_DATA_TABLE_ENTRY64
{
	LIST_ENTRY64 InLoadOrderLinks;
	LIST_ENTRY64 InMemoryOrderLinks;
	LIST_ENTRY64 InInitializationOrderLinks;
	ULONG64 DllBase;
	ULONG64 EntryPoint;
	ULONG64 SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY64 HashLinks;
	ULONG64 SectionPointer;
	ULONG64 CheckSum;
	ULONG64 TimeDateStamp;
	ULONG64 LoadedImports;
	ULONG64 EntryPointActivationContext;
	ULONG64 PatchInformation;
	LIST_ENTRY64 ForwarderLinks;
	LIST_ENTRY64 ServiceTagLinks;
	LIST_ENTRY64 StaticLinks;
	ULONG64 ContextInformation;
	ULONG64 OriginalBase;
	LARGE_INTEGER LoadTime;
} LDR_DATA_TABLE_ENTRY64, * PLDR_DATA_TABLE_ENTRY64;

// -------------------------------------------------------
// 导出函数定义
// -------------------------------------------------------

NTKERNELAPI NTSTATUS ObSetHandleAttributes
(
	HANDLE Handle,
	POBJECT_HANDLE_FLAG_INFORMATION HandleFlags,
	KPROCESSOR_MODE PreviousMode
);

NTKERNELAPI VOID KeStackAttachProcess
(
	PEPROCESS PROCESS,
	PKAPC_STATE ApcState
);

NTKERNELAPI VOID KeUnstackDetachProcess
(
	PKAPC_STATE ApcState
);

NTKERNELAPI NTSTATUS PsLookupProcessByProcessId
(
	IN HANDLE ProcessId,
	OUT PEPROCESS* Process
);

NTSYSAPI NTSTATUS NTAPI ZwQueryObject
(
	HANDLE  Handle,
	ULONG ObjectInformationClass,
	PVOID ObjectInformation,
	ULONG ObjectInformationLength,
	PULONG  ReturnLength OPTIONAL
);

NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation
(
	ULONG SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG  ReturnLength
);

NTSYSAPI NTSTATUS NTAPI ZwDuplicateObject
(
	HANDLE    SourceProcessHandle,
	HANDLE    SourceHandle,
	HANDLE    TargetProcessHandle OPTIONAL,
	PHANDLE   TargetHandle OPTIONAL,
	ACCESS_MASK DesiredAccess,
	ULONG   HandleAttributes,
	ULONG   Options
);

NTSYSAPI NTSTATUS NTAPI ZwOpenProcess
(
	PHANDLE       ProcessHandle,
	ACCESS_MASK     AccessMask,
	POBJECT_ATTRIBUTES  ObjectAttributes,
	PCLIENT_ID      ClientId
);

NTSYSAPI NTSTATUS ZwDeleteFile(
	POBJECT_ATTRIBUTES ObjectAttributes
);

#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004

#define MAX_PATH 260
typedef struct
{
	wchar_t nt_path[MAX_PATH];      //nt路径 
	wchar_t device_path[MAX_PATH];  //设备路径

}path_packet;
