#include "public.h"
#include "injectDLL.h"
#include "nativeApi.h"
#include "allocVM.h"
#include "globals.h"

#define ULONG_PTR_SUB(a,b) ((ULONG)(((ULONG_PTR)(a))-((ULONG_PTR)(b))))


//设置当前模式为内核
//Nt函数只有在内核模式才能调用
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
		mov eax,fs:0x124;//获取_KTHREAD 结构体地址

		add eax,0x140;//获取previousmode位置

		mov byte ptr [eax],0;

		pop eax;
	}*/
}

//获取导入表所在区的偏移
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

//由于有些程序的导入表大小并不准确，需要自己遍历导入表，获取大小。
ULONG getImportTableSize(PIMAGE_IMPORT_DESCRIPTOR pDes)
{
	ULONG i=0;
	while (pDes->Name != 0)
	{
		i++;
		pDes++;
	}
	return ++i;//包含最后全0项
}

//修改导入表-主函数
VOID addIAT (
			IN HANDLE    ProcessId, // 当前进行load_image的进程
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

	//用于存储需要添加的新导入表项
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
		DbgPrint("DllName或FunctionName未设置!");
		return;
	}
	if((sizeof(DllName)>128)||(sizeof(FunctionName)>128))
	{
		DbgPrint("DllName或FunctionName过长!");
		return;
	}
	//修改先前模式为内核态
	preMode = setKernelMode(0);
	PVOID lpBuffer = NULL;

	ntStatus = PsLookupProcessByProcessId(ProcessId,&Process);
	if (!NT_SUCCESS(ntStatus)) {
		return ;
	}

// 判断进程是否已经终止
// 	if (KeWaitForSingleObject( Process, Executive, KernelMode, FALSE, &procTimeout ) == STATUS_WAIT_0)
// 	{
// 		KdPrint(( "Process is terminating. Abort\n"));
// 
// 		if (Process)
// 			ObDereferenceObject( Process );
// 
// 		return ;
// 	}


	PVOID ulBaseImage = BaseImage;// 进程基地址

	CurrentProcess = PsGetCurrentProcess();

	if (CurrentProcess != Process) {
		KeStackAttachProcess (Process, &ApcState);
		Attached = TRUE;
	}

	//获取pe头
	pDos = (PIMAGE_DOS_HEADER) ulBaseImage;
	pHeader = (PIMAGE_NT_HEADERS32)((ULONG_PTR)ulBaseImage+pDos->e_lfanew);
	pHeader64 = (PIMAGE_NT_HEADERS64)pHeader;

	//判断是否为64位程序
	bIs64Bit = ( pHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC );

	
	ULONG nImportDllCount = 1;//无导入表，如ntdll
	if(bIs64Bit)
		pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress + (ULONG_PTR)ulBaseImage);
	else
		pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress + (ULONG_PTR)ulBaseImage);

	if ( (ULONG_PTR)pImportDesc != (ULONG_PTR)ulBaseImage)
	{
		nImportDllCount =  getImportTableSize(pImportDesc);
	}


	PExtData pExtData = NULL;

	//获取句柄
	hProcessHandle = OpenProcess(ProcessId);
	if(NULL == hProcessHandle)
	{
		ObDereferenceObject(Process);
		return ;
	}

	//加上一个导入表描述，再加上一个自己的结构,ExtData。
	allocSize =sizeof(ExtData) + sizeof(IMAGE_IMPORT_DESCRIPTOR) * (nImportDllCount + 1);

	//分配导入表
	tempSize = allocSize;

	if (bIs64Bit)
		lpBuffer = (PVOID)((ULONG_PTR)BaseImage+pHeader64->OptionalHeader.SizeOfImage+2*allocSize+100000);//SizeOfImage
	//32位程序在0x70000000出分配内存
	else
		lpBuffer = (PVOID)0x70000000;
	lpBuffer = AllocateInjectMemory(hProcessHandle,lpBuffer,tempSize);

	//DbgPrint("New IAT Buffer Address: %p!", lpBuffer);
/*
	ntStatus = ZwAllocateVirtualMemory(hProcessHandle, &lpBuffer, 0, (PSIZE_T)&tempSize,
		MEM_COMMIT|MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE);*/

	//偏移为负数
	if((ULONG_PTR)ulBaseImage>(ULONG_PTR)lpBuffer) 
	{
		KdPrint(("Can't allocate suite memory! \n"));
		ZwClose(hProcessHandle);
		return ;
	}

	//指向新导入表后面的额外数据（用于存放dll名，函数名等信息）
	pExtData = (PExtData)((char *)lpBuffer + sizeof(IMAGE_IMPORT_DESCRIPTOR) * (nImportDllCount + 1));

	RtlZeroMemory(lpBuffer,allocSize);

	RtlCopyMemory(&pExtData->apiName[2],FunctionName, strlen(FunctionName)); //name要留一个word存储hint
	RtlCopyMemory(pExtData->dllName,DllName,strlen(DllName));

	pImportNew = (PIMAGE_IMPORT_DESCRIPTOR)lpBuffer;

	// 把原来数据移至新空间。
	RtlCopyMemory(pImportNew , pImportDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR) * nImportDllCount );

	// 构造自己的DLL    IMAGE_IMPORT_DESCRIPTOR结构
	pExtData->thunkEnd = 0;
	pExtData->orgthunkEnd = 0;
	pOriginalThunkData = (PIMAGE_THUNK_DATA)&(pExtData->orgthunk);
	pFirstThunkData = (PIMAGE_THUNK_DATA)&(pExtData->thunk);
	pApiName = (PIMAGE_IMPORT_BY_NAME)pExtData->apiName; 
	pApiName->Hint = 0;

	// 至少要一个导出API，并让thunk指向函数名
	pOriginalThunkData[0].u1.AddressOfData = ULONG_PTR_SUB(pApiName,ulBaseImage);
	pFirstThunkData[0].u1.AddressOfData = ULONG_PTR_SUB(pApiName,ulBaseImage);
	// DLL名字的RVA
	Add_ImportDesc.Name = ULONG_PTR_SUB(pExtData->dllName,ulBaseImage);
	//构造加入项
	Add_ImportDesc.FirstThunk = ULONG_PTR_SUB(pFirstThunkData,ulBaseImage);
	Add_ImportDesc.Characteristics = ULONG_PTR_SUB(pOriginalThunkData,ulBaseImage);
	Add_ImportDesc.TimeDateStamp = 0;
	Add_ImportDesc.ForwarderChain = 0;


	//将自己的表追加在末尾
	pImportNew += (nImportDllCount-1);
	RtlCopyMemory(pImportNew, &Add_ImportDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR));

	//表尾清零
	pImportNew += 1;
	RtlZeroMemory(pImportNew, sizeof(IMAGE_IMPORT_DESCRIPTOR));


	//设置IAT RVA以及size指针
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

	//DbgPrint("旧导入表RVA：%p!", *pVirtualAddr);
	//DbgPrint("旧导入表大小：%x!", *pSize);

	//将DataDirectory页面改为可写属性
	PVOID temp_BaseAddr = NULL;
	//将8个表项设置为可写
	SIZE_T protectSize = 16*sizeof(IMAGE_DATA_DIRECTORY);

	ULONG oldProtect = 0;

	//调用nt函数前将先前模式设置为内核模式 
	temp_BaseAddr = (PVOID)pVirtualAddr;
	//更改内存属性为可写
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

	//添加IAT，并把绑定导入表清零。
	if( bIs64Bit)
	{
		//取消绑定输入表里的所有东西
		pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
		pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;
		//若无IAT，则添加IAT
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
		//若无IAT，则添加IAT
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

	// 改DataDirectory中导入表的rva和size
	*pSize += sizeof(IMAGE_IMPORT_DESCRIPTOR);
	*pVirtualAddr = ULONG_PTR_SUB(lpBuffer , ulBaseImage);

	//DbgPrint("新导入表RVA：%p!", *pVirtualAddr);
	//DbgPrint("新导入表大小：%x!", *pSize);

	//将DataDirectory页面改回原来的读写属性
	temp_BaseAddr = pVirtualAddr;  //pHeader->OptionalHeader.DataDirectory地址
	protectSize = 16*sizeof(IMAGE_DATA_DIRECTORY);
	ntStatus = g_NtProtectVirtualMemory(hProcessHandle,(PVOID *)&temp_BaseAddr,(PSIZE_T)&protectSize,oldProtect,&oldProtect);

	setKernelMode(preMode);

	if (Attached) {
		KeUnstackDetachProcess (&ApcState);
	}

	ObDereferenceObject (Process);
	ZwClose(hProcessHandle);

	DbgPrint("IAT HOOK成功!!!!\n");
}
