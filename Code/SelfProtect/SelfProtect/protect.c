#include "protect.h"
#include "globals.h"

OB_PREOP_CALLBACK_STATUS PreProcessHandle(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION pOperationInformation
)
{
	PEPROCESS pEProcess = NULL;

	// 判断对象类型 是否是进程
	if (*PsProcessType != pOperationInformation->ObjectType)
	{
		return OB_PREOP_SUCCESS;
	}

	//获取该进程结构对象的名称
	pEProcess = (PEPROCESS)pOperationInformation->Object;
	// 判断是否为保护进程，不是则放行
	if (pEProcess != PRoc)
	{
		//DbgPrint("[PROC]%p %p", pEProcess, PRoc);
		return OB_PREOP_SUCCESS;
	}
	

	// 判断操作类型,如果该句柄是终止操作，则拒绝该操作
	switch (pOperationInformation->Operation)
	{

		//进程/线程句柄被dup(复制)的操作
		case OB_OPERATION_HANDLE_DUPLICATE:
			break;
	
		//进程/线程句柄被创建时调用回调
		case OB_OPERATION_HANDLE_CREATE:
		{   
			//如果要结束进程,进程管理器结束进程发送0x1001，taskkill指令结束进程发送0x0001，taskkil加/f参数结束进程发送0x1401
			int code = pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess;

			if ((code  == PROCESS_TERMINATE_0) || (code == PROCESS_TERMINATE_1) || (code == PROCESS_KILL_F))
			{
				//给进程赋予新权限
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0x0;
				DbgPrint("拒绝执行当前操作!");
			}

			if (code == PROCESS_TERMINATE_2)
			{
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess = STANDARD_RIGHTS_ALL;
				DbgPrint("拒绝执行当前操作PROCESS_TERMINATE_2!");
			}
			
			break;
		}
	}
	return OB_PREOP_SUCCESS;
}

