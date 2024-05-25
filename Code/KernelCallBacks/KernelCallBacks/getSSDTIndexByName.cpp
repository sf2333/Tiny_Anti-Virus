#pragma once

#include "public.h"
#include "functions.h"

unsigned char* FileData = 0;
ULONG FileSize = 0;


static ULONG RvaToOffset(PIMAGE_NT_HEADERS pnth, ULONG Rva, ULONG FileSize)
{
#define PE_ERROR_VALUE (ULONG)-1
	PIMAGE_SECTION_HEADER psh = IMAGE_FIRST_SECTION(pnth);
	USHORT NumberOfSections = pnth->FileHeader.NumberOfSections;
	for(int i = 0; i < NumberOfSections; i++)
	{
		if(psh->VirtualAddress <= Rva)
		{
			if((psh->VirtualAddress + psh->Misc.VirtualSize) > Rva)
			{
				Rva -= psh->VirtualAddress;
				Rva += psh->PointerToRawData;
				return Rva < FileSize ? Rva : PE_ERROR_VALUE;
			}
		}
		psh++;
	}
	return PE_ERROR_VALUE;
}

ULONG GetExportOffset(const unsigned char* FileData, ULONG FileSize, const char* ExportName)
{
#define PE_ERROR_VALUE (ULONG)-1
	//Verify DOS Header
	PIMAGE_DOS_HEADER pdh = (PIMAGE_DOS_HEADER)FileData;
	if(pdh->e_magic != IMAGE_DOS_SIGNATURE)
	{
		KdPrint(("[TITANHIDE] Invalid IMAGE_DOS_SIGNATURE!\n"));
		return PE_ERROR_VALUE;
	}

	//Verify PE Header
	PIMAGE_NT_HEADERS pnth = (PIMAGE_NT_HEADERS)(FileData + pdh->e_lfanew);
	if(pnth->Signature != IMAGE_NT_SIGNATURE)
	{
		KdPrint(("[TITANHIDE] Invalid IMAGE_NT_SIGNATURE!\n"));
		return PE_ERROR_VALUE;
	}

	//Verify Export Directory
	PIMAGE_DATA_DIRECTORY pdd = NULL;
	if(pnth->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		pdd = ((PIMAGE_NT_HEADERS64)pnth)->OptionalHeader.DataDirectory;
	else
		pdd = ((PIMAGE_NT_HEADERS32)pnth)->OptionalHeader.DataDirectory;
	pnth->OptionalHeader.DataDirectory;
	ULONG ExportDirRva = pdd[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	ULONG ExportDirSize = pdd[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	ULONG ExportDirOffset = RvaToOffset(pnth, ExportDirRva, FileSize);
	if(ExportDirOffset == PE_ERROR_VALUE)
	{
		KdPrint(("[TITANHIDE] Invalid Export Directory!\n"));
		return PE_ERROR_VALUE;
	}

	//Read Export Directory
	PIMAGE_EXPORT_DIRECTORY ExportDir = (PIMAGE_EXPORT_DIRECTORY)(FileData + ExportDirOffset);
	ULONG NumberOfNames = ExportDir->NumberOfNames;
	ULONG AddressOfFunctionsOffset = RvaToOffset(pnth, ExportDir->AddressOfFunctions, FileSize);
	ULONG AddressOfNameOrdinalsOffset = RvaToOffset(pnth, ExportDir->AddressOfNameOrdinals, FileSize);
	ULONG AddressOfNamesOffset = RvaToOffset(pnth, ExportDir->AddressOfNames, FileSize);
	if(AddressOfFunctionsOffset == PE_ERROR_VALUE ||
		AddressOfNameOrdinalsOffset == PE_ERROR_VALUE ||
		AddressOfNamesOffset == PE_ERROR_VALUE)
	{
		KdPrint(("[TITANHIDE] Invalid Export Directory Contents!\n"));
		return PE_ERROR_VALUE;
	}
	ULONG* AddressOfFunctions = (ULONG*)(FileData + AddressOfFunctionsOffset);
	USHORT* AddressOfNameOrdinals = (USHORT*)(FileData + AddressOfNameOrdinalsOffset);
	ULONG* AddressOfNames = (ULONG*)(FileData + AddressOfNamesOffset);

	//Find Export
	ULONG ExportOffset = PE_ERROR_VALUE;
	for(ULONG i = 0; i < NumberOfNames; i++)
	{
		ULONG CurrentNameOffset = RvaToOffset(pnth, AddressOfNames[i], FileSize);
		if(CurrentNameOffset == PE_ERROR_VALUE)
			continue;
		const char* CurrentName = (const char*)(FileData + CurrentNameOffset);
		ULONG CurrentFunctionRva = AddressOfFunctions[AddressOfNameOrdinals[i]];
		if(CurrentFunctionRva >= ExportDirRva && CurrentFunctionRva < ExportDirRva + ExportDirSize)
			continue; //we ignore forwarded exports
		if(!strcmp(CurrentName, ExportName))  //compare the export name to the requested export
		{
			ExportOffset = RvaToOffset(pnth, CurrentFunctionRva, FileSize);
			break;
		}
	}

	if(ExportOffset == PE_ERROR_VALUE)
	{
		KdPrint(("[TITANHIDE] Export %s not found in export table!\n",ExportName));
	}

	return ExportOffset;
}

void* RtlAllocateMemory(bool InZeroMemory, SIZE_T InSize)
{
	void* Result = ExAllocatePoolWithTag(NonPagedPool, InSize, 'HIDE');
	if(InZeroMemory && (Result != NULL))
		RtlZeroMemory(Result, InSize);
	return Result;
}

void RtlFreeMemory(void* InPointer)
{
	ExFreePool(InPointer);
}

NTSTATUS Initialize()
{
	UNICODE_STRING FileName;
	OBJECT_ATTRIBUTES ObjectAttributes;
	RtlInitUnicodeString(&FileName, L"\\SystemRoot\\system32\\ntdll.dll");
	InitializeObjectAttributes(&ObjectAttributes, &FileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL, NULL);

	if(KeGetCurrentIrql() != PASSIVE_LEVEL)
	{
#ifdef _DEBUG
		DbgPrint("[TITANHIDE] KeGetCurrentIrql != PASSIVE_LEVEL!\n");
#endif
		return STATUS_UNSUCCESSFUL;
	}

	HANDLE FileHandle;
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS NtStatus = ZwCreateFile(&FileHandle,
		GENERIC_READ,
		&ObjectAttributes,
		&IoStatusBlock, NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, 0);
	if(NT_SUCCESS(NtStatus))
	{
		FILE_STANDARD_INFORMATION StandardInformation = { 0 };
		NtStatus = ZwQueryInformationFile(FileHandle, &IoStatusBlock, &StandardInformation, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
		if(NT_SUCCESS(NtStatus))
		{
			FileSize = StandardInformation.EndOfFile.LowPart;
			KdPrint(("[TITANHIDE] FileSize of ntdll.dll is %08X!\n", StandardInformation.EndOfFile.LowPart));
			FileData = (unsigned char*)RtlAllocateMemory(true, FileSize);

			LARGE_INTEGER ByteOffset;
			ByteOffset.LowPart = ByteOffset.HighPart = 0;
			NtStatus = ZwReadFile(FileHandle,
				NULL, NULL, NULL,
				&IoStatusBlock,
				FileData,
				FileSize,
				&ByteOffset, NULL);

			if(!NT_SUCCESS(NtStatus))
			{
				RtlFreeMemory(FileData);
				KdPrint(("[TITANHIDE] ZwReadFile failed with status %08X...\n", NtStatus));
			}
		}
		else
			KdPrint(("[TITANHIDE] ZwQueryInformationFile failed with status %08X...\n", NtStatus));
		ZwClose(FileHandle);
	}
	else
		KdPrint(("[TITANHIDE] ZwCreateFile failed with status %08X...\n", NtStatus));
	return NtStatus;
}

void Deinitialize()
{
	RtlFreeMemory(FileData);
}

int GetExportSsdtIndex(const char* ExportName)
{
	ULONG_PTR ExportOffset = GetExportOffset(FileData, FileSize, ExportName);
	if(ExportOffset == PE_ERROR_VALUE)
		return -1;

	int SsdtOffset = -1;
	unsigned char* ExportData = FileData + ExportOffset;
	for(int i = 0; i < 32 && ExportOffset + i < FileSize; i++)
	{
		if(ExportData[i] == 0xC2 || ExportData[i] == 0xC3)  //RET
			break;
		if(ExportData[i] == 0xB8)  //mov eax,X
		{
			SsdtOffset = *(int*)(ExportData + i + 1);
			break;
		}
	}

	if(SsdtOffset == -1)
	{
		KdPrint(("[TITANHIDE] SSDT Offset for %s not found...\n", ExportName));
	}
	return SsdtOffset;
}