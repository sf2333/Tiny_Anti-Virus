// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include "detours.h"
using namespace std;

//根据不同编译目标调用不同detours.lib
#ifdef _WIN64
#pragma comment(lib,"detours_x64.lib")
#else
#pragma comment(lib,"detours_x86.lib")
#endif


//打印debug信息 显示到debugview中
//OUTINFO_0_PARAM表示输出纯字符串，OUTINFO_1_PARAM表示可以携带一个参数，以此类推2、3
#define OUTINFO_0_PARAM(fmt) {CHAR sOut[256];CHAR sfmt[50];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt));OutputDebugStringA(sOut);}    
#define OUTINFO_1_PARAM(fmt,var) {CHAR sOut[256];CHAR sfmt[50];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt),var);OutputDebugStringA(sOut);}    

//导出函数
extern "C" __declspec(dllexport) void add();

//全局变量
WCHAR CurFullPath[MAX_PATH] = { 0 };

typedef enum rule1_funcs
{
    OpenProcess_index = 0,
    VirtualAlloc_index,
    WriteProcessMemory_index,
    CreateRemoteThread_index,
}rule1_funcs;
rule1_funcs my_rule1_funcs;

BOOL Rule1[4] = { FALSE };


void add()
{
    return;
}

void DebugPrint(LPCWSTR message)
{
    MessageBoxW(NULL, message, L"Debug Information", MB_OK | MB_ICONINFORMATION);
}

void Log2txt(WCHAR* content)
{

    FILE* fp;
    //打开日志文件
    fopen_s(&fp, "C:\\Users\\awqhc\\Desktop\\IAT\\MyLogs.txt", "a+");

    if (fp == NULL) {
        OUTINFO_0_PARAM("hook.dll中打开日志文件失败!\n");
        return;
    }

    time_t now = time(NULL);
    wchar_t time_str[20];
    wcsftime(time_str, sizeof(time_str), L"%Y-%m-%d %H:%M", localtime(&now));
    fwprintf(fp, L"Log-%s %s\n", time_str, content);  // fwprintf所以使用%s 如fprintf则使用%ws
 
    //关闭文件
    fclose(fp);
}

void Alarm()
{
    WCHAR Msg[MAX_PATH] = L"已拦截恶意API调用行为：";
    wcscat(Msg, CurFullPath);
    MessageBoxW(NULL, Msg, TEXT("警告"), MB_OK);
}

int IsSatisfyRule1(rule1_funcs CurFunc)
{
    for(int i = OpenProcess_index; i < CurFunc; i++)
    {
        if (Rule1[i] == FALSE)
        {
            //不满足调用链清空
            for (int j = OpenProcess_index; j <= CreateRemoteThread_index; j++)
            {
                Rule1[j] = FALSE;
            }
            return 0; //未按顺序调用，不拦截
        }
       
    }

    if (Rule1[CreateRemoteThread_index])
    {
        
        return -1; //拦截
    }


    return 1; //正常调用,继续观察
}

//定义Old_Funcs
static HANDLE(WINAPI* Old_OpenProcess)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId) = OpenProcess;

static LPVOID(WINAPI* Old_VirtualAllocEx)(
    _In_ HANDLE hProcess,
    _In_opt_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flAllocationType,
    _In_ DWORD flProtect) = VirtualAllocEx;

static BOOL(WINAPI* Old_WriteProcessMemory)(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpBaseAddress,
    _In_reads_bytes_(nSize) LPCVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ SIZE_T* lpNumberOfBytesWritten
    ) = WriteProcessMemory;

static HANDLE(WINAPI* Old_CreateRemoteThread)(
    _In_ HANDLE hProcess,
    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ SIZE_T dwStackSize,
    _In_ LPTHREAD_START_ROUTINE lpStartAddress,
    _In_opt_ LPVOID lpParameter,
    _In_ DWORD dwCreationFlags,
    _Out_opt_ LPDWORD lpThreadId
    ) = CreateRemoteThread;

static HMODULE(WINAPI* Old_LoadLibraryA)(LPCSTR lpLibFileName) = LoadLibraryA;

static HMODULE(WINAPI* Old_LoadLibraryW)(LPCWSTR lpLibFileName) = LoadLibraryW;


//编写Hook函数
HANDLE WINAPI New_OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId) 
{
    /*WCHAR message[MAX_PATH];
    wcscpy(message, CurFullPath); 
    wcscat(message, L" 调用OpenProcess");*/
        
    Rule1[OpenProcess_index] = TRUE;
    OUTINFO_0_PARAM("调用OpenProcess!");

    int flag;
    flag = IsSatisfyRule1(OpenProcess_index);
    switch (flag)
    {
        case 0:
        {
            OUTINFO_0_PARAM("不满足调用链!")
            break;
        }
        case 1:
        {
            OUTINFO_0_PARAM("满足当前调用链!")
                break;
        }
        case -1:
        {
            OUTINFO_0_PARAM("触发规则1！");
            Alarm();
            return 0;
        }
        default:
        {
            OUTINFO_1_PARAM("flag错误 %d", flag);
            break;
        }
    }

    return Old_OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);

}

LPVOID WINAPI  New_VirtualAllocEx(
    _In_ HANDLE hProcess,
    _In_opt_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flAllocationType,
    _In_ DWORD flProtect)
{
    Rule1[VirtualAlloc_index] = TRUE;
    OUTINFO_0_PARAM("调用VirtualAllocEx!");
    
    int flag;
    flag = IsSatisfyRule1(OpenProcess_index);
    switch (flag)
    {
        case 0:
        {
            OUTINFO_0_PARAM("不满足调用链!")
                break;
        }
        case 1:
        {
            OUTINFO_0_PARAM("满足当前调用链!")
                break;
        }
        case -1:
        {
            OUTINFO_0_PARAM("触发规则1！");
            Alarm();
            return 0;
        }
        default:
        {
            OUTINFO_1_PARAM("flag错误 %d", flag);
            break;
        }
    }

    return Old_VirtualAllocEx
        (
            _In_ hProcess,
            _In_opt_ lpAddress,
            _In_ dwSize,
            _In_ flAllocationType,
            _In_ flProtect
        );
}

BOOL New_WriteProcessMemory(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpBaseAddress,
    _In_reads_bytes_(nSize) LPCVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ SIZE_T* lpNumberOfBytesWritten
)
{
    Rule1[WriteProcessMemory_index] = TRUE;
    OUTINFO_0_PARAM("调用WriteProcessMemory!");

    int flag;
    flag = IsSatisfyRule1(OpenProcess_index);
    switch (flag)
    {
        case 0:
        {
            OUTINFO_0_PARAM("不满足调用链!")
                break;
        }
        case 1:
        {
            OUTINFO_0_PARAM("满足当前调用链!")
                break;
        }
        case -1:
        {
            OUTINFO_0_PARAM("触发规则1！");
            Alarm();
            return 0;
        }
        default:
        {
            OUTINFO_1_PARAM("flag错误 %d", flag);
            break;
        }
    }

    return  Old_WriteProcessMemory
    (
        _In_ hProcess,
        _In_ lpBaseAddress,
        _In_reads_bytes_(nSize) lpBuffer,
        _In_ nSize,
        _Out_opt_ lpNumberOfBytesWritten
    );
}

HANDLE New_CreateRemoteThread(
    _In_ HANDLE hProcess,
    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ SIZE_T dwStackSize,
    _In_ LPTHREAD_START_ROUTINE lpStartAddress,
    _In_opt_ LPVOID lpParameter,
    _In_ DWORD dwCreationFlags,
    _Out_opt_ LPDWORD lpThreadId
)
{
    Rule1[CreateRemoteThread_index] = TRUE;
    OUTINFO_0_PARAM("调用CreateRemoteThread!");

    int flag;
    flag = IsSatisfyRule1(OpenProcess_index);
    switch (flag)
    {
        case 0:
        {
            OUTINFO_0_PARAM("不满足调用链!")
                break;
        }
        case 1:
        {
            OUTINFO_0_PARAM("满足当前调用链!")
                break;
        }
        case -1:
        {
            OUTINFO_0_PARAM("触发规则1！");
            Alarm();
            return 0;
        }
        default:
        {
            OUTINFO_1_PARAM("flag错误 %d", flag);
            break;
        }
    }

    return  Old_CreateRemoteThread
    (
        _In_ hProcess,
        _In_opt_ lpThreadAttributes,
        _In_ dwStackSize,
        _In_ lpStartAddress,
        _In_opt_ lpParameter,
        _In_ dwCreationFlags,
        _Out_opt_ lpThreadId
    );
}

HMODULE WINAPI New_LoadLibraryA(LPCSTR lpLibFileName)
{
    //Rule1[LoadLibrary_index] = TRUE;
    OUTINFO_0_PARAM("调用LoadLibraryA!");
    return Old_LoadLibraryA(lpLibFileName);
}

HMODULE WINAPI New_LoadLibraryW(LPCWSTR lpLibFileName)
{
    //Rule1[LoadLibrary_index] = TRUE;
    OUTINFO_0_PARAM("调用LoadLibraryW!");
    return Old_LoadLibraryW(lpLibFileName);
}

void Hook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    // 参数一是原函数地址，参数二是新函数地址

    //可以Attach多个API
    DetourAttach((PVOID*)&Old_OpenProcess, New_OpenProcess);
    DetourAttach((PVOID*)&Old_VirtualAllocEx, New_VirtualAllocEx);
    DetourAttach((PVOID*)&Old_WriteProcessMemory, New_WriteProcessMemory);
    DetourAttach((PVOID*)&Old_CreateRemoteThread, New_CreateRemoteThread);
    DetourAttach((PVOID*)&Old_LoadLibraryA, New_LoadLibraryA);
    DetourAttach((PVOID*)&Old_LoadLibraryW, New_LoadLibraryW);
    DetourTransactionCommit();
}

void UnHook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // 和Hook完全一样，不同的只是将DetourAttach换成DetourDetach
    DetourDetach((PVOID*)&Old_OpenProcess, New_OpenProcess);
    DetourDetach((PVOID*)&Old_VirtualAllocEx, New_VirtualAllocEx);
    DetourDetach((PVOID*)&Old_WriteProcessMemory, New_WriteProcessMemory);
    DetourDetach((PVOID*)&Old_CreateRemoteThread, New_CreateRemoteThread);
    DetourDetach((PVOID*)&Old_LoadLibraryA, New_LoadLibraryA);
    DetourDetach((PVOID*)&Old_LoadLibraryW, New_LoadLibraryW);
    DetourTransactionCommit();
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //设置输出为中文
        setlocale(LC_ALL, "CHS");

        //获取当前进程exe全路径
        GetModuleFileName(NULL, CurFullPath, MAX_PATH);
        Hook();
        OUTINFO_1_PARAM("成功注入目标进程 %ws!", CurFullPath);
        
        break;

    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        UnHook();
        break;
    }
    return TRUE;
}

