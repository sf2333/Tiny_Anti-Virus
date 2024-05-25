using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace Dashboard
{
    class DLL
    {

    }

    public class SysDLL
    {
        /*
        ExportMyDLL BOOL InstallMinifilter(byte[] m_pSysPath, byte[] m_pServiceName, byte[] m_pDisplayName, byte[] m_pMiniAltitude);
        ExportMyDLL BOOL InstallProcessBlock(byte[] m_pSysPath, byte[] m_pServiceName, byte[] m_pDisplayName);
        ExportMyDLL BOOL Start(byte[] m_pServiceName);
        ExportMyDLL BOOL Stop(byte[] m_pServiceName);
        ExportMyDLL BOOL Remove(byte[] m_pServiceName);
        */

        //与dll中一致 
        [DllImport("sysinstaller.dll", EntryPoint = "InstallMinifilter")]
        public extern static bool InstallMinifilter(byte[] m_pSysPath, byte[] m_pServiceName, byte[] m_pDisplayName, byte[] m_pMiniAltitude);

        [DllImport("sysinstaller.dll", EntryPoint = "InstallDriver")]
        public extern static bool InstallDriver(byte[] m_pSysPath, byte[] m_pServiceName, byte[] m_pDisplayName);

        [DllImport("sysinstaller.dll", EntryPoint = "Start")]
        public extern static bool Start(byte[] m_pServiceName);

        [DllImport("sysinstaller.dll", EntryPoint = "Stop")]
        public extern static bool Stop(byte[] m_pServiceName);

        [DllImport("sysinstaller.dll", EntryPoint = "Remove")]
        public extern static bool Remove(byte[] m_pServiceName);


    }
    
    public class AdUserDLL
    {
        /*
        ExportDLL int ScanMain(int request_nums, int thread_nums, PVOID compiledRules);
        ExportDLL BOOL EndConnect();
        ExportDLL BOOL SendFileName(char* file_path);
        */

        //与dll中一致 
        [DllImport("AdUserDLL.dll", EntryPoint = "ScanMain")]
        public extern static int ScanMain(int request_nums, int thread_nums, IntPtr compiledRules);

        [DllImport("AdUserDLL.dll", EntryPoint = "EndConnect")]
        public extern static bool EndConnect();

        [DllImport("AdUserDLL.dll", EntryPoint = "SendFileName")]
        public extern static bool SendFileName(string fn);  //c++中 char对应c# string

    }

    public class YaraApi
    {
        /*
        ExportMyDLL void* InitializeYaraScanner(char* rulesFilePath, void* compiler, void* compiledRules);
        ExportMyDLL void CleanupYaraScanner(void* compiler, void* compiledRules);
        ExportMyDLL int PerformFileScan(char* targetData, yara_callback callback, void* compiledRules);
        */

        //与dll中一致 
        [DllImport("YaraApiTest.dll", EntryPoint = "InitializeYaraScanner", CallingConvention = CallingConvention.Cdecl)]
        public extern static int InitializeYaraScanner(string rulesFilePath, out IntPtr compiler, out IntPtr compiledRules); //c++中 char对应c# string； 需要添加ref传回调用端

        [DllImport("YaraApiTest.dll", EntryPoint = "CleanupYaraScanner")]
        public extern static void CleanupYaraScanner(IntPtr compiler, IntPtr compiledRules);

        //在c#中定义回调函数传入dll
        /*
        typedef int (* yara_callback) (
          YR_SCAN_CONTEXT* context,
          int message,
          void* message_data,
          void* user_data);
        */

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        //public delegate int FileScanCallbackDelegate(int message);
        public delegate int FileScanCallbackDelegate(IntPtr context, int message, IntPtr messageData, IntPtr userData);

        [DllImport("YaraApiTest.dll", EntryPoint = "PerformFileScan", SetLastError = true, CharSet = CharSet.Ansi, ExactSpelling = false, CallingConvention = CallingConvention.Cdecl)]
        public extern static int PerformFileScan(string targetData, FileScanCallbackDelegate callback, IntPtr compiler);

    }


}
