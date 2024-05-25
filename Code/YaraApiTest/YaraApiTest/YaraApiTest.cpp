#include "YaraApiTest.h"
#include "yara.h"

#pragma comment(lib, "libyara64-MD.lib")


void print_rule_identifier(YR_RULES* compiledRules)
{
    YR_RULE* rule;
    yr_rules_foreach(compiledRules, rule)
    {
        OUTINFO_1_PARAM("规则标识符: %s\n", rule->identifier);;
    }

}

// 初始化 YARA 扫描器
int InitializeYaraScanner(char* rulesFilePath, void** retcompiler, void** retcompiledRules)
{
    int result;
    FILE* ruleFile = NULL;
    void*  compiler = NULL;
    void* compiledRules = NULL;

    if (yr_initialize() != ERROR_SUCCESS)
    {
        return 1; // 初始化失败
    }

    result = yr_compiler_create((YR_COMPILER**)&compiler);
    if (result != ERROR_SUCCESS)
    {
        OUTINFO_0_PARAM("创建编译器失败!\n");
        return 1;// 创建编译器失败
    }

    fopen_s(&ruleFile, rulesFilePath, "r");
    if (ruleFile == NULL) {
        OUTINFO_0_PARAM("打开规则文件失败!\n");
        return 1; // 打开规则文件失败
    }

    result = yr_compiler_add_file((YR_COMPILER *)compiler, ruleFile, NULL, NULL);
    fclose(ruleFile); //关闭打开的文件
    if (result != ERROR_SUCCESS)
    {
        OUTINFO_0_PARAM("添加规则文件失败!\n");
        return 1; // 添加规则文件失败
    }

    result = yr_compiler_get_rules((YR_COMPILER*)compiler, (YR_RULES**)&compiledRules);
    if (result != ERROR_SUCCESS)
    {
        OUTINFO_0_PARAM("编译规则失败!\n");
        return 1; // 编译规则失败
    }

    *retcompiler = compiler;
    *retcompiledRules = compiledRules;

    //print_rule_identifier((YR_RULES*)compiledRules);

    OUTINFO_0_PARAM("YARA扫描器初始化完成!\n");

    return 0;  //初始化成功
}

// 清理 YARA 扫描器
void CleanupYaraScanner(void* compiler, void* compiledRules)
{
    if (compiledRules != NULL)
    {
        yr_rules_destroy((YR_RULES*)compiledRules);
    }
    if (compiler != NULL)
    {
        yr_compiler_destroy((YR_COMPILER*)compiler);
    }
    yr_finalize();

    OUTINFO_0_PARAM("清理资源完成!\n");
}


INT my_callback(
    void* context,
    int message,
    void* message_data,
    void* user_data)
{

    //OUTINFO_0_PARAM("MSG: %d\n", message);

    switch (message)
    {
        case CALLBACK_MSG_RULE_MATCHING:
        {
            //匹配该特征规则，打印匹配信息后，结束本次扫描。
            OUTINFO_2_PARAM("文件名%s 匹配到规则:%s !!!\n", (char*)(user_data), ((YR_RULE*)message_data)->identifier);
            return CALLBACK_ABORT;
        }

        case CALLBACK_MSG_RULE_NOT_MATCHING:
        {
            //匹配该特征规则，打印匹配信息后，结束本次扫描。
            //OUTINFO_1_PARAM("文件名:%s 没有匹配到规则!\n", (char*)(user_data));
            break;
        }

        case CALLBACK_MSG_SCAN_FINISHED:
        {
            //扫描完成，没有匹配任何特征规则。
            OUTINFO_1_PARAM("文件%s 没有匹配任何特征规则!\n", (char*)(user_data));
            break;
        }
        
        default:
            break;
    }
 
    //没有匹配该特征规则，继续匹配下一条特征规则。
    return CALLBACK_CONTINUE;
}



// 执行文件扫描
int PerformFileScan(char* targetfile, yara_callback filescan_callback,  void* compiledRules)
{
    int result;
    //OUTINFO_0_PARAM("开始扫描!\n");
    
    try
    {
        if (compiledRules != NULL)
        {
            result = yr_rules_scan_file((YR_RULES*)compiledRules, (const char*)targetfile, SCAN_FLAGS_FAST_MODE, (YR_CALLBACK_FUNC)filescan_callback, (char*)targetfile, 0);
            if (result != ERROR_SUCCESS)
            {
                OUTINFO_0_PARAM("扫描中出错!\n");
                return 1;
            }
        }
        else
        {
            OUTINFO_0_PARAM("compiledRules为NULL!\n");
        }
    }
    catch (exception& e)
    {
        OUTINFO_1_PARAM("扫描中出错: %s !\n", e.what());
    }
    return 0;
 }



// 执行文件扫描
int PerformBufferScan(uint8_t* targetbuffer, size_t buffersize, yara_callback filescan_callback, void* compiledRules, void* result_flag)
{
    int result;
    //OUTINFO_0_PARAM("开始扫描!\n");

    try
    {
        if (compiledRules != NULL)
        {
            result = yr_rules_scan_mem((YR_RULES*)compiledRules, (const uint8_t*)targetbuffer, buffersize,  SCAN_FLAGS_FAST_MODE, (YR_CALLBACK_FUNC)filescan_callback, result_flag, 0);
            if (result != ERROR_SUCCESS)
            {
                OUTINFO_0_PARAM("扫描中出错!\n");
                return 1;
            }
        }
        else
        {
            OUTINFO_0_PARAM("compiledRules为NULL!\n");
        }
    }
    catch (exception& e)
    {
        OUTINFO_1_PARAM("扫描中出错: %s !\n", e.what());
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
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