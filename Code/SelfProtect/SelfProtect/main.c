#include "hide.h"
#include "protect.h"
#include "globals.h"


// 查询Key键中的Value值
BOOLEAN RegQueryValueKey(PUNICODE_STRING RegisterPath, LPWSTR ValueName, PKEY_VALUE_PARTIAL_INFORMATION* pkvpi)
{
    ULONG ulSize;
    NTSTATUS ntStatus;
    PKEY_VALUE_PARTIAL_INFORMATION pvpi;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE hRegister;
    UNICODE_STRING usValueName;

    RtlInitUnicodeString(&usValueName, ValueName);

    // 初始化
    InitializeObjectAttributes(&objectAttributes, RegisterPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    // 打开注册表Key
    ntStatus = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
    if (!NT_SUCCESS(ntStatus))
    {
        return FALSE;
    }

    // 查询长度
    ntStatus = ZwQueryValueKey(hRegister, &usValueName, KeyValuePartialInformation, NULL, 0, &ulSize);
    if (ntStatus == STATUS_OBJECT_NAME_NOT_FOUND || ulSize == 0)
    {
        return FALSE;
    }

    // 分配空间保存查询结果
    pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
    ntStatus = ZwQueryValueKey(hRegister, &usValueName, KeyValuePartialInformation, pvpi, ulSize, &ulSize);
    if (!NT_SUCCESS(ntStatus))
    {
        return FALSE;
    }

    // 这里的pvpi未被释放，可在外部释放
    // 执行 ExFreePool(pvpi); 释放
    *pkvpi = pvpi;
    return TRUE;
}

int QueryStartMode(_In_ PUNICODE_STRING RegistryPath)
{
    BOOLEAN flag = FALSE;
    DWORD64 get_dw = 0;
    PKEY_VALUE_PARTIAL_INFORMATION pkvi;
    int StartMode = 0;
    // 查询设置
    flag = RegQueryValueKey(RegistryPath, L"Mode", &pkvi);
    if (flag == TRUE)
    {
        // 拷贝查询结果
        RtlCopyMemory(&get_dw, pkvi->Data, pkvi->DataLength);

        // 输出结果
        DbgPrint("[*] 查询注册表Mode结果: %d \n", get_dw);
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
    // 查询设置
    flag = RegQueryValueKey(RegistryPath, L"Pid", &pkvi);
    if (flag == TRUE)
    {
        // 拷贝查询结果
        RtlCopyMemory(&get_dw, pkvi->Data, pkvi->DataLength);

        // 输出结果
        DbgPrint("[*] 查询注册表PID结果: %d \n", get_dw);
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
                DbgPrint("[自我保护]隐藏取消！\n");
            else
                DbgPrint("[自我保护]隐藏取消失败！\n");
            break;
        }
        case 1:
        {
            if (NULL != pRegistrationHandle)
            {
                DbgPrint("[自我保护]防杀取消！\n");
                ObUnRegisterCallbacks(pRegistrationHandle);
                pRegistrationHandle = NULL;
            }
            break;
        }
        default:
            break;
    }
    
    DbgPrint("[自我保护]驱动程序卸载成功! \n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS ret = STATUS_SUCCESS;
    int cur_pid;

    DriverObject->DriverUnload = UnDriver;

    //过PG
    bypass_signcheck(DriverObject);

    //判断开启进程隐藏还是防杀
    mode = QueryStartMode(RegistryPath);
    cur_pid = QueryCurPid(RegistryPath);

    //获取目标进程eprocess地址
    PRoc = GetProcessObjectByID(cur_pid);
    if (PRoc != NULL)
    {
        DbgPrint("[自我保护]PRoc %p\n", PRoc);
    }
    else
    {
        DbgPrint("[自我保护]PRoc获取失败\n");
        return STATUS_UNSUCCESSFUL;
    }

    switch (mode)
    {
        case 0:
        {
            //进程隐藏
            gsave_pid = NULL;
            gsave_pid = ControlProcessHide(PRoc, gsave_pid);
            ret = STATUS_SUCCESS;
            DbgPrint("[自我保护]隐藏开启！\n");
           
            break;
        }
        case 1:
        {

            //进程防杀
            pRegistrationHandle = 0;

            OB_CALLBACK_REGISTRATION ocr;
            OB_OPERATION_REGISTRATION oor;

            //初始化 OB_OPERATION_REGISTRATION 
            RtlZeroMemory(&oor, sizeof(OB_OPERATION_REGISTRATION));
            RtlZeroMemory(&ocr, sizeof(OB_CALLBACK_REGISTRATION));

            //设置监听的对象类型
            oor.ObjectType = PsProcessType;
            //设置监听的操作类型
            oor.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
            //设置操作发生前执行的回调
            oor.PreOperation = PreProcessHandle;
            //设置操作发生前执行的回调
            //oor.PostOperation = ?

            //初始化 OB_CALLBACK_REGISTRATION 
            // 设置版本号，必须为OB_FLT_REGISTRATION_VERSION
            ocr.Version = OB_FLT_REGISTRATION_VERSION;
            //设置自定义参数，可以为NULL
            ocr.RegistrationContext = NULL;
            // 设置回调函数个数
            ocr.OperationRegistrationCount = 1;
            //设置回调函数信息结构体,如果个数有多个,需要定义为数组.
            ocr.OperationRegistration = &oor;
            RtlInitUnicodeString(&ocr.Altitude, L"321000"); // 设置加载顺序

            if (NT_SUCCESS(ObRegisterCallbacks(&ocr, &pRegistrationHandle)))
            {
                DbgPrint("[自我保护]防杀开启！\n");
                ret = STATUS_SUCCESS;
            }
            else
            {
                DbgPrint("[自我保护]防杀开启失败！\n");
                ret = STATUS_UNSUCCESSFUL;
            }
            break;
        }

        default:
        {
            DbgPrint("注册表Mode值有误!");
            ret = STATUS_UNSUCCESSFUL;
            break;
        }
            

    }

    return ret;
}
