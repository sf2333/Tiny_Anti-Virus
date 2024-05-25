#include <iostream>
#include <stdio.h>
#include <exception>  
using namespace std;

//����ص�����ָ��
typedef  int(*yara_callback)(
    void* context,
    int message,
    void* message_data,
    void* user_data);

#define ExportMyDLL extern "C" __declspec(dllexport)

ExportMyDLL int InitializeYaraScanner(char* rulesFilePath, void** retcompiler, void** retcompiledRules);
ExportMyDLL void CleanupYaraScanner(void* compiler, void* compiledRules);
ExportMyDLL int PerformFileScan(char* targetData, yara_callback callback, void* compiledRules);
ExportMyDLL int PerformBufferScan(uint8_t* targetbuffer, size_t buffersize, yara_callback filescan_callback, void* compiledRules, void* result_flag);


//��ӡdebug��Ϣ
//OUTINFO_0_PARAM��ʾ������ַ�����OUTINFO_1_PARAM��ʾ����Я��һ���������Դ�����2��3
#define OUTINFO_0_PARAM(fmt) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt));OutputDebugStringA(sOut);}    
#define OUTINFO_1_PARAM(fmt,var) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt),var);OutputDebugStringA(sOut);}    
#define OUTINFO_2_PARAM(fmt,var1,var2) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt),var1,var2);OutputDebugStringA(sOut);}    


