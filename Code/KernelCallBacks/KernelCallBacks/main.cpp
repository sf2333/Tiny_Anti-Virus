#ifdef __cplusplus
extern "C" {
#endif

#include <ntifs.h>
#include <ntimage.h>
#include <string.h>

#ifdef __cplusplus
}; // extern "C"
#endif

#include "importTableInjectDll.h"
#include "injectWork.h"
#include "registerWork.h"

#ifdef __cplusplus
namespace { // anonymous namespace to limit the scope of this global variable!
#endif
PDRIVER_OBJECT pdoGlobalDrvObj = 0;
#ifdef __cplusplus
}; // anonymous namespace
#endif

//ȫ�ֱ���
LARGE_INTEGER	cookie;

extern "C"
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegisterPath)
{
	PDEVICE_OBJECT pdoDeviceObj = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pdoGlobalDrvObj = DriverObject;

	DbgPrint("[��������]ϵͳ�ص���������\r\n");
	DriverObject->DriverUnload = UnloadDriver;

	// Create the device object.
	if(!NT_SUCCESS(status = IoCreateDevice(
		DriverObject,
		0,
		&usDeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&pdoDeviceObj
		)))
	{
		// Bail out (implicitly forces the driver to unload).
		return status;
	};

	// Now create the respective symbolic link object
	if(!NT_SUCCESS(status = IoCreateSymbolicLink(
		&usSymlinkName,
		&usDeviceName
		)))
	{
		IoDeleteDevice(pdoDeviceObj);
		return status;
	}

	//ע��R3HOOK��dll ʵ��api���
	startInject("MyHookDll.dll","add");

	//ע��ע���ص�
	CmRegisterCallback(RegistryCallback, NULL, &cookie);


	//ע����̻ص�

	return STATUS_SUCCESS;
}

VOID UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	//ȡ��dllע��
	stopInject();

	//ע��ע���ص�
	CmUnRegisterCallback(cookie);

	PDEVICE_OBJECT pdoNextDeviceObj = pdoGlobalDrvObj->DeviceObject;
	IoDeleteSymbolicLink(&usSymlinkName);

	// Delete all the device objects
	while(pdoNextDeviceObj)
	{
		PDEVICE_OBJECT pdoThisDeviceObj = pdoNextDeviceObj;
		pdoNextDeviceObj = pdoThisDeviceObj->NextDevice;
		IoDeleteDevice(pdoThisDeviceObj);
	}
	DbgPrint("[��������]ϵͳ�ص�����ж��\r\n"); 
}

