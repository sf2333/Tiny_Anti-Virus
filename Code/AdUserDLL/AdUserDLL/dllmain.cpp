// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <windows.h>
#include <winioctl.h>
#include <winsvc.h>
#include <conio.h>
#include <time.h>
#include <Psapi.h>
#include <locale.h>
#include <FltUser.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <assert.h>
#include <dontuse.h>

#include "scanuk.h"
#include "scanuser.h"
#include "ulity.h"
#include "PathTrans.h"

#include "YaraApiTest.h"

#pragma comment(lib, "fltLib.lib")
#pragma comment(lib, "YaraApiTest.lib")




//导出函数
ExportDLL int ScanMain(int request_nums, int thread_nums, PVOID compiledRules);
ExportDLL BOOL EndConnect();
ExportDLL BOOL SendFileName(char* file_path);

//结构体
typedef struct _Threadparms
{
    PSCANNER_THREAD_CONTEXT pcontext;
    PVOID Yara_Rules;
}Threadparms;

//全局变量
SCANNER_THREAD_CONTEXT context;
int index = 0;

//
//  Default and Maximum number of threads.
//

#define SCANNER_DEFAULT_REQUEST_COUNT       5
#define SCANNER_DEFAULT_THREAD_COUNT        2
#define SCANNER_MAX_THREAD_COUNT            64


//
//  Context passed to worker threads
//


int PopupOnCorner() {

    return 0;
}



int my_callback(
    void* context,
    int message,
    void* message_data,
    void* res_flag)
{

    //OUTINFO_1_PARAM("old_res_flag: %d\n", *(int *)res_flag);

    switch (message)
    {
        case 1:
        {
            //匹配该特征规则，打印匹配信息后，结束本次扫描。
            OUTINFO_1_PARAM("[AD]匹配到规则:%s !!!\n", ((YR_RULE*)message_data)->identifier);
            *(int*)res_flag = TRUE;
            return 1;
        }

        case 2:
        {
            //没有匹配该特征规则
            //OUTINFO_1_PARAM("文件名:%s 没有匹配到规则!\n", (char*)(user_data));
            break;
        }

        case 3:
        {
            //扫描完成，没有匹配任何特征规则。
            OUTINFO_0_PARAM("[AD]没有匹配任何特征规则!\n");
            break;
        }

        default:
            break;
    }

    //没有匹配该特征的规则，继续匹配下一条特征规则。
    return 0;
}

BOOL
ScanBuffer(
    _In_reads_bytes_(BufferSize) PUCHAR Buffer,
    _In_ ULONG BufferSize,
    _In_ PVOID compiledRules
) 
{
    int ret;
    int matched_flag = FALSE;

    /*
    index += 1;
    char filename[50];
    sprintf(filename, "buffer_contents_%d.bin", index);
    FILE* file = fopen(filename, "wb");
    if (file) 
    {
        fwrite(Buffer, 1, BufferSize, file);
        fclose(file);
    }
    else
    {
        OUTINFO_0_PARAM("Error opening file for writing.\n");
    }
    */

    ret = PerformBufferScan(Buffer, BufferSize, my_callback, compiledRules, &matched_flag);

    //正常结束
    if (!ret)
    {
        //扫描出漏洞
        if (matched_flag)
        {
            int tid = GetCurrentThreadId();
            OUTINFO_1_PARAM("[AD]线程：%d 发现病毒特征码\n", tid);
            return TRUE;
        }
    }

    //未扫描出漏洞
    return FALSE;
}


//主要扫描函数
DWORD
ScannerWorker(
    _In_ LPVOID TP
)
/*++

Routine Description

    This is a worker thread that


Arguments

    Context  - This thread context has a pointer to the port handle we use to send/receive messages,
                and a completion port handle that was already associated with the comm. port by the caller

Return Value

    HRESULT indicating the status of thread exit.

--*/
{
    PSCANNER_NOTIFICATION notification;
    SCANNER_REPLY_MESSAGE replyMessage;
    PSCANNER_MESSAGE message;
    LPOVERLAPPED pOvlp;
    BOOL result;
    DWORD outSize;
    HRESULT hr;
    ULONG_PTR key;

    //处理传入的参数
    Threadparms* pt = (Threadparms*)TP;
    PSCANNER_THREAD_CONTEXT Context = pt->pcontext;
    PVOID compiledRules = pt->Yara_Rules;

#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant

    while (TRUE) {

#pragma warning(pop)

        //
        //  Poll for messages from the filter component to scan.
        //

        result = GetQueuedCompletionStatus(Context->Completion, &outSize, &key, &pOvlp, INFINITE);

        //
        //  Obtain the message: note that the message we sent down via FltGetMessage() may NOT be
        //  the one dequeued off the completion queue: this is solely because there are multiple
        //  threads per single port handle. Any of the FilterGetMessage() issued messages can be
        //  completed in random order - and we will just dequeue a random one.
        //

        message = CONTAINING_RECORD(pOvlp, SCANNER_MESSAGE, Ovlp);

        if (!result) {

            //
            //  An error occured.
            //

            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        OUTINFO_1_PARAM("Received message, size %Id\n", pOvlp->InternalHigh);

        notification = &message->Notification;

        assert(notification->BytesToScan <= SCANNER_READ_BUFFER_SIZE);
        _Analysis_assume_(notification->BytesToScan <= SCANNER_READ_BUFFER_SIZE);

 
        result = ScanBuffer(notification->Contents, notification->BytesToScan, compiledRules);
        if (result) {

                MessageBoxA(NULL, "发现读写病毒文件行为，进行拦截！", "主动防御警告", MB_SYSTEMMODAL); 
        }

        replyMessage.ReplyHeader.Status = 0;
        replyMessage.ReplyHeader.MessageId = message->MessageHeader.MessageId;

        //
        //  Need to invert the boolean -- result is true if found
        //  foul language, in which case SafeToOpen should be set to false.
        //

        replyMessage.Reply.SafeToOpen = !result;
        OUTINFO_1_PARAM("Replying message, SafeToOpen: %d\n", replyMessage.Reply.SafeToOpen);


        hr = FilterReplyMessage(Context->Port,
            (PFILTER_REPLY_HEADER)&replyMessage,
            sizeof(replyMessage));

        if (SUCCEEDED(hr)) {

            OUTINFO_0_PARAM("Replied message\n");

        }
        else {

            OUTINFO_1_PARAM("Scanner: Error replying message. Error = 0x%X\n", hr);
            break;
        }

        memset(&message->Ovlp, 0, sizeof(OVERLAPPED));

        hr = FilterGetMessage(Context->Port,
            &message->MessageHeader,
            FIELD_OFFSET(SCANNER_MESSAGE, Ovlp),
            &message->Ovlp);

        if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {

            break;
        }
    }

    if (!SUCCEEDED(hr)) {

        if (hr == HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)) {

            //
            //  Scanner port disconncted.
            //

            OUTINFO_0_PARAM("Scanner: Port is disconnected, probably due to scanner filter unloading.\n");

        }
        else {

            OUTINFO_1_PARAM("Scanner: Unknown error occured. Error = 0x%X\n", hr);
        }
    }

    free(message);

    return hr;
}

//在用户端对驱动发送的内容进行扫描
int 
ScanMain(
    _In_ int request_nums,
    _In_ int thread_nums,
    _In_ PVOID compiledRules
)
{
    DWORD requestCount = SCANNER_DEFAULT_REQUEST_COUNT;
    DWORD threadCount = SCANNER_DEFAULT_THREAD_COUNT;
    HANDLE threads[SCANNER_MAX_THREAD_COUNT];
    HANDLE port, completion;
    PSCANNER_MESSAGE msg;
    DWORD threadId;
    HRESULT hr;
    DWORD i, j;

    //
    //  Check how many threads and per thread requests are desired.
    //

    requestCount = request_nums;
    threadCount = thread_nums;
    if (requestCount <= 0) {

        //Usage();
        return 1;
    }
    if (threadCount <= 0 || threadCount > 64) {

        //Usage();
        return 1;
    }
    

    //
    //  Open a commuication channel to the filter
    //

    OUTINFO_0_PARAM("Scanner: Connecting to the filter ...\n");

    hr = FilterConnectCommunicationPort(ScannerPortName,
        0,
        NULL,
        0,
        NULL,
        &port);

    if (IS_ERROR(hr)) {

        OUTINFO_1_PARAM("ERROR: Connecting to filter port: 0x%08x\n", hr);
        return 2;
    }


    //
    //  Create a completion port to associate with this handle.
    //

    completion = CreateIoCompletionPort(port,
        NULL,
        0,
        threadCount);

    if (completion == NULL) {

        OUTINFO_1_PARAM("ERROR: Creating completion port: %d\n", GetLastError());
        CloseHandle(port);
        return 3;
    }

    OUTINFO_2_PARAM("Scanner: Port = 0x%p Completion = 0x%p\n", port, completion);

    context.Port = port;
    context.Completion = completion;

    //
    //  Create specified number of threads.
    //

    //设置参数
    Threadparms T1;
    T1.pcontext = &context;
    T1.Yara_Rules = compiledRules;

    for (i = 0; i < threadCount; i++) {

        threads[i] = CreateThread(NULL,
            0,
            (LPTHREAD_START_ROUTINE)ScannerWorker,
            &T1,
            0,
            &threadId);

        if (threads[i] == NULL) {

            //
            //  Couldn't create thread.
            //

            hr = GetLastError();
            OUTINFO_1_PARAM("ERROR: Couldn't create thread: %d\n", hr);
            EndConnect();
        }

        for (j = 0; j < requestCount; j++) {

            //
            //  Allocate the message.
            //

#pragma prefast(suppress:__WARNING_MEMORY_LEAK, "msg will not be leaked because it is freed in ScannerWorker")
            msg = (PSCANNER_MESSAGE)malloc(sizeof(SCANNER_MESSAGE));

            if (msg == NULL) {

                hr = ERROR_NOT_ENOUGH_MEMORY;
                EndConnect();
            }

            memset(&msg->Ovlp, 0, sizeof(OVERLAPPED));

            //
            //  Request messages from the filter driver.
            //

            hr = FilterGetMessage(port,
                &msg->MessageHeader,
                FIELD_OFFSET(SCANNER_MESSAGE, Ovlp),
                &msg->Ovlp);

            if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {

                free(msg);
                EndConnect();
            }
        }
    }

    hr = S_OK;


    WaitForMultipleObjectsEx(i, threads, TRUE, INFINITE, FALSE);

    OUTINFO_0_PARAM("Scanner: Main运行结束!!!\n");

    return (int)port;
}

//关闭主防连接
BOOL EndConnect()
{
    
    OUTINFO_0_PARAM("Scanner:  All done. by EndConnect\n");
    if (CloseHandle(context.Port)) {
        return CloseHandle(context.Completion);
    }
    else
        return FALSE;

}


//强制删除模块-发送删除文件名
BOOL SendFileName(char* file_path)
{
    
    BOOL ret = FALSE;
    int type = 0;
    wchar_t szOutPath[MAX_PATH] = { 0 };

    char* mbString = file_path; 
    //OUTINFO_1_PARAM("%s\n", mbString);

    //转换为宽字符串
    wchar_t* wideString = NULL;
    ret = Char2Wchar(mbString, &wideString);
    if (!ret)
    {
        OUTINFO_0_PARAM("Char2Wchar失败!\n");
        return ret;
    }

    //输入nt路径进行转换
    ret = NtPathToDevicePath(wideString, szOutPath);
    if (!ret)
    {
        OUTINFO_0_PARAM("NtPathToDevicePath失败!\n");
        return ret;
    }

    // 创建输入缓冲区并写入数据
    path_packet pk;
    wcscpy_s(pk.nt_path, sizeof(wchar_t) * MAX_PATH, wideString);
    wcscpy_s(pk.device_path, sizeof(wchar_t) * MAX_PATH, szOutPath);
    //OUTINFO_0_PARAM("%ls %ls\n", pk.nt_path, pk.device_path);

    // 打开驱动程序所代表的设备对象 需要符号链接win32
    HANDLE hDevice = CreateFile(L"\\??\\MyDriver", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        OUTINFO_1_PARAM("驱动打开失败:%d\n", GetLastError());
        ret = FALSE;
        return ret;
    }

    // 发送设备控制请求
    DWORD bytesReturned;
    ret = DeviceIoControl(hDevice, MY_CUSTOM_IOCTL_CODE, &pk, sizeof(pk), NULL, 0, &bytesReturned, NULL);
    if (!ret)
    {
        OUTINFO_0_PARAM("发送设备控制请求失败\n");
        CloseHandle(hDevice);
        return ret;
    }

    // 关闭设备句柄
    CloseHandle(hDevice);

    return ret;
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