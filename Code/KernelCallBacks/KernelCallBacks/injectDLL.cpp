#include "public.h"
#include "injectDLL.h"
#include "nativeApi.h"
#include "allocVM.h"
#include "globals.h"

#define ULONG_PTR_SUB(a,b) ((ULONG)(((ULONG_PTR)(a))-((ULONG_PTR)(b))))


//���õ�ǰģʽΪ�ں�
//Nt����ֻ�����ں�ģʽ���ܵ���
char setKernelMode(char mode)
{
	char ret = 0;
	int offset = g_undocument_data.PreviousMode;
	void *pEthread = PsGetCurrentThread();

	ret = *((char *)pEthread+offset);

	*((char *)pEthread+offset) = mode;

	return ret;
/*
	__asm{
		push eax;
		mov eax,fs:0x124;//��ȡ_KTHREAD �ṹ���ַ

		add eax,0x140;//��ȡpreviousmodeλ��

		mov byte ptr [eax],0;

		pop eax;
	}*/
}

//��ȡ�������������ƫ��
template <class T>
ULONG getVirtualAddressForIAT(T pNTHeader,PULONG sizeOfSection)
{
	ULONG importDescRVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	int numOfSections = pNTHeader->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNTHeader);

	for (int i=0;i<numOfSections;i++)
	{
		if ((importDescRVA>=pSectionHeader->VirtualAddress)&&(importDescRVA<(pSectionHeader->SizeOfRawData+pSectionHeader->VirtualAddress)))
		{
			*sizeOfSection = pSectionHeader->Misc.VirtualSize == 0?pSectionHeader->SizeOfRawData:pSectionHeader->Misc.VirtualSize;
			return pSectionHeader->VirtualAddress;
		}
		pSectionHeader++;
	}
	return NULL;

}


HANDLE OpenProcess(HANDLE ProcessId)
{
	HANDLE		result		= NULL;
	PEPROCESS	pEProcess	= NULL;

	if (NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &pEProcess)))
	{
		ObOpenObjectByPointer(pEProcess, OBJ_KERNEL_HANDLE , NULL, PROCESS_ALL_ACCESS, NULL, KernelMode, &result);

		ObDereferenceObject(pEProcess);
	}

	return result;
}

//������Щ����ĵ�����С����׼ȷ����Ҫ�Լ������������ȡ��С��
ULONG getImportTableSize(PIMAGE_IMPORT_DESCRIPTOR pDes)
{
	ULONG i=0;
	while (pDes->Name != 0)
	{
		i++;
		pDes++;
	}
	return ++i;//�������ȫ0��
}

//�޸ĵ����-������
VOID addIAT (
			IN HANDLE    ProcessId, // ��ǰ����load_image�Ľ���
			IN PVOID    BaseImage,
			IN char*	DllName,
			IN char*	FunctionName
			)
{
	NTSTATUS ntStatus;
	PEPROCESS Process;
	KAPC_STATE ApcState;
	ULONG Attached = FALSE;
	PEPROCESS CurrentProcess;
	PIMAGE_IMPORT_DESCRIPTOR pImportNew;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	HANDLE hProcessHandle;
	SIZE_T allocSize = 0,tempSize = 0;
	IMAGE_IMPORT_DESCRIPTOR Add_ImportDesc;

	PIMAGE_IMPORT_BY_NAME pApiName;
	IMAGE_THUNK_DATA *pOriginalThunkData;
	IMAGE_THUNK_DATA *pFirstThunkData;
	PIMAGE_BOUND_IMPORT_DESCRIPTOR pBoundImport;
	char preMode = 0;
	LARGE_INTEGER procTimeout = { 0 };
	PIMAGE_DOS_HEADER pDos;
	PIMAGE_NT_HEADERS32 pHeader;
	PIMAGE_NT_HEADERS64 pHeader64;
	bool bIs64Bit;
	MyZwProtectVirtualMemory  g_NtProtectVirtualMemory = (MyZwProtectVirtualMemory)g_undocument_data.NtProtectVirtualMemory;

	//���ڴ洢��Ҫ��ӵ��µ������
	typedef struct 
	{
		char dllName[128];
		char apiName[128]; 
		DWORD thunk;
		DWORD thunkEnd;
		DWORD orgthunk;
		DWORD orgthunkEnd;
	}ExtData,*PExtData;

	if ((DllName == NULL)||(FunctionName) == NULL)
	{
		DbgPrint("DllName��FunctionNameδ����!");
		return;
	}
	if((sizeof(DllName)>128)||(sizeof(FunctionName)>128))
	{
		DbgPrint("DllName��FunctionName����!");
		return;
	}
	//�޸���ǰģʽΪ�ں�̬
	preMode = setKernelMode(0);
	PVOID lpBuffer = NULL;

	ntStatus = PsLookupProcessByProcessId(ProcessId,&Process);
	if (!NT_SUCCESS(ntStatus)) {
		return ;
	}

// �жϽ����Ƿ��Ѿ���ֹ
// 	if (KeWaitForSingleObject( Process, Executive, KernelMode, FALSE, &procTimeout ) == STATUS_WAIT_0)
// 	{
// 		KdPrint(( "Process is terminating. Abort\n"));
// 
// 		if (Process)
// 			ObDereferenceObject( Process );
// 
// 		return ;
// 	}


	PVOID ulBaseImage = BaseImage;// ���̻���ַ

	CurrentProcess = PsGetCurrentProcess();

	if (CurrentProcess != Process) {
		KeStackAttachProcess (Process, &ApcState);
		Attached = TRUE;
	}

	//��ȡpeͷ
	pDos = (PIMAGE_DOS_HEADER) ulBaseImage;
	pHeader = (PIMAGE_NT_HEADERS32)((ULONG_PTR)ulBaseImage+pDos->e_lfanew);
	pHeader64 = (PIMAGE_NT_HEADERS64)pHeader;

	//�ж��Ƿ�Ϊ64λ����
	bIs64Bit = ( pHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC );

	
	ULONG nImportDllCount = 1;//�޵������ntdll
	if(bIs64Bit)
		pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress + (ULONG_PTR)ulBaseImage);
	else
		pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress + (ULONG_PTR)ulBaseImage);

	if ( (ULONG_PTR)pImportDesc != (ULONG_PTR)ulBaseImage)
	{
		nImportDllCount =  getImportTableSize(pImportDesc);
	}


	PExtData pExtData = NULL;

	//��ȡ���
	hProcessHandle = OpenProcess(ProcessId);
	if(NULL == hProcessHandle)
	{
		ObDereferenceObject(Process);
		return ;
	}

	//����һ��������������ټ���һ���Լ��Ľṹ,ExtData��
	allocSize =sizeof(ExtData) + sizeof(IMAGE_IMPORT_DESCRIPTOR) * (nImportDllCount + 1);

	//���䵼���
	tempSize = allocSize;

	if (bIs64Bit)
		lpBuffer = (PVOID)((ULONG_PTR)BaseImage+pHeader64->OptionalHeader.SizeOfImage+2*allocSize+100000);//SizeOfImage
	//32λ������0x70000000�������ڴ�
	else
		lpBuffer = (PVOID)0x70000000;
	lpBuffer = AllocateInjectMemory(hProcessHandle,lpBuffer,tempSize);

	//DbgPrint("New IAT Buffer Address: %p!", lpBuffer);
/*
	ntStatus = ZwAllocateVirtualMemory(hProcessHandle, &lpBuffer, 0, (PSIZE_T)&tempSize,
		MEM_COMMIT|MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE);*/

	//ƫ��Ϊ����
	if((ULONG_PTR)ulBaseImage>(ULONG_PTR)lpBuffer) 
	{
		KdPrint(("Can't allocate suite memory! \n"));
		ZwClose(hProcessHandle);
		return ;
	}

	//ָ���µ�������Ķ������ݣ����ڴ��dll��������������Ϣ��
	pExtData = (PExtData)((char *)lpBuffer + sizeof(IMAGE_IMPORT_DESCRIPTOR) * (nImportDllCount + 1));

	RtlZeroMemory(lpBuffer,allocSize);

	RtlCopyMemory(&pExtData->apiName[2],FunctionName, strlen(FunctionName)); //nameҪ��һ��word�洢hint
	RtlCopyMemory(pExtData->dllName,DllName,strlen(DllName));

	pImportNew = (PIMAGE_IMPORT_DESCRIPTOR)lpBuffer;

	// ��ԭ�����������¿ռ䡣
	RtlCopyMemory(pImportNew , pImportDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR) * nImportDllCount );

	// �����Լ���DLL    IMAGE_IMPORT_DESCRIPTOR�ṹ
	pExtData->thunkEnd = 0;
	pExtData->orgthunkEnd = 0;
	pOriginalThunkData = (PIMAGE_THUNK_DATA)&(pExtData->orgthunk);
	pFirstThunkData = (PIMAGE_THUNK_DATA)&(pExtData->thunk);
	pApiName = (PIMAGE_IMPORT_BY_NAME)pExtData->apiName; 
	pApiName->Hint = 0;

	// ����Ҫһ������API������thunkָ������
	pOriginalThunkData[0].u1.AddressOfData = ULONG_PTR_SUB(pApiName,ulBaseImage);
	pFirstThunkData[0].u1.AddressOfData = ULONG_PTR_SUB(pApiName,ulBaseImage);
	// DLL���ֵ�RVA
	Add_ImportDesc.Name = ULONG_PTR_SUB(pExtData->dllName,ulBaseImage);
	//���������
	Add_ImportDesc.FirstThunk = ULONG_PTR_SUB(pFirstThunkData,ulBaseImage);
	Add_ImportDesc.Characteristics = ULONG_PTR_SUB(pOriginalThunkData,ulBaseImage);
	Add_ImportDesc.TimeDateStamp = 0;
	Add_ImportDesc.ForwarderChain = 0;


	//���Լ��ı�׷����ĩβ
	pImportNew += (nImportDllCount-1);
	RtlCopyMemory(pImportNew, &Add_ImportDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR));

	//��β����
	pImportNew += 1;
	RtlZeroMemory(pImportNew, sizeof(IMAGE_IMPORT_DESCRIPTOR));


	//����IAT RVA�Լ�sizeָ��
	ULONG *pVirtualAddr,*pSize;
	if (bIs64Bit)
	{
		pVirtualAddr = &(pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		pSize = &(pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size);
	}
	else
	{
		pVirtualAddr = &(pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		pSize = &(pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size);
	}

	//DbgPrint("�ɵ����RVA��%p!", *pVirtualAddr);
	//DbgPrint("�ɵ�����С��%x!", *pSize);

	//��DataDirectoryҳ���Ϊ��д����
	PVOID temp_BaseAddr = NULL;
	//��8����������Ϊ��д
	SIZE_T protectSize = 16*sizeof(IMAGE_DATA_DIRECTORY);

	ULONG oldProtect = 0;

	//����nt����ǰ����ǰģʽ����Ϊ�ں�ģʽ 
	temp_BaseAddr = (PVOID)pVirtualAddr;
	//�����ڴ�����Ϊ��д
	ntStatus = g_NtProtectVirtualMemory(hProcessHandle,&temp_BaseAddr,(PSIZE_T)&protectSize,PAGE_EXECUTE_READWRITE,&oldProtect);
	if (!NT_SUCCESS(ntStatus))
	{
		g_NtProtectVirtualMemory(hProcessHandle,&temp_BaseAddr,(PSIZE_T)&protectSize,oldProtect,&oldProtect);
		setKernelMode(preMode);

		if (Attached) {
			KeUnstackDetachProcess (&ApcState);
		}
		ZwClose(hProcessHandle);
		ObDereferenceObject(Process);
		return;
	}

	ULONG sectionVAR;
	ULONG sectionSize;

	//���IAT�����Ѱ󶨵�������㡣
	if( bIs64Bit)
	{
		//ȡ���������������ж���
		pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
		pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;
		//����IAT�������IAT
		if (0 == pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress)
		{
			sectionVAR = getVirtualAddressForIAT(pHeader64,&sectionSize);
			if (sectionVAR)
			{
				pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = sectionVAR;
				pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = sectionSize;
			}

		}
	}
	else
	{
		pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
		pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;
		//����IAT�������IAT
		if (0 == pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress)
		{
			sectionVAR = getVirtualAddressForIAT(pHeader,&sectionSize);
			if (sectionVAR)
			{
				pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = sectionVAR;
				pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = sectionSize;
			}

		}
	}

	// ��DataDirectory�е�����rva��size
	*pSize += sizeof(IMAGE_IMPORT_DESCRIPTOR);
	*pVirtualAddr = ULONG_PTR_SUB(lpBuffer , ulBaseImage);

	//DbgPrint("�µ����RVA��%p!", *pVirtualAddr);
	//DbgPrint("�µ�����С��%x!", *pSize);

	//��DataDirectoryҳ��Ļ�ԭ���Ķ�д����
	temp_BaseAddr = pVirtualAddr;  //pHeader->OptionalHeader.DataDirectory��ַ
	protectSize = 16*sizeof(IMAGE_DATA_DIRECTORY);
	ntStatus = g_NtProtectVirtualMemory(hProcessHandle,(PVOID *)&temp_BaseAddr,(PSIZE_T)&protectSize,oldProtect,&oldProtect);

	setKernelMode(preMode);

	if (Attached) {
		KeUnstackDetachProcess (&ApcState);
	}

	ObDereferenceObject (Process);
	ZwClose(hProcessHandle);

	DbgPrint("IAT HOOK�ɹ�!!!!\n");
}
