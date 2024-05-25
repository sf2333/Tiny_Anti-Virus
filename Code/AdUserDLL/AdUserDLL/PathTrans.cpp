#include "PathTrans.h"
#include "ulity.h"


BOOL Char2Wchar(char* mbString, wchar_t** wideStringPtr) //����wideString��ָ���ַ
{
    int wideLength = MultiByteToWideChar(CP_ACP, 0, mbString, -1, NULL, 0);
    if (wideLength == 0) {
        OUTINFO_0_PARAM("ת��ʧ��1\n");
        return FALSE;
    }
    wchar_t* wideString = (wchar_t*)malloc(wideLength * sizeof(wchar_t));
    if (wideString == NULL) {
        OUTINFO_0_PARAM("�ڴ����ʧ��\n");
        return FALSE;
    }
    if (MultiByteToWideChar(CP_ACP, 0, mbString, -1, wideString, wideLength) == 0) {
        OUTINFO_0_PARAM("ת��ʧ��2\n");
        free(wideString);
        return FALSE;
    }
    //OUTINFO_0_PARAM("����·��(���ַ��ַ���)��%ls\n", wideString);
    *wideStringPtr = wideString;  // ��������ڴ��ַ��ֵ�������ָ��
    return TRUE;
}

BOOL NtPathToDevicePath(wchar_t* pszNtPath, wchar_t* pszDevicePath)
{
    static TCHAR    szDriveStr[MAX_PATH] = { 0 };
    static TCHAR    szDevName[MAX_PATH] = { 0 };
    TCHAR            szDrive[3];
    INT             cchDevName;
    INT             i;

    //������  
    if (IsBadWritePtr(pszNtPath, 1) != 0)return FALSE;
    if (IsBadReadPtr(pszDevicePath, 1) != 0)return FALSE;


    //��ȡ���ش����ַ���  
    ZeroMemory(szDriveStr, ARRAYSIZE(szDriveStr));
    ZeroMemory(szDevName, ARRAYSIZE(szDevName));

    if (GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))
    {
        for (i = 0; szDriveStr[i]; i += 4)
        {
            //����A��B��
            if (!lstrcmpi(&(szDriveStr[i]), _T("A:\\")) || !lstrcmpi(&(szDriveStr[i]), _T("B:\\")))
                continue;

            //�����̷� ����Ϊ2 ��ʽΪC:
            szDrive[0] = szDriveStr[i];
            szDrive[1] = szDriveStr[i + 1];
            szDrive[2] = '\0';

            //��ѯ�̷���Ӧ��Dos�豸��  ��ʽΪ\Device\HarddiskVolume1
            if (!QueryDosDevice(szDrive, szDevName, MAX_PATH))
            {
                OUTINFO_0_PARAM("��ѯDos�豸��ʧ��!");
                return FALSE;
            }

            //ȡDos�豸����
            cchDevName = lstrlen(szDevName);
            //OUTINFO_0_PARAM("%ls %ls %d\n", szDrive, szDevName, cchDevName);

            if (_tcsnicmp(pszNtPath, szDrive, 2) == 0)//����  
            {
                lstrcpy(pszDevicePath, szDevName);//����������  
                lstrcat(pszDevicePath, pszNtPath + 2);//����·��  

                return TRUE;
            }
        }
    }

    lstrcpy(pszDevicePath, pszNtPath);

    return FALSE;
}
