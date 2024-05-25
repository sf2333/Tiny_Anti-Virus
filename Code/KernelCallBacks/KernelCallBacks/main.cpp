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

//全局变量
LARGE_INTEGER	cookie;

extern "C"
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegisterPath)
{
	PDEVICE_OBJECT pdoDeviceObj = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pdoGlobalDrvObj = DriverObject;

	DbgPrint("[主动防御]系统回调驱动加载\r\n");
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

	//注入R3HOOK的dll 实现api监控
	startInject("MyHookDll.dll","add");

	//注册注册表回调
	CmRegisterCallback(RegistryCallback, NULL, &cookie);


	//注册进程回调

	return STATUS_SUCCESS;
}

VOID UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	//取消dll注入
	stopInject();

	//注销注册表回调
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
	DbgPrint("[主动防御]系统回调驱动卸载\r\n"); 
}

