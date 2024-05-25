#include "protect.h"
#include "globals.h"

OB_PREOP_CALLBACK_STATUS PreProcessHandle(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION pOperationInformation
)
{
	PEPROCESS pEProcess = NULL;

	// �ж϶������� �Ƿ��ǽ���
	if (*PsProcessType != pOperationInformation->ObjectType)
	{
		return OB_PREOP_SUCCESS;
	}

	//��ȡ�ý��̽ṹ���������
	pEProcess = (PEPROCESS)pOperationInformation->Object;
	// �ж��Ƿ�Ϊ�������̣����������
	if (pEProcess != PRoc)
	{
		//DbgPrint("[PROC]%p %p", pEProcess, PRoc);
		return OB_PREOP_SUCCESS;
	}
	

	// �жϲ�������,����þ������ֹ��������ܾ��ò���
	switch (pOperationInformation->Operation)
	{

		//����/�߳̾����dup(����)�Ĳ���
		case OB_OPERATION_HANDLE_DUPLICATE:
			break;
	
		//����/�߳̾��������ʱ���ûص�
		case OB_OPERATION_HANDLE_CREATE:
		{   
			//���Ҫ��������,���̹������������̷���0x1001��taskkillָ��������̷���0x0001��taskkil��/f�����������̷���0x1401
			int code = pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess;

			if ((code  == PROCESS_TERMINATE_0) || (code == PROCESS_TERMINATE_1) || (code == PROCESS_KILL_F))
			{
				//�����̸�����Ȩ��
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0x0;
				DbgPrint("�ܾ�ִ�е�ǰ����!");
			}

			if (code == PROCESS_TERMINATE_2)
			{
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess = STANDARD_RIGHTS_ALL;
				DbgPrint("�ܾ�ִ�е�ǰ����PROCESS_TERMINATE_2!");
			}
			
			break;
		}
	}
	return OB_PREOP_SUCCESS;
}

