// Injectmain.cpp
#include "windows.h"
#include "tchar.h"
BOOL InjectDll(DWORD dwPID, LPCTSTR szDllPath)
{
	HANDLE hProcess = NULL, hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL;
	DWORD dwBufSize = (DWORD)(_tcslen(szDllPath) + 1) * sizeof(TCHAR);
	LPTHREAD_START_ROUTINE pThreadProc;
	// Open target process to inject dll
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		_tprintf(L"Fail to open process %d ! [%d]\n", dwPID, GetLastError());
		return FALSE;
	}
	// Allocate memory in the remote process big enough for the DLL path name
	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);
	// Write the DLL path name to the space allocated in the target process
	WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllPath, dwBufSize, NULL);
	// Find the address of LoadLibrary in target process(same to this process)
	hMod = GetModuleHandle(L"kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryW");
	// Create a remote thread in target process
	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	return TRUE;
}
int _tmain(int argc, TCHAR* argv[])
{
	if (argc != 3)
	{
		_tprintf(L"Usage: %s <pid> <dll_path> \n", argv[0]);
		return 1;
	}
	// Inject DLL
	if (InjectDll((DWORD)_tstol(argv[1]), argv[2]))
		_tprintf(L"InjectDll sucess! \n");
	else
		_tprintf(L"InjectDLL fail! \n");
	return 0;
}