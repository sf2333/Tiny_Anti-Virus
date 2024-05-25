#include <iostream>
#include <stdio.h>
#include <exception>  
using namespace std;

//定义回调函数指针
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


//打印debug信息
//OUTINFO_0_PARAM表示输出纯字符串，OUTINFO_1_PARAM表示可以携带一个参数，以此类推2、3
#define OUTINFO_0_PARAM(fmt) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt));OutputDebugStringA(sOut);}    
#define OUTINFO_1_PARAM(fmt,var) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt),var);OutputDebugStringA(sOut);}    
#define OUTINFO_2_PARAM(fmt,var1,var2) {CHAR sOut[256];CHAR sfmt[100];sprintf_s(sfmt,"%s%s","INFO--",fmt);sprintf_s(sOut,(sfmt),var1,var2);OutputDebugStringA(sOut);}    


