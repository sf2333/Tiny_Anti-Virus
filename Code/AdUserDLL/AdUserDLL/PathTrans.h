//FDR3
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#define MY_CUSTOM_IOCTL_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct
{
    wchar_t nt_path[MAX_PATH];      //nt路径 
    wchar_t device_path[MAX_PATH];  //设备路径

}path_packet;

BOOL Char2Wchar(char* mbString, wchar_t** wideStringPtr);

BOOL NtPathToDevicePath(wchar_t* pszNtPath, wchar_t* pszDevicePath);