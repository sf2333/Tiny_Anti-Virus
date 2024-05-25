#include "hide.h"
#include "protect.h"
#include "globals.h"


// ��ѯKey���е�Valueֵ
BOOLEAN RegQueryValueKey(PUNICODE_STRING RegisterPath, LPWSTR ValueName, PKEY_VALUE_PARTIAL_INFORMATION* pkvpi)
{
    ULONG ulSize;
    NTSTATUS ntStatus;
    PKEY_VALUE_PARTIAL_INFORMATION pvpi;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE hRegister;
    UNICODE_STRING usValueName;

    RtlInitUnicodeString(&usValueName, ValueName);

    // ��ʼ��
    InitializeObjectAttributes(&objectAttributes, RegisterPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    // ��ע���Key
    ntStatus = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
    if (!NT_SUCCESS(ntStatus))
    {
        return FALSE;
    }

    // ��ѯ����
    ntStatus = ZwQueryValueKey(hRegister, &usValueName, KeyValuePartialInformation, NULL, 0, &ulSize);
    if (ntStatus == STATUS_OBJECT_NAME_NOT_FOUND || ulSize == 0)
    {
        return FALSE;
    }

    // ����ռ䱣���ѯ���
    pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
    ntStatus = ZwQueryValueKey(hRegister, &usValueName, KeyValuePartialInformation, pvpi, ulSize, &ulSize);
    if (!NT_SUCCESS(ntStatus))
    {
        return FALSE;
    }

    // �����pvpiδ���ͷţ������ⲿ�ͷ�
    // ִ�� ExFreePool(pvpi); �ͷ�
    *pkvpi = pvpi;
    return TRUE;
}

int QueryStartMode(_In_ PUNICODE_STRING RegistryPath)
{
    BOOLEAN flag = FALSE;
    DWORD64 get_dw = 0;
    PKEY_VALUE_PARTIAL_INFORMATION pkvi;
    int StartMode = 0;
    // ��ѯ����
    flag = RegQueryValueKey(RegistryPath, L"Mode", &pkvi);
    if (flag == TRUE)
    {
        // ������ѯ���
        RtlCopyMemory(&get_dw, pkvi->Data, pkvi->DataLength);

        // ������
        DbgPrint("[*] ��ѯע���Mode���: %d \n", get_dw);
        StartMode = get_dw;
        ExFreePool(pkvi);
    }

    return StartMode;
}

int QueryCurPid(_In_ PUNICODE_STRING RegistryPath)
{
    BOOLEAN flag = FALSE;
    DWORD64 get_dw = 0;
    PKEY_VALUE_PARTIAL_INFORMATION pkvi;
    int CurPid = 0;
    // ��ѯ����
    flag = RegQueryValueKey(RegistryPath, L"Pid", &pkvi);
    if (flag == TRUE)
    {
        // ������ѯ���
        RtlCopyMemory(&get_dw, pkvi->Data, pkvi->DataLength);

        // ������
        DbgPrint("[*] ��ѯע���PID���: %d \n", get_dw);
        CurPid = get_dw;
        ExFreePool(pkvi);
    }

    return CurPid;
}

VOID UnDriver(PDRIVER_OBJECT driver)
{
    UNREFERENCED_PARAMETER(driver);

    switch (mode)
    {
        case 0:
        {
            if(!ControlProcessHide(PRoc, gsave_pid))
                DbgPrint("[���ұ���]����ȡ����\n");
            else
                DbgPrint("[���ұ���]����ȡ��ʧ�ܣ�\n");
            break;
        }
        case 1:
        {
            if (NULL != pRegistrationHandle)
            {
                DbgPrint("[���ұ���]��ɱȡ����\n");
                ObUnRegisterCallbacks(pRegistrationHandle);
                pRegistrationHandle = NULL;
            }
            break;
        }
        default:
            break;
    }
    
    DbgPrint("[���ұ���]��������ж�سɹ�! \n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS ret = STATUS_SUCCESS;
    int cur_pid;

    DriverObject->DriverUnload = UnDriver;

    //��PG
    bypass_signcheck(DriverObject);

    //�жϿ����������ػ��Ƿ�ɱ
    mode = QueryStartMode(RegistryPath);
    cur_pid = QueryCurPid(RegistryPath);

    //��ȡĿ�����eprocess��ַ
    PRoc = GetProcessObjectByID(cur_pid);
    if (PRoc != NULL)
    {
        DbgPrint("[���ұ���]PRoc %p\n", PRoc);
    }
    else
    {
        DbgPrint("[���ұ���]PRoc��ȡʧ��\n");
        return STATUS_UNSUCCESSFUL;
    }

    switch (mode)
    {
        case 0:
        {
            //��������
            gsave_pid = NULL;
            gsave_pid = ControlProcessHide(PRoc, gsave_pid);
            ret = STATUS_SUCCESS;
            DbgPrint("[���ұ���]���ؿ�����\n");
           
            break;
        }
        case 1:
        {

            //���̷�ɱ
            pRegistrationHandle = 0;

            OB_CALLBACK_REGISTRATION ocr;
            OB_OPERATION_REGISTRATION oor;

            //��ʼ�� OB_OPERATION_REGISTRATION 
            RtlZeroMemory(&oor, sizeof(OB_OPERATION_REGISTRATION));
            RtlZeroMemory(&ocr, sizeof(OB_CALLBACK_REGISTRATION));

            //���ü����Ķ�������
            oor.ObjectType = PsProcessType;
            //���ü����Ĳ�������
            oor.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
            //���ò�������ǰִ�еĻص�
            oor.PreOperation = PreProcessHandle;
            //���ò�������ǰִ�еĻص�
            //oor.PostOperation = ?

            //��ʼ�� OB_CALLBACK_REGISTRATION 
            // ���ð汾�ţ�����ΪOB_FLT_REGISTRATION_VERSION
            ocr.Version = OB_FLT_REGISTRATION_VERSION;
            //�����Զ������������ΪNULL
            ocr.RegistrationContext = NULL;
            // ���ûص���������
            ocr.OperationRegistrationCount = 1;
            //���ûص�������Ϣ�ṹ��,��������ж��,��Ҫ����Ϊ����.
            ocr.OperationRegistration = &oor;
            RtlInitUnicodeString(&ocr.Altitude, L"321000"); // ���ü���˳��

            if (NT_SUCCESS(ObRegisterCallbacks(&ocr, &pRegistrationHandle)))
            {
                DbgPrint("[���ұ���]��ɱ������\n");
                ret = STATUS_SUCCESS;
            }
            else
            {
                DbgPrint("[���ұ���]��ɱ����ʧ�ܣ�\n");
                ret = STATUS_UNSUCCESSFUL;
            }
            break;
        }

        default:
        {
            DbgPrint("ע���Modeֵ����!");
            ret = STATUS_UNSUCCESSFUL;
            break;
        }
            

    }

    return ret;
}
