// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <stdexcept>
using namespace std;

#define ExportMyDLL extern "C" __declspec(dllexport)

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"advapi32.lib")

//导出函数
ExportMyDLL BOOL InstallMinifilter(PCHAR m_pSysPath, PCHAR m_pServiceName, PCHAR m_pDisplayName, PCHAR m_pMiniAltitude);
ExportMyDLL BOOL InstallDriver(PCHAR m_pSysPath, PCHAR m_pServiceName, PCHAR m_pDisplayName);
ExportMyDLL BOOL Start(PCHAR m_pServiceName);
ExportMyDLL BOOL Stop(PCHAR m_pServiceName);
ExportMyDLL BOOL Remove(PCHAR m_pServiceName);


//打印debug信息
//OUTINFO_0_PARAM表示输出纯字符串，OUTINFO_1_PARAM表示可以携带一个参数，以此类推2、3
#define OUTINFO_0_PARAM(fmt) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt));OutputDebugStringA(sOut);}    
#define OUTINFO_1_PARAM(fmt,var) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt),var);OutputDebugStringA(sOut);}    



//-------------------------------------------
// 驱动安装
// SYS文件跟程序放在同个目录下
//-------------------------------------------

// 获取服务句柄
BOOL GetSvcHandle(PCHAR m_pServiceName)
{
	SC_HANDLE        m_hSCManager;
	SC_HANDLE        m_hService;
	m_hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == m_hSCManager)
	{
		int m_dwLastError = GetLastError();
		OUTINFO_1_PARAM("无法打开注册表管理器! 错误代码：%d\n", m_dwLastError);
		return FALSE;
	}
	m_hService = OpenServiceA(m_hSCManager, m_pServiceName, SERVICE_ALL_ACCESS);
	if (NULL == m_hService)
	{
		CloseServiceHandle(m_hSCManager);
		return FALSE;
	}

	CloseServiceHandle(m_hService);
	CloseServiceHandle(m_hSCManager);
	return TRUE;
}

// 安装驱动
BOOL InstallDriver(PCHAR m_pSysPath, PCHAR m_pServiceName, PCHAR m_pDisplayName)
{
	SC_HANDLE        m_hSCManager;
	SC_HANDLE        m_hService;
	int              m_dwLastError;
	char SysPath[MAX_PATH] = { 0 };

	//得到完整的驱动路径
	GetFullPathNameA(m_pSysPath, MAX_PATH, SysPath, NULL);

	m_hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == m_hSCManager)
	{
		m_dwLastError = GetLastError();
		OUTINFO_1_PARAM("无法打开注册表管理器! 错误代码：%d\n", m_dwLastError);
		return FALSE;
	}
	m_hService = CreateServiceA(m_hSCManager, m_pServiceName, m_pDisplayName,
		SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		SysPath, NULL, NULL, NULL, NULL, NULL);
	if (NULL == m_hService)
	{
		m_dwLastError = GetLastError();


		if (ERROR_SERVICE_EXISTS == m_dwLastError) //指定的服务已存在于此数据库中。
		{
			OUTINFO_0_PARAM("指定服务已安装! 尝试直接打开\n");
			m_hService = OpenServiceA(m_hSCManager, m_pServiceName, SERVICE_ALL_ACCESS);
			if (NULL == m_hService)
			{
				CloseServiceHandle(m_hSCManager);
				return FALSE;
			}
		}
		else
		{
			OUTINFO_1_PARAM("创建服务出错! 错误代码：%d\n", m_dwLastError);
			CloseServiceHandle(m_hSCManager);
			return FALSE;
		}
	}

	CloseServiceHandle(m_hService);
	CloseServiceHandle(m_hSCManager);
	return TRUE;
}

BOOL InstallMinifilter(PCHAR m_pSysPath, PCHAR m_pServiceName, PCHAR m_pDisplayName, PCHAR m_pMiniAltitude)
{
	HKEY    hKey;
	DWORD    dwData;
	char SysPath[MAX_PATH] = { 0 };
	char szTempStr[MAX_PATH] = { 0 };
	//得到完整的驱动路径
	
	GetFullPathNameA(m_pSysPath, MAX_PATH, SysPath, NULL);

	if (NULL == m_pServiceName || NULL == SysPath)
	{
		return FALSE;
	}
	//得到完整的驱动路径
	// GetFullPathNameA(lpszDriverPath, MAX_PATH, szDriverImagePath, NULL);

	SC_HANDLE hServiceMgr = NULL;// SCM管理器的句柄
	SC_HANDLE hService = NULL;// NT驱动程序的服务句柄

	//打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		// OpenSCManager失败
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}

	// OpenSCManager成功  

	//创建驱动所对应的服务
	hService = CreateServiceA(hServiceMgr,
		m_pServiceName,             // 驱动程序的在注册表中的名字
		m_pDisplayName,             // 注册表驱动程序的DisplayName 值
		SERVICE_ALL_ACCESS,         // 加载驱动程序的访问权限
		SERVICE_FILE_SYSTEM_DRIVER, // 表示加载的服务是文件系统驱动程序
		SERVICE_DEMAND_START,       // 注册表驱动程序的Start 值
		SERVICE_ERROR_IGNORE,       // 注册表驱动程序的ErrorControl 值
		SysPath,                 // 注册表驱动程序的ImagePath 值
		"FSFilter Activity Monitor",// 注册表驱动程序的Group 值
		NULL,
		"FltMgr",                   // 注册表驱动程序的DependOnService 值
		NULL,
		NULL);

	if (hService == NULL)
	{
		OUTINFO_1_PARAM("CreateServiceA时发生错误，内部错误码;%d", GetLastError());

		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			//服务创建失败，是由于服务已经创立过
			OUTINFO_0_PARAM("指定服务已安装! 尝试直接打开\n");
			CloseServiceHandle(hService);       // 服务句柄
			CloseServiceHandle(hServiceMgr);    // SCM句柄
			return FALSE;
		}
		else if (GetLastError() == ERROR_SERVICE_MARKED_FOR_DELETE)
		{
			//服务创建失败，是由于服务已经被标记为删除
			//可能原因是驱动程序卸载中途发生错误，sc管理器未关闭，需要重启系统
			//OUTINFO_0_PARAM("需要重启系统!");
			OUTINFO_0_PARAM("Minifilter驱动在上一次卸载中途发生错误，sc管理器未关闭，需要重启系统!");
			CloseServiceHandle(hService);       // 服务句柄
			CloseServiceHandle(hServiceMgr);    // SCM句柄
			return FALSE;
		}
		else
		{
			OUTINFO_0_PARAM("未知错误直接退出!")
			CloseServiceHandle(hService);       // 服务句柄
			CloseServiceHandle(hServiceMgr);    // SCM句柄
			return FALSE;
		}
	}


	CloseServiceHandle(hService);       // 服务句柄
	CloseServiceHandle(hServiceMgr);    // SCM句柄

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances子健下的键值项 
	//-------------------------------------------------------------------------------------------------------
	strcpy_s(szTempStr, MAX_PATH, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat_s(szTempStr, MAX_PATH, m_pServiceName);
	strcat_s(szTempStr, MAX_PATH, "\\Instances");

	if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, szTempStr, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	// 注册表驱动程序的DefaultInstance 值 
	strcpy_s(szTempStr, MAX_PATH, m_pServiceName);
	strcat_s(szTempStr, MAX_PATH, " Instance");

	if (RegSetValueExA(hKey, "DefaultInstance", 0, REG_SZ, (CONST BYTE*)szTempStr, (DWORD)strlen(szTempStr)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances\\DriverName Instance子健下的键值项 
	//-------------------------------------------------------------------------------------------------------
	strcpy_s(szTempStr, MAX_PATH, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat_s(szTempStr, MAX_PATH, m_pServiceName);
	strcat_s(szTempStr, MAX_PATH, "\\Instances\\");
	strcat_s(szTempStr, MAX_PATH, m_pServiceName);
	strcat_s(szTempStr, MAX_PATH, " Instance");


	if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, szTempStr, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	// 注册表驱动程序的Altitude 值
	strcpy_s(szTempStr, MAX_PATH, m_pMiniAltitude);
	if (RegSetValueExA(hKey, "Altitude", 0, REG_SZ, (CONST BYTE*)szTempStr, (DWORD)strlen(szTempStr)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	// 注册表驱动程序的Flags 值
	dwData = 0x0;
	if (RegSetValueExA(hKey, "Flags", 0, REG_DWORD, (CONST BYTE*) & dwData, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);
	
	return TRUE;
}

BOOL Start(PCHAR m_pServiceName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;

	if (NULL == m_pServiceName)
	{
		OUTINFO_1_PARAM("内部错误码;%d", GetLastError());
		return FALSE;
	}

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		OUTINFO_1_PARAM("内部错误码;%d", GetLastError());
		CloseServiceHandle(schManager);
		return FALSE;
	}
	schService = OpenServiceA(schManager, m_pServiceName, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		OUTINFO_1_PARAM("内部错误码;%d", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	if (!StartService(schService, 0, NULL))
	{
		OUTINFO_1_PARAM("内部错误码;%d", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		if (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
		{
			OUTINFO_1_PARAM("内部错误码;%d", GetLastError());
			// 服务已经开启
			return FALSE;
		}
		OUTINFO_1_PARAM("内部错误码;%d", GetLastError());
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

BOOL Stop(PCHAR m_pServiceName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;
	bool            bStopped = false;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		return FALSE;
	}
	schService = OpenServiceA(schManager, m_pServiceName, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}
	if (!ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus) && (svcStatus.dwCurrentState != SERVICE_STOPPED))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

BOOL Remove(PCHAR m_pServiceName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		return FALSE;
	}
	schService = OpenServiceA(schManager, m_pServiceName, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}
	ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus);
	if (!DeleteService(schService))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

