#include "uilt.h"


BOOLEAN bypass_signcheck(PDRIVER_OBJECT pDriverObject)
{
#ifdef _WIN64
    typedef struct _KLDR_DATA_TABLE_ENTRY
    {
        LIST_ENTRY listEntry;
        ULONG64 __Undefined1;
        ULONG64 __Undefined2;
        ULONG64 __Undefined3;
        ULONG64 NonPagedDebugInfo;
        ULONG64 DllBase;
        ULONG64 EntryPoint;
        ULONG SizeOfImage;
        UNICODE_STRING path;
        UNICODE_STRING name;
        ULONG   Flags;
        USHORT  LoadCount;
        USHORT  __Undefined5;
        ULONG64 __Undefined6;
        ULONG   CheckSum;
        ULONG   __padding1;
        ULONG   TimeDateStamp;
        ULONG   __padding2;
    } KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#else
    typedef struct _KLDR_DATA_TABLE_ENTRY
    {
        LIST_ENTRY listEntry;
        ULONG unknown1;
        ULONG unknown2;
        ULONG unknown3;
        ULONG unknown4;
        ULONG unknown5;
        ULONG unknown6;
        ULONG unknown7;
        UNICODE_STRING path;
        UNICODE_STRING name;
        ULONG   Flags;
    } KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#endif
    PKLDR_DATA_TABLE_ENTRY pLdrData = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
    pLdrData->Flags = pLdrData->Flags | 0x20;

    return TRUE;

}

typedef NTSTATUS(*QUERY_INFO_PROCESS) (
    __in HANDLE ProcessHandle,
    __in PROCESSINFOCLASS ProcessInformationClass,
    __out_bcount(ProcessInformationLength) PVOID ProcessInformation,
    __in ULONG ProcessInformationLength,
    __out_opt PULONG ReturnLength
    );

QUERY_INFO_PROCESS ZwQueryInformationProcess;

//通过进程ID获得eprocess
PEPROCESS GetProcessObjectByID(ULONG pid)
{

    NTSTATUS status;
    PEPROCESS ep;

    status = PsLookupProcessByProcessId((HANDLE)pid, &ep);
    if (NT_SUCCESS(status))
    {
        return ep;
    }

    return NULL;
}

NTSTATUS
GetProcessImagePath(
    IN   ULONG    dwProcessId,
    OUT PUNICODE_STRING ProcessImagePath
)
{
    NTSTATUS Status;
    HANDLE  hProcess;
    PEPROCESS pEprocess;
    ULONG  returnedLength;
    ULONG  bufferLength;
    PVOID  buffer;
    PUNICODE_STRING imageName;

    PAGED_CODE();  // this eliminates the possibility of the IDLE Thread/Process   

    if (NULL == ZwQueryInformationProcess) {

        UNICODE_STRING routineName;

        RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");

        ZwQueryInformationProcess =
            (QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&routineName);

        if (NULL == ZwQueryInformationProcess) {
            DbgPrint("[自我保护]Cannot resolve ZwQueryInformationProcess！\n");
        }
    }

    Status = PsLookupProcessByProcessId((HANDLE)dwProcessId, &pEprocess);
    if (!NT_SUCCESS(Status))
        return Status;

    Status = ObOpenObjectByPointer(pEprocess,           // Object   
        OBJ_KERNEL_HANDLE,   // HandleAttributes   
        NULL,                // PassedAccessState OPTIONAL   
        GENERIC_READ,        // DesiredAccess   
        *PsProcessType,      // ObjectType   
        KernelMode,          // AccessMode   
        &hProcess);
    if (!NT_SUCCESS(Status))
        return Status;


    //   
    // Step one - get the size we need   
    //   
    Status = ZwQueryInformationProcess(hProcess,
        ProcessImageFileName,
        NULL,  // buffer   
        0,  // buffer size   
        &returnedLength);


    if (STATUS_INFO_LENGTH_MISMATCH != Status) {

        return  Status;

    }

    //   
    // Is the passed-in buffer going to be big enough for us?    
    // This function returns a single contguous buffer model...   
    //   
    bufferLength = returnedLength - sizeof(UNICODE_STRING);

    if (ProcessImagePath->MaximumLength < bufferLength) {

        ProcessImagePath->Length = (USHORT)bufferLength;

        return  STATUS_BUFFER_OVERFLOW;

    }

    //   
    // If we get here, the buffer IS going to be big enough for us, so   
    // let's allocate some storage.   
    //   
    buffer = ExAllocatePoolWithTag(PagedPool, returnedLength, 'ipgD');

    if (NULL == buffer) {

        return  STATUS_INSUFFICIENT_RESOURCES;

    }

    //   
    // Now lets go get the data   
    //   
    Status = ZwQueryInformationProcess(hProcess,
        ProcessImageFileName,
        buffer,
        returnedLength,
        &returnedLength);

    if (NT_SUCCESS(Status)) {
        //   
        // Ah, we got what we needed   
        //   
        ProcessImagePath = (PUNICODE_STRING)buffer;

        //RtlCopyUnicodeString(ProcessImagePath, imageName);

    }

    ZwClose(hProcess);

    //   
    // free our buffer   
    //   
    
    //ExFreePool(buffer);

    //   
    // And tell the caller what happened.   
    //      
    return  Status;

}
