#include "cleaning.h"

//�������������ͨ��
#define MY_CUSTOM_IOCTL_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)


/*
���ܺ���
*/

// ��ȡ�����Ϣ���ж��Ƿ��ǽ����ļ����  
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

	// ���ݾ����Ӧ��PID�򿪽��̻�ȡ�����Ӧ�Ľ��̾��
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

		// ���Ѵ򿪽����е��ļ�������Ƶ���ǰ���� NtCurrentProcess ��
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

		// ��ѯ�����������Ϣ
	
		// �Ȳ�ѯ����������Ϣ������BasicInfo
		ZwQueryObject(hDupObj, ObjectBasicInformation, &BasicInfo, sizeof(OBJECT_BASIC_INFORMATION), NULL);

		// ��ѯ������Ϣ�еĶ���������������Ϣ���浽pNameInfo��
		// �Ȼ�ȡ��������С
		status = ZwQueryObject(hDupObj, ObjectNameInformation, NULL, 0, &ulObjNameInfoSize);
		if (0 >= ulObjNameInfoSize)
		{
			/*if (status != STATUS_KEY_DELETED)
			{
				DbgPrint("ZwQueryObject_1, status: %x\n", status);
			}*/
			break;

		}
		
		//̫���ֵ����Ϊ�쳣
		if (ulObjNameInfoSize >= 1000)
		{
			//DbgPrint("����ZwQueryObject�Ŀռ���Ҫ��̫�࣡size: %x\n", ulObjNameInfoSize);
			break;
		}

		pObjNameInfo = ExAllocatePool(NonPagedPool, ulObjNameInfoSize);
		if (NULL == pObjNameInfo)
		{
			//DbgPrint("ExAllocatePool_2, size: %x\n", ulObjNameInfoSize);

			break;
		}
		RtlZeroMemory(pObjNameInfo, ulObjNameInfoSize);

		// ��ȡ�������������Ϣ
		status = ZwQueryObject(hDupObj, ObjectNameInformation, pObjNameInfo, ulObjNameInfoSize, &ulObjNameInfoSize);
		if (!NT_SUCCESS(status))
		{
			/*if (status != STATUS_OBJECT_PATH_INVALID)
			{
				DbgPrint("ZwQueryObject_2��status: %x\n", status);
			}*/
			break;
		}

		// �ж��Ƿ�·�����Ⱥ���
		if (pObjNameInfo->Name.Length >= MAX_PATH)
		{
			//DbgPrint("�ļ�������,����\n");
			break;
		}

		//DbgPrint("%wZ\n", &pObjNameInfo->Name);
		RtlZeroMemory(wszSrcFile, MAX_PATH * sizeof(WCHAR));
		RtlZeroMemory(wszDestFile, MAX_PATH * sizeof(WCHAR));
		RtlCopyMemory(wszSrcFile, pObjNameInfo->Name.Buffer, pObjNameInfo->Name.Length);
		RtlCopyMemory(wszDestFile, ustrUnlockFileName.Buffer, ustrUnlockFileName.Length);
		if (!wcscmp(wszSrcFile, wszDestFile))
		{
			DbgPrint("[Found]�ҵ�Ŀ����!!!\n");
			DbgPrint("[File]%wZ\n", &pObjNameInfo->Name);
			bRet = TRUE;
			break;
		}
	} while (FALSE);

	// �ͷ�
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

// �ر��ļ����, �����ļ�  
BOOLEAN CloseFileHandle(SYSTEM_HANDLE_TABLE_ENTRY_INFO sysHandleTableEntryInfo)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pEProcess = NULL;
	HANDLE hFileHandle = NULL;
	KAPC_STATE apcState = { 0 };
	OBJECT_HANDLE_FLAG_INFORMATION objectHandleFlagInfo = { 0 };
	// ��ȡ�ļ����
	hFileHandle = sysHandleTableEntryInfo.HandleValue;
	// ��ȡ�ļ������Ӧ�Ľ��̽ṹ����EPROCESS
	status = PsLookupProcessByProcessId(sysHandleTableEntryInfo.UniqueProcessId, &pEProcess);
	if (!NT_SUCCESS(status))
	{
		//DbgPrint("PsLookupProcessByProcessId����\n");
		return FALSE;
	}
	// ���ӵ��ļ������Ӧ�Ľ��̿ռ�
	KeStackAttachProcess(pEProcess, &apcState);
	// ���ļ��������������Ϊ�����Թرա�
	objectHandleFlagInfo.Inherit = 0;
	objectHandleFlagInfo.ProtectFromClose = 0;
	ObSetHandleAttributes((HANDLE)hFileHandle, &objectHandleFlagInfo, KernelMode);
	// �ر��ļ����
	ZwClose((HANDLE)hFileHandle);
	// �������̸���
	KeUnstackDetachProcess(&apcState);
	return TRUE;
}

// �������, �����ļ� 
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

	// ����ZwQuerySystemInformation��16�Ź�����ö��ϵͳ��ľ��
	// �Ȼ�ȡ��������С
	status = ZwQuerySystemInformation(16, &tempSysHandleInfo, sizeof(tempSysHandleInfo), &ulSysHandleInfoSize);
	if (0 >= ulSysHandleInfoSize)
	{
		//DbgPrint("ZwQuerySystemInformation_1, status: %x\n", status);
		return FALSE;
	}
	//DbgPrint("��ѯSystemInformation���ݴ�СΪ %d\n", ulSysHandleInfoSize);

	// ���뻺�����ڴ�
	do
	{
		status = pSysHandleInfo = (PSYSTEM_HANDLE_INFORMATION)ExAllocatePool(NonPagedPool, ulSysHandleInfoSize);
		if (NULL == pSysHandleInfo)
		{
			//DbgPrint("ExAllocate SystemInformationʱ��������! status: %x\n", status);
			return FALSE;
		}
		RtlZeroMemory(pSysHandleInfo, ulSysHandleInfoSize);

		// ��ȡϵͳ�����о������Ϣ
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

	// ��ȡϵͳ���о�������Լ������Ϣ����
	ullSysHandleCount = pSysHandleInfo->NumberOfHandles;
	pSysHandleTableEntryInfo = pSysHandleInfo->Handles;

	DbgPrint("��Ҫ���ҵ��ļ���: %wZ\n", ustrUnlockFileName);

	// ��ʼ����ϵͳ�����о��
	for (i = 0; i < ullSysHandleCount; i++)
	{
		// ��ȡ�����Ϣ���ж��Ƿ��ǽ����ļ����
		bRet = IsUnlockFileHandle(pSysHandleTableEntryInfo[i], ustrUnlockFileName);
		if (bRet)
		{
			// �ر��ļ����, �����ļ�
			CloseFileHandle(pSysHandleTableEntryInfo[i]);
			// ��ʾ
			DbgPrint("[�������][%d][%d]\n",
				pSysHandleTableEntryInfo[i].UniqueProcessId, pSysHandleTableEntryInfo[i].HandleValue);
			return TRUE;
		}
	}
	// �ͷ�
	ExFreePool(pSysHandleInfo);

	return FALSE;
}

// ǿ��ɾ���ļ�
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


	// �ж��жϵȼ�������0
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
		// ��ȡ��ǰ���̵�EProcess
		pCurEprocess = IoGetCurrentProcess();

		// ���ӽ���
		KeStackAttachProcess(pCurEprocess, &kapc);

		// ��ʼ���ṹ
		InitializeObjectAttributes(&fileOb, &pwzFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		// �ļ�ϵͳɸѡ���������� ����ָ���豸���������ɸѡ�����ļ�ϵͳ���ʹ�������
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
				DbgPrint("�Ҳ�����Ҫɾ�����ļ�!\n");
			else
				DbgPrint("IoCreateFileSpecifyDeviceObjectHint��status: %x\n", status);

			return FALSE;
		}

		// �ڶ��������ṩ������֤����������������Ȩ�ޣ��򷵻�ָ���������ĵ���Ӧָ�롣
		status = ObReferenceObjectByHandle(hFile, 0, 0, 0, &pHandleFileObject, 0);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("ObReferenceObjectByHandle��status: %x\n", status);
			return FALSE;
		}

		// ����ڶ�������Ϊ0
		((PFILE_OBJECT)(pHandleFileObject))->SectionObjectPointer->ImageSectionObject = 0;

		// ɾ��Ȩ�޴�
		((PFILE_OBJECT)(pHandleFileObject))->DeleteAccess = 1;

		// ����ɾ���ļ�API
		status = ZwDeleteFile(&fileOb);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("ZwDeleteFileʧ�ܣ�status: %x\n", status);
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
�������� 
*/

// IoControl���� 
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

	//DbgPrint("�ɹ�����MyDeviceControlRoutine!\n");
	switch (ioControlCode)
	{

		case MY_CUSTOM_IOCTL_CODE:
		{
			// �����Զ�����豸��������
			// ʹ�� inputBuffer �� inputBufferLength ���в���
			// �����д�� outputBuffer�������� outputBufferLength

			__try
			{
				// ��ȡ���뻺�����е�����
				if (inputBuffer != NULL && inputBufferLength > 0)
				{
					//DbgPrint("�ɹ�����MY_CUSTOM_IOCTL_CODE!\n");
					
					//�Խṹ��ķ�ʽ�������뻺����
					path_packet* pk = NULL;
					pk = (path_packet*)inputBuffer;
					//DbgPrint("Device·��: %ls Win32·��: %ls\n", pk->device_path, pk->nt_path);

					//�ַ�����ʽת��
					wchar_t* device_pathw = pk->device_path;
					wchar_t win32_pathw[MAX_PATH] = L"\\??\\";
					UNICODE_STRING device_pathu;
					UNICODE_STRING win32_pathu;
					wcscat(win32_pathw, pk->nt_path);

					RtlInitUnicodeString(&device_pathu, device_pathw);
					RtlInitUnicodeString(&win32_pathu, win32_pathw);
					DbgPrint("�����Device·��: %wZ\n Win32·��: %wZ\n", &device_pathu, &win32_pathu);

					//�ֱ���Ҫ�豸·����win32·��
					BOOLEAN result = FALSE;
					result = Unlockfile(device_pathu);
					if (result)
						DbgPrint("[Unlockfile]�ɹ������ļ�!\n");
				
					result = ForceDeleteFile(win32_pathu);
					if (result)
						DbgPrint("[ForceDeleteFile]�ɹ�ɾ���ļ�!\n");
				}
			}
			__except (1)
			{
				status = GetExceptionCode();
				DbgPrint("IOCTL_GET_EPROCESS %x \n", status);
			}

			// ����ͨ��״̬
			status = STATUS_SUCCESS;
			break;
		}

		default:
		{
			// δ֪���豸��������

			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}
	}

	// �趨DeviceIoControl��*lpBytesReturned��ֵ�����ͨ��ʧ���򷵻�0���ȣ�
	if (status == STATUS_SUCCESS)
	{
		Irp->IoStatus.Information = outputBufferLength;
	}
	else
	{
		Irp->IoStatus.Information = 0;
	}

	// �趨DeviceIoControl�ķ���ֵ�ǳɹ�����ʧ��
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

// ������Ĭ����ǲ����
NTSTATUS DefaultDispatch(PDEVICE_OBJECT _pDeviceObject, PIRP _pIrp)
{
	_pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	_pIrp->IoStatus.Information = 0;
	IoCompleteRequest(_pIrp, IO_NO_INCREMENT);
	return _pIrp->IoStatus.Status;
}

NTSTATUS  Create(DEVICE_OBJECT* device, IRP* irp) 
{
	//DbgPrint("[ForceDelete]�����豸��R3��������\n");
	// ����IRP���״̬
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS  Close(DEVICE_OBJECT* device, IRP* irp) 
{
	//DbgPrint("[ForceDelete]�����豸��R3�ر�����\n");
	// ����IRP���״̬
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

void Unload(PDRIVER_OBJECT driver)
{
	UNREFERENCED_PARAMETER(driver);
	DbgPrint("[ForceDelete]��������������˳�!");

	// ɾ����������
	UNICODE_STRING DosSymName;
	RtlInitUnicodeString(&DosSymName, L"\\??\\MyDriver");
	IoDeleteSymbolicLink(&DosSymName);

	// ж���豸����
	IoDeleteDevice(driver->DeviceObject);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT	driver, PUNICODE_STRING	RegPath)
{
	UNREFERENCED_PARAMETER(driver);
	UNREFERENCED_PARAMETER(RegPath);

	NTSTATUS status = STATUS_SUCCESS;;
	UNICODE_STRING MyDriver; //�����ַ���
	PDEVICE_OBJECT device = NULL;//���ڴ���豸����
	PDEVICE_OBJECT deviceObject;

	
	// ��ʼ��������ǲ
	for (ULONG i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		// DbgPrint("��ʼ����ǲ: %d \n", i);
		driver->MajorFunction[i] = DefaultDispatch;
	}

	// ����ǲ����, �����������������, �û����ڴ�����ʱ��ʧ��.
	driver->MajorFunction[IRP_MJ_CREATE] = Create;
	driver->MajorFunction[IRP_MJ_CLOSE] = Close;
	// ����IRP_MJ_DEVICE_CONTROL�������
	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MyDeviceControlRoutine;
	driver->DriverUnload = Unload;

	// �����豸����
	RtlInitUnicodeString(&MyDriver, L"\\DEVICE\\MyDevice");//�����豸����
	status = IoCreateDevice(driver, sizeof(driver->DriverExtension), &MyDriver, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device);
	if (NT_SUCCESS(status))
	{
		//������������
		UNICODE_STRING uzSymbolName; //������������		 
		RtlInitUnicodeString(&uzSymbolName, L"\\??\\MyDriver"); //CreateFile
		status = IoCreateSymbolicLink(&uzSymbolName, &MyDriver);
		if (NT_SUCCESS(status))
		{
			DbgPrint("[ForceDelete]�����������ӳɹ�!");
		}
		else
		{
			DbgPrint("������������ %wZ ʧ�� status=%X", &uzSymbolName, status);
			return status;
		}
	}
	else
	{
		DbgPrint("[ForceDelete]�����豸���󴴽�ʧ�ܣ�ɾ���豸!\n");
		IoDeleteDevice(device);
		return status;
	}

	DbgPrint("[ForceDelete]�����������������!");
	return STATUS_SUCCESS;
}