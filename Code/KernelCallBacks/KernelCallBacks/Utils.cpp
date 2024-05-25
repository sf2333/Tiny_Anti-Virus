#include "functions.h"
#include "Utils.h"

pfn_ZwQueryInformationProcess g_pZwQueryInformationProcess;

NTSTATUS
GetProcessPathById(HANDLE ProcessId, 
				   PUNICODE_STRING *pProcessPath)

{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PEPROCESS pEprocess = NULL;
	HANDLE  hProcess = NULL;
	ULONG RetSize = 0;
	PUNICODE_STRING pPath = NULL;
	UNICODE_STRING ustFunName;
	ULONG Tag = 'pgaT';

	RtlInitUnicodeString(&ustFunName, L"ZwQueryInformationProcess");
	g_pZwQueryInformationProcess = (pfn_ZwQueryInformationProcess)MmGetSystemRoutineAddress(&ustFunName);
	do 
	{
		if (!g_pZwQueryInformationProcess)
		{
			break;
		}

		Status = PsLookupProcessByProcessId(( HANDLE )ProcessId, &pEprocess);
		if (NT_ERROR(Status))
		{
			pEprocess = NULL;
			break;
		}

		Status = ObOpenObjectByPointer(pEprocess, 
			OBJ_KERNEL_HANDLE,
			NULL,
			GENERIC_READ,
			NULL,
			KernelMode,
			&hProcess);
		if (NT_ERROR(Status))
		{
			hProcess = NULL;
			break;
		}
		Status = g_pZwQueryInformationProcess(hProcess,
			ProcessImageFileName,
			NULL,
			0,
			&RetSize);
		if (STATUS_INFO_LENGTH_MISMATCH != Status)
		{
			break;
		}

		pPath = (PUNICODE_STRING)ExAllocatePoolWithTag(NonPagedPool, RetSize, Tag);
		if (!pPath)
		{
			break;
		}
		Status = g_pZwQueryInformationProcess(hProcess,
			ProcessImageFileName,
			pPath,
			RetSize,
			&RetSize);
		if (NT_ERROR(Status))
		{
			break;
		}
//目前不需要DosName
		//VolumeDeviceToDosName(pPath);
		*pProcessPath = pPath;
	} while (FALSE);

	if (hProcess)
	{
		ZwClose(hProcess);
		hProcess = NULL;
	}

	if (pEprocess)
	{
		ObDereferenceObject(pEprocess);
		pEprocess = NULL;
	}

	if (NT_ERROR(Status) && pPath)
	{
		ExFreePoolWithTag(pPath, Tag);
		pPath = NULL;
	}

	return Status;

}

VOID
VolumeDeviceToDosName(PUNICODE_STRING  DeviceName)

{

	UNICODE_STRING driveLetterName;
	WCHAR          driveLetterNameBuf[256];
	WCHAR Volume;
	WCHAR DriLetter[3];
	UNICODE_STRING linkTarget;
	USHORT Len = 0;
	NTSTATUS Status = STATUS_SUCCESS;

	for (Volume = L'A'; Volume <= L'Z'; Volume++)
	{
		RtlInitEmptyUnicodeString(&driveLetterName,driveLetterNameBuf,sizeof(driveLetterNameBuf));
		RtlAppendUnicodeToString(&driveLetterName, L"\\??\\");
		DriLetter[0] = Volume;
		DriLetter[1] = L':';
		DriLetter[2] = 0;
		RtlAppendUnicodeToString(&driveLetterName,DriLetter);
		Status = QuerySymbolicLink(&driveLetterName, &linkTarget);
		if (NT_ERROR(Status))
		{
			continue;
		}
		if (RtlPrefixUnicodeString(&linkTarget, DeviceName, TRUE))
		{
			Len = linkTarget.Length;
			ExFreePoolWithTag(linkTarget.Buffer, 'mysq');
			break;
		}
		ExFreePoolWithTag(linkTarget.Buffer, 'mysq');
	}

	if (Len > 4)
	{
		DeviceName->Buffer += ((Len - 4) / sizeof(WCHAR));
		DeviceName->Length -= (Len - 4);
		DeviceName->Buffer[0] = Volume;
		DeviceName->Buffer[1] = L':';
	}
}

NTSTATUS 
QuerySymbolicLink(IN PUNICODE_STRING SymbolicLinkName,
				  OUT PUNICODE_STRING LinkTarget)                                  
{
	OBJECT_ATTRIBUTES ObjAttr = {0};
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	HANDLE Handle = NULL;
	ULONG Tag = 'sgaT';

	do 
	{
		LinkTarget->Buffer = NULL;
		InitializeObjectAttributes(&ObjAttr, SymbolicLinkName, OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, 0, 0);
		Status = ZwOpenSymbolicLinkObject(&Handle, GENERIC_READ, &ObjAttr);
		if (NT_ERROR(Status))
		{
			break;
		}

		LinkTarget->MaximumLength = 200*sizeof(WCHAR);
		LinkTarget->Length = 0;
		LinkTarget->Buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPool, LinkTarget->MaximumLength, Tag);
		if (!LinkTarget->Buffer)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlZeroMemory(LinkTarget->Buffer, LinkTarget->MaximumLength);
		Status = ZwQuerySymbolicLinkObject(Handle, LinkTarget, NULL);
	} while (FALSE);

	if (Handle)
	{
		ZwClose(Handle);
		Handle = NULL;
	}

	if (NT_ERROR(Status) && LinkTarget->Buffer)
	{
		ExFreePoolWithTag(LinkTarget->Buffer, Tag);
		LinkTarget->Buffer = NULL;
	}

	return Status;
}
