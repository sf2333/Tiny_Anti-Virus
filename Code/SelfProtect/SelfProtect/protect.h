#include "uilt.h"

//���̹�������ϸ�����������
#define PROCESS_TERMINATE_0       0x1001
//taskkillָ���������
#define PROCESS_TERMINATE_1       0x0001 
//taskkillָ���/f����ǿɱ���̽�����
#define PROCESS_KILL_F			  0x1401
//���̹�������������
#define PROCESS_TERMINATE_2       0x1041


OB_PREOP_CALLBACK_STATUS PreProcessHandle(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION pOperationInformation
);
