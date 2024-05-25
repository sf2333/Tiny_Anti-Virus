#include "cleaning.h"

//定义控制码用于通信
#define MY_CUSTOM_IOCTL_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)


/*
功能函数
*/

// 获取句柄信息并判断是否是解锁文件句柄  
BOOLEAN IsUnlockFileHandle(SYSTEM_HANDLE_TABLE_ENTRY_INFO sysHandleTableEntryInfo, UNICODE_STRING ustrUnlockFileName)
{
	NTSTATUS status = STATUS_SUCCESS;
	CLIENT_ID clientId = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	HANDLE hSourceProcess = NULL;
	HANDLE hDupObj = NULL;

	OBJECT_BASIC_INFORMATION BasicInfo;
	POBJECT_NAME_INFORMATION pObjNameInfo = NULL;
	ULONG ulObjNameInfoSize = 0;

	BOOLEAN bRet = FALSE;
	WCHAR wszSrcFile[MAX_PATH] = { 0 };
	WCHAR wszDestFile[MAX_PATH] = { 0 };

	// 根据句柄对应的PID打开进程获取句柄对应的进程句柄
	do
	{
		//InitializeObjectAttributes(&objectAttributes, NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
		InitializeObjectAttributes(&objectAttributes, NULL, 0, NULL, NULL);
		RtlZeroMemory(&clientId, sizeof(clientId));
		clientId.UniqueProcess = sysHandleTableEntryInfo.UniqueProcessId;
		//status = ZwOpenProcess(&hSourceProcess, PROCESS_ALL_ACCESS, &objectAttributes, &clientId);
		status = ZwOpenProcess(&hSourceProcess, PROCESS_DUP_HANDLE, &objectAttributes, &clientId);
		
		if (!NT_SUCCESS(status))
		{
			//DbgPrint("ZwOpenProcess, status: %x\n", status);
			break;
		}

		// 将已打开进程中的文件句柄复制到当前进程 NtCurrentProcess 中
		status = ZwDuplicateObject(hSourceProcess, sysHandleTableEntryInfo.HandleValue,
			NtCurrentProcess(), &hDupObj, PROCESS_ALL_ACCESS, 0, DUPLICATE_SAME_ACCESS);
		if (!NT_SUCCESS(status))
		{
			/*if (status != STATUS_NOT_SUPPORTED && status != STATUS_ACCESS_DENIED)
			{
				DbgPrint("ZwDuplicateObject, status: %x\n", status);
			}*/
			break;
		}

		// 查询句柄的名称信息
	
		// 先查询对象句柄的信息并放入BasicInfo
		ZwQueryObject(hDupObj, ObjectBasicInformation, &BasicInfo, sizeof(OBJECT_BASIC_INFORMATION), NULL);

		// 查询对象信息中的对象名，并将该信息保存到pNameInfo中
		// 先获取缓冲区大小
		status = ZwQueryObject(hDupObj, ObjectNameInformation, NULL, 0, &ulObjNameInfoSize);
		if (0 >= ulObjNameInfoSize)
		{
			/*if (status != STATUS_KEY_DELETED)
			{
				DbgPrint("ZwQueryObject_1, status: %x\n", status);
			}*/
			break;

		}
		
		//太大的值被视为异常
		if (ulObjNameInfoSize >= 1000)
		{
			//DbgPrint("用于ZwQueryObject的空间需要的太多！size: %x\n", ulObjNameInfoSize);
			break;
		}

		pObjNameInfo = ExAllocatePool(NonPagedPool, ulObjNameInfoSize);
		if (NULL == pObjNameInfo)
		{
			//DbgPrint("ExAllocatePool_2, size: %x\n", ulObjNameInfoSize);

			break;
		}
		RtlZeroMemory(pObjNameInfo, ulObjNameInfoSize);

		// 获取句柄名称类型信息
		status = ZwQueryObject(hDupObj, ObjectNameInformation, pObjNameInfo, ulObjNameInfoSize, &ulObjNameInfoSize);
		if (!NT_SUCCESS(status))
		{
			/*if (status != STATUS_OBJECT_PATH_INVALID)
			{
				DbgPrint("ZwQueryObject_2，status: %x\n", status);
			}*/
			break;
		}

		// 判断是否路径长度合适
		if (pObjNameInfo->Name.Length >= MAX_PATH)
		{
			//DbgPrint("文件名过长,跳过\n");
			break;
		}

		//DbgPrint("%wZ\n", &pObjNameInfo->Name);
		RtlZeroMemory(wszSrcFile, MAX_PATH * sizeof(WCHAR));
		RtlZeroMemory(wszDestFile, MAX_PATH * sizeof(WCHAR));
		RtlCopyMemory(wszSrcFile, pObjNameInfo->Name.Buffer, pObjNameInfo->Name.Length);
		RtlCopyMemory(wszDestFile, ustrUnlockFileName.Buffer, ustrUnlockFileName.Length);
		if (!wcscmp(wszSrcFile, wszDestFile))
		{
			DbgPrint("[Found]找到目标句柄!!!\n");
			DbgPrint("[File]%wZ\n", &pObjNameInfo->Name);
			bRet = TRUE;
			break;
		}
	} while (FALSE);

	// 释放
	if (NULL != pObjNameInfo)
	{
		ExFreePool(pObjNameInfo);
	}
	if (NULL != hDupObj)
	{
		ZwClose(hDupObj);
	}
	if (NULL != hSourceProcess)
	{
		ZwClose(hSourceProcess);
	}
	return bRet;
}

// 关闭文件句柄, 解锁文件  
BOOLEAN CloseFileHandle(SYSTEM_HANDLE_TABLE_ENTRY_INFO sysHandleTableEntryInfo)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pEProcess = NULL;
	HANDLE hFileHandle = NULL;
	KAPC_STATE apcState = { 0 };
	OBJECT_HANDLE_FLAG_INFORMATION objectHandleFlagInfo = { 0 };
	// 获取文件句柄
	hFileHandle = sysHandleTableEntryInfo.HandleValue;
	// 获取文件句柄对应的进程结构对象EPROCESS
	status = PsLookupProcessByProcessId(sysHandleTableEntryInfo.UniqueProcessId, &pEProcess);
	if (!NT_SUCCESS(status))
	{
		//DbgPrint("PsLookupProcessByProcessId出错\n");
		return FALSE;
	}
	// 附加到文件句柄对应的进程空间
	KeStackAttachProcess(pEProcess, &apcState);
	// 将文件句柄的属性设置为“可以关闭”
	objectHandleFlagInfo.Inherit = 0;
	objectHandleFlagInfo.ProtectFromClose = 0;
	ObSetHandleAttributes((HANDLE)hFileHandle, &objectHandleFlagInfo, KernelMode);
	// 关闭文件句柄
	ZwClose((HANDLE)hFileHandle);
	// 结束进程附加
	KeUnstackDetachProcess(&apcState);
	return TRUE;
}

// 遍历句柄, 解锁文件 
BOOLEAN Unlockfile(UNICODE_STRING ustrUnlockFileName)
{
	NTSTATUS status = STATUS_SUCCESS;
	SYSTEM_HANDLE_INFORMATION  tempSysHandleInfo = { 0 };
	PSYSTEM_HANDLE_INFORMATION  pSysHandleInfo = NULL;
	ULONG ulSysHandleInfoSize = 0;
	ULONG ulReturnLength = 0;
	ULONGLONG ullSysHandleCount = 0;
	PSYSTEM_HANDLE_TABLE_ENTRY_INFO pSysHandleTableEntryInfo = NULL;
	ULONGLONG i = 0;
	BOOLEAN bRet = FALSE;

	// 调用ZwQuerySystemInformation的16号功能来枚举系统里的句柄
	// 先获取缓冲区大小
	status = ZwQuerySystemInformation(16, &tempSysHandleInfo, sizeof(tempSysHandleInfo), &ulSysHandleInfoSize);
	if (0 >= ulSysHandleInfoSize)
	{
		//DbgPrint("ZwQuerySystemInformation_1, status: %x\n", status);
		return FALSE;
	}
	//DbgPrint("查询SystemInformation内容大小为 %d\n", ulSysHandleInfoSize);

	// 申请缓冲区内存
	do
	{
		status = pSysHandleInfo = (PSYSTEM_HANDLE_INFORMATION)ExAllocatePool(NonPagedPool, ulSysHandleInfoSize);
		if (NULL == pSysHandleInfo)
		{
			//DbgPrint("ExAllocate SystemInformation时发生错误! status: %x\n", status);
			return FALSE;
		}
		RtlZeroMemory(pSysHandleInfo, ulSysHandleInfoSize);

		// 获取系统中所有句柄的信息
		status = ZwQuerySystemInformation(16, pSysHandleInfo, ulSysHandleInfoSize, &ulReturnLength);
		if (!NT_SUCCESS(status))
		{
			ExFreePool(pSysHandleInfo);
			if (status == STATUS_INFO_LENGTH_MISMATCH)
			{
				//DbgPrint("ZwQuerySystemInformation size not match!, need size:%d\n", ulReturnLength);
				ulSysHandleInfoSize = ulReturnLength + 1000;
				ulReturnLength = 0;
			}
			else
			{
				//DbgPrint("ZwQuerySystemInformation_2, status: %x\n", status);
				return FALSE;
			}
			
		}
	} while (!NT_SUCCESS(status));

	// 获取系统所有句柄数量以及句柄信息数组
	ullSysHandleCount = pSysHandleInfo->NumberOfHandles;
	pSysHandleTableEntryInfo = pSysHandleInfo->Handles;

	DbgPrint("需要查找的文件名: %wZ\n", ustrUnlockFileName);

	// 开始遍历系统上所有句柄
	for (i = 0; i < ullSysHandleCount; i++)
	{
		// 获取句柄信息并判断是否是解锁文件句柄
		bRet = IsUnlockFileHandle(pSysHandleTableEntryInfo[i], ustrUnlockFileName);
		if (bRet)
		{
			// 关闭文件句柄, 解锁文件
			CloseFileHandle(pSysHandleTableEntryInfo[i]);
			// 显示
			DbgPrint("[解锁句柄][%d][%d]\n",
				pSysHandleTableEntryInfo[i].UniqueProcessId, pSysHandleTableEntryInfo[i].HandleValue);
			return TRUE;
		}
	}
	// 释放
	ExFreePool(pSysHandleInfo);

	return FALSE;
}

// 强制删除文件
BOOLEAN ForceDeleteFile(UNICODE_STRING pwzFileName)
{
	PEPROCESS pCurEprocess = NULL;
	KAPC_STATE kapc = { 0 };
	OBJECT_ATTRIBUTES fileOb;
	HANDLE hFile = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	IO_STATUS_BLOCK iosta;
	PDEVICE_OBJECT DeviceObject = NULL;
	PVOID pHandleFileObject = NULL;


	// 判断中断等级不大于0
	if (KeGetCurrentIrql() > PASSIVE_LEVEL)
	{
		return FALSE;
	}
	if (pwzFileName.Buffer == NULL || pwzFileName.Length <= 0)
	{
		return FALSE;
	}

	__try
	{
		// 读取当前进程的EProcess
		pCurEprocess = IoGetCurrentProcess();

		// 附加进程
		KeStackAttachProcess(pCurEprocess, &kapc);

		// 初始化结构
		InitializeObjectAttributes(&fileOb, &pwzFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		// 文件系统筛选器驱动程序 仅向指定设备对象下面的筛选器和文件系统发送创建请求。
		status = IoCreateFileSpecifyDeviceObjectHint(&hFile,
			SYNCHRONIZE | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES | FILE_READ_DATA,
			&fileOb,
			&iosta,
			NULL,
			0,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			0,
			0,
			CreateFileTypeNone,
			0,
			IO_IGNORE_SHARE_ACCESS_CHECK,
			DeviceObject);

		if (!NT_SUCCESS(status))
		{
			if (status == STATUS_OBJECT_NAME_NOT_FOUND)
				DbgPrint("找不到需要删除的文件!\n");
			else
				DbgPrint("IoCreateFileSpecifyDeviceObjectHint，status: %x\n", status);

			return FALSE;
		}

		// 在对象句柄上提供访问验证，如果可以授予访问权限，则返回指向对象的正文的相应指针。
		status = ObReferenceObjectByHandle(hFile, 0, 0, 0, &pHandleFileObject, 0);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("ObReferenceObjectByHandle，status: %x\n", status);
			return FALSE;
		}

		// 镜像节对象设置为0
		((PFILE_OBJECT)(pHandleFileObject))->SectionObjectPointer->ImageSectionObject = 0;

		// 删除权限打开
		((PFILE_OBJECT)(pHandleFileObject))->DeleteAccess = 1;

		// 调用删除文件API
		status = ZwDeleteFile(&fileOb);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("ZwDeleteFile失败，status: %x\n", status);
			return FALSE;
		}
	}

	_finally
	{
		if (pHandleFileObject != NULL)
		{
			ObDereferenceObject(pHandleFileObject);
			pHandleFileObject = NULL;
		}
		KeUnstackDetachProcess(&kapc);

		if (hFile != NULL || hFile != (PVOID)-1)
		{
			ZwClose(hFile);
			hFile = (PVOID)-1;
		}
	}
	return TRUE;
}


/* 
驱动例程 
*/

// IoControl函数 
NTSTATUS MyDeviceControlRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION irpStack;
	PVOID inputBuffer;
	PVOID outputBuffer;
	ULONG inputBufferLength;
	ULONG outputBufferLength;
	ULONG ioControlCode;

	irpStack = IoGetCurrentIrpStackLocation(Irp);
	inputBuffer = Irp->AssociatedIrp.SystemBuffer;
	outputBuffer = Irp->AssociatedIrp.SystemBuffer;
	inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

	//DbgPrint("成功进入MyDeviceControlRoutine!\n");
	switch (ioControlCode)
	{

		case MY_CUSTOM_IOCTL_CODE:
		{
			// 处理自定义的设备控制请求
			// 使用 inputBuffer 和 inputBufferLength 进行操作
			// 将结果写入 outputBuffer，并设置 outputBufferLength

			__try
			{
				// 读取输入缓冲区中的内容
				if (inputBuffer != NULL && inputBufferLength > 0)
				{
					//DbgPrint("成功进入MY_CUSTOM_IOCTL_CODE!\n");
					
					//以结构体的方式接收输入缓冲区
					path_packet* pk = NULL;
					pk = (path_packet*)inputBuffer;
					//DbgPrint("Device路径: %ls Win32路径: %ls\n", pk->device_path, pk->nt_path);

					//字符串格式转换
					wchar_t* device_pathw = pk->device_path;
					wchar_t win32_pathw[MAX_PATH] = L"\\??\\";
					UNICODE_STRING device_pathu;
					UNICODE_STRING win32_pathu;
					wcscat(win32_pathw, pk->nt_path);

					RtlInitUnicodeString(&device_pathu, device_pathw);
					RtlInitUnicodeString(&win32_pathu, win32_pathw);
					DbgPrint("输入的Device路径: %wZ\n Win32路径: %wZ\n", &device_pathu, &win32_pathu);

					//分别需要设备路径和win32路径
					BOOLEAN result = FALSE;
					result = Unlockfile(device_pathu);
					if (result)
						DbgPrint("[Unlockfile]成功解锁文件!\n");
				
					result = ForceDeleteFile(win32_pathu);
					if (result)
						DbgPrint("[ForceDeleteFile]成功删除文件!\n");
				}
			}
			__except (1)
			{
				status = GetExceptionCode();
				DbgPrint("IOCTL_GET_EPROCESS %x \n", status);
			}

			// 返回通信状态
			status = STATUS_SUCCESS;
			break;
		}

		default:
		{
			// 未知的设备控制请求

			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}
	}

	// 设定DeviceIoControl的*lpBytesReturned的值（如果通信失败则返回0长度）
	if (status == STATUS_SUCCESS)
	{
		Irp->IoStatus.Information = outputBufferLength;
	}
	else
	{
		Irp->IoStatus.Information = 0;
	}

	// 设定DeviceIoControl的返回值是成功还是失败
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

// 驱动绑定默认派遣函数
NTSTATUS DefaultDispatch(PDEVICE_OBJECT _pDeviceObject, PIRP _pIrp)
{
	_pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	_pIrp->IoStatus.Information = 0;
	IoCompleteRequest(_pIrp, IO_NO_INCREMENT);
	return _pIrp->IoStatus.Status;
}

NTSTATUS  Create(DEVICE_OBJECT* device, IRP* irp) 
{
	//DbgPrint("[ForceDelete]驱动设备被R3创建啦！\n");
	// 设置IRP完成状态
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS  Close(DEVICE_OBJECT* device, IRP* irp) 
{
	//DbgPrint("[ForceDelete]驱动设备被R3关闭啦！\n");
	// 设置IRP完成状态
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

void Unload(PDRIVER_OBJECT driver)
{
	UNREFERENCED_PARAMETER(driver);
	DbgPrint("[ForceDelete]病毒清除驱动已退出!");

	// 删除符号链接
	UNICODE_STRING DosSymName;
	RtlInitUnicodeString(&DosSymName, L"\\??\\MyDriver");
	IoDeleteSymbolicLink(&DosSymName);

	// 卸载设备对象
	IoDeleteDevice(driver->DeviceObject);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT	driver, PUNICODE_STRING	RegPath)
{
	UNREFERENCED_PARAMETER(driver);
	UNREFERENCED_PARAMETER(RegPath);

	NTSTATUS status = STATUS_SUCCESS;;
	UNICODE_STRING MyDriver; //驱动字符串
	PDEVICE_OBJECT device = NULL;//用于存放设备对象
	PDEVICE_OBJECT deviceObject;

	
	// 初始化其他派遣
	for (ULONG i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		// DbgPrint("初始化派遣: %d \n", i);
		driver->MajorFunction[i] = DefaultDispatch;
	}

	// 绑定派遣函数, 如果不绑定这两个函数, 用户层在打开驱动时会失败.
	driver->MajorFunction[IRP_MJ_CREATE] = Create;
	driver->MajorFunction[IRP_MJ_CLOSE] = Close;
	// 设置IRP_MJ_DEVICE_CONTROL处理程序
	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MyDeviceControlRoutine;
	driver->DriverUnload = Unload;

	// 创建设备对象
	RtlInitUnicodeString(&MyDriver, L"\\DEVICE\\MyDevice");//驱动设备名字
	status = IoCreateDevice(driver, sizeof(driver->DriverExtension), &MyDriver, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device);
	if (NT_SUCCESS(status))
	{
		//创建符号链接
		UNICODE_STRING uzSymbolName; //符号链接名字		 
		RtlInitUnicodeString(&uzSymbolName, L"\\??\\MyDriver"); //CreateFile
		status = IoCreateSymbolicLink(&uzSymbolName, &MyDriver);
		if (NT_SUCCESS(status))
		{
			DbgPrint("[ForceDelete]创建符号链接成功!");
		}
		else
		{
			DbgPrint("创建符号链接 %wZ 失败 status=%X", &uzSymbolName, status);
			return status;
		}
	}
	else
	{
		DbgPrint("[ForceDelete]驱动设备对象创建失败，删除设备!\n");
		IoDeleteDevice(device);
		return status;
	}

	DbgPrint("[ForceDelete]病毒清除驱动已启动!");
	return STATUS_SUCCESS;
}