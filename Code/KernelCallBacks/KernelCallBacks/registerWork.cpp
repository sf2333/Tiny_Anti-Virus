#include "public.h"
#include "registerWork.h"

extern "C"  
NTKERNELAPI UCHAR* PsGetProcessImageFileName
(
    __in PEPROCESS Process
);

extern "C"  
NTKERNELAPI NTSTATUS ObQueryNameString
(
    IN  PVOID Object,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN  ULONG Length,
    OUT PULONG ReturnLength
);

void GetRegisterPath(PUNICODE_STRING pRegistryPath, PVOID pRegistryObject)
{
    SIZE_T ulSize = 1024 * sizeof(WCHAR);
    ULONG ulRetLen = NULL;
    PVOID lpObjectNameInfo = ExAllocatePool(NonPagedPool, ulSize);
    NTSTATUS status = ObQueryNameString(pRegistryObject, (POBJECT_NAME_INFORMATION)lpObjectNameInfo, ulSize, &ulRetLen);
    RtlCopyUnicodeString(pRegistryPath, (PUNICODE_STRING)lpObjectNameInfo);
    ExFreePool(lpObjectNameInfo);
}

void ShowProcessName()
{
    PEPROCESS pEProcess = PsGetCurrentProcess();
    if (NULL != pEProcess)
    {
        UCHAR* lpszProcessName = PsGetProcessImageFileName(pEProcess);
        if (NULL != lpszProcessName)
        {
            DbgPrint("Current Process[%s]\n", lpszProcessName);
        }
    }
}

BOOLEAN Compare(UNICODE_STRING ustrRegPath)
{   
    wchar_t service_path[] = L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet001\\Services\\";
    wchar_t* white_list[2] = { L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet001\\Services\\W32Time\\SecureTimeLimits", L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet001\\Services\\bam\\State\\" };

    //ƥ�䵽����·��
    if (!wcsncmp(ustrRegPath.Buffer, service_path, wcslen(service_path)))
    {    
        for (int i = 0; i < 2; i++)
        {
            //����ǰ��������·��������������
            if (!wcsncmp(ustrRegPath.Buffer, white_list[i], wcslen(white_list[i])))
            {
                DbgPrint("����������·����%ls\n", ustrRegPath.Buffer);
                return FALSE;
            }
        }

        //DbgPrint("ע���·����%ls\n", ustrRegPath.Buffer);
        return TRUE;
    }

    return FALSE;
}

NTSTATUS RegistryCallback(_In_ PVOID CallbackContext, _In_opt_ PVOID Argument1, _In_opt_ PVOID Argument2)
{

    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING ustrRegPath;

    // ��������������
    LONG lOperateType = (LONG)Argument1;

    ustrRegPath.Length = 0;
    ustrRegPath.MaximumLength = 1024 * sizeof(WCHAR);
    ustrRegPath.Buffer = (PWCH)ExAllocatePool(NonPagedPool, ustrRegPath.MaximumLength);
    if (NULL == ustrRegPath.Buffer)
    {
        DbgPrint("ExAllocatePool error��\n");
        return status;
    }

    //DbgPrint("[lOperateType]--%d\n", lOperateType);

    switch (lOperateType)
    {
       
         // ����ע���֮ǰ 
        case RegNtPreCreateKey:
        {
            GetRegisterPath(&ustrRegPath, ((PREG_CREATE_KEY_INFORMATION)Argument2)->RootObject);
            if (Compare(ustrRegPath))
            {
                //������
                DbgPrint("[RegNtPreCreateKey][%wZ][%wZ]\n", &ustrRegPath, ((PREG_CREATE_KEY_INFORMATION)Argument2)->CompleteName);
            }

            break;
        }

        //win10ʹ�����create
        case RegNtPreCreateKeyEx:
        {
            /*
            GetRegisterPath(&ustrRegPath, ((PREG_CREATE_KEY_INFORMATION)Argument2)->RootObject);

            if (Compare(ustrRegPath))
            {
                //����
                ShowProcessName();
                status = STATUS_ACCESS_DENIED;
                DbgPrint("[RegNtPreCreateKeyEx][%wZ][%wZ]\n", &ustrRegPath, ((PREG_CREATE_KEY_INFORMATION)Argument2)->CompleteName);
            }
            */

            break;
        }

        // ��ע���֮ǰ
        case RegNtPreOpenKey:
        { 
            GetRegisterPath(&ustrRegPath, ((PREG_OPEN_KEY_INFORMATION)Argument2)->RootObject);
           
            if (Compare(ustrRegPath))
            {
                //������
                DbgPrint("[RegNtPreOpenKey][%wZ][%wZ]\n", &ustrRegPath, ((PREG_OPEN_KEY_INFORMATION)Argument2)->CompleteName);
            }
            break;
        }

        //win10ʹ�����open
        case RegNtPreOpenKeyEx:
        {
            /*
            GetRegisterPath(&ustrRegPath, ((PREG_OPEN_KEY_INFORMATION)Argument2)->RootObject);

            if (Compare(ustrRegPath))
            {
                //������
                DbgPrint("[RegNtPreOpenKeyEx][%wZ][%wZ]\n", &ustrRegPath, ((PREG_OPEN_KEY_INFORMATION)Argument2)->CompleteName);
            }
            */
            break;
        }

        // ɾ����֮ǰ
        case RegNtPreDeleteKey:
        {
            GetRegisterPath(&ustrRegPath, ((PREG_DELETE_KEY_INFORMATION)Argument2)->Object);

            if (Compare(ustrRegPath))
            {
                ShowProcessName();
                DbgPrint("[RegNtPreDeleteKey][%wZ]\n", &ustrRegPath);
                status = STATUS_ACCESS_DENIED;
            }
            break;
        }

        // ɾ����ֵ֮ǰ
        case RegNtPreDeleteValueKey:
        {
            GetRegisterPath(&ustrRegPath, ((PREG_DELETE_VALUE_KEY_INFORMATION)Argument2)->Object);

            if (Compare(ustrRegPath))
            {
                ShowProcessName();
                DbgPrint("[RegNtPreDeleteValueKey][%wZ][%wZ]\n", &ustrRegPath, ((PREG_DELETE_VALUE_KEY_INFORMATION)Argument2)->ValueName);
                status = STATUS_ACCESS_DENIED;
            }
            
            break;
        }

        // �޸ļ�ֵ֮ǰ
        case RegNtPreSetValueKey:
        {
            GetRegisterPath(&ustrRegPath, ((PREG_SET_VALUE_KEY_INFORMATION)Argument2)->Object);

            if (Compare(ustrRegPath))
            {
                ShowProcessName();
                DbgPrint("[RegNtPreSetValueKey][%wZ][%wZ]\n", &ustrRegPath, ((PREG_SET_VALUE_KEY_INFORMATION)Argument2)->ValueName);
                status = STATUS_ACCESS_DENIED;
            }
            break;
        }

        default:
            break;
    }

    if (NULL != ustrRegPath.Buffer)
    {
        ExFreePool(ustrRegPath.Buffer);
        ustrRegPath.Buffer = NULL;
    }

    return status;
}


