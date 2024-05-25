#include "PathTrans.h"
#include "ulity.h"


BOOL Char2Wchar(char* mbString, wchar_t** wideStringPtr) //传入wideString的指针地址
{
    int wideLength = MultiByteToWideChar(CP_ACP, 0, mbString, -1, NULL, 0);
    if (wideLength == 0) {
        OUTINFO_0_PARAM("转换失败1\n");
        return FALSE;
    }
    wchar_t* wideString = (wchar_t*)malloc(wideLength * sizeof(wchar_t));
    if (wideString == NULL) {
        OUTINFO_0_PARAM("内存分配失败\n");
        return FALSE;
    }
    if (MultiByteToWideChar(CP_ACP, 0, mbString, -1, wideString, wideLength) == 0) {
        OUTINFO_0_PARAM("转换失败2\n");
        free(wideString);
        return FALSE;
    }
    //OUTINFO_0_PARAM("输入路径(宽字符字符串)：%ls\n", wideString);
    *wideStringPtr = wideString;  // 将分配的内存地址赋值给传入的指针
    return TRUE;
}

BOOL NtPathToDevicePath(wchar_t* pszNtPath, wchar_t* pszDevicePath)
{
    static TCHAR    szDriveStr[MAX_PATH] = { 0 };
    static TCHAR    szDevName[MAX_PATH] = { 0 };
    TCHAR            szDrive[3];
    INT             cchDevName;
    INT             i;

    //检查参数  
    if (IsBadWritePtr(pszNtPath, 1) != 0)return FALSE;
    if (IsBadReadPtr(pszDevicePath, 1) != 0)return FALSE;


    //获取本地磁盘字符串  
    ZeroMemory(szDriveStr, ARRAYSIZE(szDriveStr));
    ZeroMemory(szDevName, ARRAYSIZE(szDevName));

    if (GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))
    {
        for (i = 0; szDriveStr[i]; i += 4)
        {
            //跳过A和B盘
            if (!lstrcmpi(&(szDriveStr[i]), _T("A:\\")) || !lstrcmpi(&(szDriveStr[i]), _T("B:\\")))
                continue;

            //设置盘符 长度为2 格式为C:
            szDrive[0] = szDriveStr[i];
            szDrive[1] = szDriveStr[i + 1];
            szDrive[2] = '\0';

            //查询盘符对应的Dos设备名  格式为\Device\HarddiskVolume1
            if (!QueryDosDevice(szDrive, szDevName, MAX_PATH))
            {
                OUTINFO_0_PARAM("查询Dos设备名失败!");
                return FALSE;
            }

            //取Dos设备长度
            cchDevName = lstrlen(szDevName);
            //OUTINFO_0_PARAM("%ls %ls %d\n", szDrive, szDevName, cchDevName);

            if (_tcsnicmp(pszNtPath, szDrive, 2) == 0)//命中  
            {
                lstrcpy(pszDevicePath, szDevName);//复制驱动器  
                lstrcat(pszDevicePath, pszNtPath + 2);//复制路径  

                return TRUE;
            }
        }
    }

    lstrcpy(pszDevicePath, pszNtPath);

    return FALSE;
}
