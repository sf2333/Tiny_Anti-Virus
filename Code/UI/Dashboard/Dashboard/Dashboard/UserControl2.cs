using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Security.Cryptography;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;

namespace Dashboard
{

    public partial class UserControl2 : UserControl
    {
        private CancellationTokenSource cancelTokenSource1;
        private Button CurBtn;
        private int virus_count;
        private List<string> virus_path_List = new List<string>();

        public UserControl2()
        {
            InitializeComponent();

            GlobalVariables.IsScanner_Runing = false;
        }

        private void UserControl2_Load(object sender, EventArgs e)
        {
            //一开始隐藏停止按钮
            btnStop.Visible = false;
        }

        //异常打印
        private static void LogException(Exception e, string title)
        {
            System.Diagnostics.Trace.WriteLine(string.Format("[R3]{0}: {1}", title, e.Message));
            System.Diagnostics.Trace.WriteLine(string.Format("[R3]异常类型: {0}", e.GetType()));
            System.Diagnostics.Trace.WriteLine(string.Format("[R3]堆栈跟踪: {0}", e.StackTrace));
            System.Diagnostics.Trace.WriteLine(string.Format("[R3]异常来源: {0}", e.Source));
            System.Diagnostics.Trace.WriteLine(string.Format("[R3]内部异常: {0}", e.InnerException));
        }

        //报错扫描日志
        public static void SaveScanLog(string signature)
        {
            FileStream fs = new FileStream(@"C:\Users\awqhc\Desktop\myui\SCAN_LOG.txt", FileMode.Append);
            //获得字节数组
            string curtime = DateTime.Now.ToString(@"hh\:mm\:ss");
            string logline = "LOG-" + curtime + "-" +  signature + Environment.NewLine;
            byte[] data = System.Text.Encoding.Default.GetBytes(logline);
            //开始写入
            fs.Write(data, 0, data.Length);
            //清空缓冲区、关闭流
            fs.Flush();
            fs.Close();
        }

        //规范输出文件名
        private string GetOutFileName(string filePath)
        {
            const int maxLength = 70;

            string fileName = Path.GetFileName(filePath);
            string directoryPath = Path.GetDirectoryName(filePath);

            if (filePath.Length > maxLength)
            {
                // 文件路径过长，需要截断
                if(fileName.Length > maxLength - 30)
                {
                    fileName = fileName.Substring(0, 20) + "..." + fileName.Substring(fileName.Length - 10);
                }
                string truncatedFilePath = directoryPath.Substring(0, Math.Min(directoryPath.Length, 30)) + "...\\" + fileName;
                return truncatedFilePath;
            }
            else
            {
                // 文件名长度在限制范围内，直接显示完整文件名
                return filePath;
            }
        }

        // 检验文件签名
        private bool IsHasSign(string filePath)
        {

            try
            {
                // 创建X509Certificate2对象,若报错则说明不具有数字签名
                X509Certificate2 certificate = new X509Certificate2(filePath);
                //richTextBox1.AppendText(filePath + "具有数字签名" + Environment.NewLine);
                return true;
            }
            catch (Exception ex)
            {
                //richTextBox1.AppendText(filePath + "没有数字签名" + Environment.NewLine);
                return false;
            }
        }

        //弃用的扫描程序
        private void Old_ProcessFiles(string path, DateTime startTime, ref int file_count, ref int virus_count)
        {
            string virus_type = null;

            
            Stack<string> stack = new Stack<string>();
            stack.Push(path);

            while (stack.Count > 0)
            {
                string currentDir = stack.Pop();

                try
                {
                    string[] files = Directory.GetFiles(currentDir, "*.*").Where(s => s.EndsWith(".exe") || s.EndsWith(".dll") || s.EndsWith(".sys")).ToArray();

                    // 处理当前目录下的文件
                    foreach (string file in files)
                    {
                        //判断是否取消任务
                        if (cancelTokenSource1.IsCancellationRequested)
                        {
                            System.Diagnostics.Trace.WriteLine("[R3]在扫描病毒时,提前结束Scanner线程成功...");
                            return;
                        }

                        //显示当前处理的路径
                        string curfile = GetOutFileName(file);
                        SDirOut.Text = curfile;

                        try
                        {
                            //获取文件md5
                            string mycode = GetMD5ByHashAlgorithm(file);

                            //匹配病毒特征
                            virus_type = VirusSignatureMatcher.MatchSignature(mycode);
                            if (virus_type != null)
                            {
                                string outline = file + "是病毒：" + virus_type;
                                richTextBox1.Text += outline + Environment.NewLine;
                                SaveScanLog(outline);
                                virus_count++;
                            }

                    
                        }
                        catch
                        {
                            // 任何错误跳过，如无权限、文件正在被使用等等
                            System.Diagnostics.Trace.WriteLine(string.Format("[R3]Scanner遇到文件-{0} 无法打开", file));

                            file_count++; //文件数量加1
                            DateTime endTime2 = DateTime.Now; // 获取结束时间
                            TimeSpan duration2 = endTime2.Subtract(startTime); // 计算时间间隔
                            SNumOut.Text = "" + file_count;
                            STimeOut.Text = duration2.ToString(@"hh\:mm\:ss");
                            continue;
                        }

                        file_count++; //文件数量加1
                        DateTime endTime = DateTime.Now; // 获取结束时间
                        TimeSpan duration = endTime.Subtract(startTime); // 计算时间间隔
                        SNumOut.Text = "" + file_count;
                        STimeOut.Text = duration.ToString(@"hh\:mm\:ss");
                    }


                    // 获取当前目录下的子目录
                    string[] dirs = Directory.GetDirectories(currentDir);
                    // 将子目录入栈，等待处理
                    foreach (string dir in dirs)
                    {
                        stack.Push(dir);

                    }
                }
                catch (IOException e)
                {
                    LogException(e, "[R3]在ProcessFiles运行时发生异常1");
                    continue;
                }
                catch (UnauthorizedAccessException e)
                {
                    LogException(e, "[R3]在ProcessFiles运行时发生异常2");
                    continue;
                }

            }
        }

        //定义结构体
        [StructLayout(LayoutKind.Sequential)]
        public struct YR_RULE
        {
            public int flags;
            public int num_atoms;
            public uint required_strings;
            public uint unused;

            public IntPtr identifier;
            public IntPtr tags;
            public IntPtr metas;
            public IntPtr strings;
            public IntPtr ns;
        }
        public int FileScanCallback(IntPtr context, int message, IntPtr messageData, IntPtr userData)
        {
            //System.Diagnostics.Trace.WriteLine("MSG: " + message + "\n");
            string fileName = Marshal.PtrToStringAnsi(userData);


            switch (message)
            {
                case 1: //CALLBACK_MSG_RULE_MATCHING
                    {
                        //匹配该特征规则，打印匹配信息后，结束本次扫描。
                        if (messageData != IntPtr.Zero)
                        {
                            YR_RULE rule = Marshal.PtrToStructure<YR_RULE>(messageData);
                            string identifer_str = Marshal.PtrToStringAnsi(rule.identifier);
                            string print_str = $"{fileName}中发现病毒:{identifer_str}!\n";
                            richTextBox1.AppendText(print_str);
                            virus_path_List.Add(fileName);
                            virus_count++;
                        }
                        else
                        {
                            System.Diagnostics.Trace.WriteLine("错误啦!messageData为空!");
                        }

                        return 1; //CALLBACK_ABORT
                    }

                case 2: //
                    {
                        //匹配该特征规则，打印匹配信息后，结束本次扫描。
                        break;
                    }

                case 3: //CALLBACK_MSG_SCAN_FINISHED
                    {
                        //扫描完成，没有匹配任何特征规则。
                        System.Diagnostics.Trace.WriteLine($"文件名{fileName} 没有匹配任何特征规则!");
                        break;
                    }

                default:
                    break;
            }

            //没有匹配该特征规则，继续匹配下一条特征规则。
            return 0; //CALLBACK_CONTINUE
        }

        private void UsingYara(string path, DateTime startTime, ref int file_count)
        {
            //string rulesFilePath = "clamav-ndb.yara";
            string rulesFilePath = "clamav-db.yara";
            IntPtr compiler = IntPtr.Zero;
            IntPtr compiledRules = IntPtr.Zero;
            int ret;

            // 初始化扫描器
            ret = YaraApi.InitializeYaraScanner(rulesFilePath, out compiler, out compiledRules);
            if(ret == 1)
            {
                System.Diagnostics.Trace.WriteLine("[病毒扫描]YARA扫描器初始化失败...");
                return;
            }

            System.Diagnostics.Trace.WriteLine("[病毒扫描]YARA扫描器初始化成功...");

            //创建回调函数实例
            //YaraApi.FileScanCallbackDelegate mycall;
            //mycall = new YaraApi.FileScanCallbackDelegate(YaraApi.FileScanCallback);


            Stack<string> stack = new Stack<string>();
            stack.Push(path);

            while (stack.Count > 0)
            {
                string currentDir = stack.Pop();

                try
                {
                    //只扫描可执行文件
                    string[] files = Directory.GetFiles(currentDir, "*.*").Where(s => s.EndsWith(".exe") || s.EndsWith(".dll") || s.EndsWith(".sys")).ToArray();
                    //string[] files = Directory.GetFiles(currentDir, "*.*").ToArray();

                    // 处理当前目录下的文件
                    foreach (string file in files)
                    {
                        //判断是否取消任务
                        if (cancelTokenSource1.IsCancellationRequested)
                        {
                            System.Diagnostics.Trace.WriteLine("[R3]在扫描病毒时,提前结束Scanner线程成功...");
                            //关闭扫描器
                            YaraApi.CleanupYaraScanner(compiler, compiledRules);
                            System.Diagnostics.Trace.WriteLine("[R3]YARA扫描器关闭...");
                            return;
                        }

                        //显示当前处理的路径
                        string curfile = GetOutFileName(file);
                        SDirOut.Text = curfile;

                        //如果不具有数字签名就进行扫描
                        if(!IsHasSign(curfile))
                        {
                            try
                            {
                                // 声明委托实例并初始化
                                YaraApi.FileScanCallbackDelegate callback = new YaraApi.FileScanCallbackDelegate(FileScanCallback);

                                // 调用 PerformFileScan 函数
                                int result = YaraApi.PerformFileScan(file, callback, compiledRules);
                            }
                            catch (Exception ex)
                            {
                                richTextBox1.Invoke((MethodInvoker)(() =>
                                {
                                    richTextBox1.AppendText("调用YARA API时发生错误：" + ex.Message + Environment.NewLine);
                                }));

                                file_count++; //文件数量加1
                                DateTime endTime2 = DateTime.Now; // 获取结束时间
                                TimeSpan duration2 = endTime2.Subtract(startTime); // 计算时间间隔
                                SNumOut.Text = "" + file_count;
                                STimeOut.Text = duration2.ToString(@"hh\:mm\:ss");
                                continue;
                            }
                        }

                        file_count++; //文件数量加1
                        DateTime endTime3 = DateTime.Now; // 获取结束时间
                        TimeSpan duration3 = endTime3.Subtract(startTime); // 计算时间间隔
                        SNumOut.Text = "" + file_count;
                        STimeOut.Text = duration3.ToString(@"hh\:mm\:ss");
                    }

                    // 获取当前目录下的子目录
                    string[] dirs = Directory.GetDirectories(currentDir);
                    // 将子目录入栈，等待处理
                    foreach (string dir in dirs)
                    {
                        stack.Push(dir);

                    }
                }
                catch (IOException e)
                {
                    LogException(e, "[R3]在ProcessFiles运行时发生异常1");
                    continue;
                }
                catch (UnauthorizedAccessException e)
                {
                    LogException(e, "[R3]在ProcessFiles运行时发生异常2");
                    continue;
                }  

            }

            //关闭扫描器
            YaraApi.CleanupYaraScanner(compiler, compiledRules);
            System.Diagnostics.Trace.WriteLine("[R3]YARA扫描器关闭...");

        }

        //扫描主线程程序
        public int ScanTask(string[] folderPath)
        {

            //初始化参数
            int file_count = 0;
            virus_count = 0;
            virus_path_List.Clear();

            DateTime startTime = DateTime.Now; // 获取开始时间
                                              
            System.Diagnostics.Trace.WriteLine("[R3]开始执行Scanner线程...");
            try
            {

                //可能是全盘扫描需要遍历盘
                foreach(string folder in folderPath)
                {
                    if (cancelTokenSource1.IsCancellationRequested)
                    {
                        break;
                    }
                    // 遍历文件夹，搜索指定文件类型并进行操作
                    UsingYara(folder, startTime, ref file_count);
                    //ProcessFiles(folder, startTime, ref file_count, ref virus_count);
                }

               
            }

            catch (System.SystemException e)
            {
                LogException(e, "[R3]Scanner主线程运行时发生异常!");

                btnStop.Visible = false;
                GlobalVariables.IsScanner_Runing = false;
                return 1;
            }

            //输出结果
            richTextBox1.Text += "本次扫描共扫描" + file_count + "个文件" + "其中发现疑似病毒的文件" + virus_count + "个!\n";
            string ScanReport = "本次扫描共扫描" + file_count + "个文件\n" + "其中发现疑似病毒的文件" + virus_count + "个!\n";

            if(virus_count > 0)
            {
                DialogResult result = MessageBox.Show(ScanReport + "\n是否清除所有疑似病毒的文件？", "扫描结果", MessageBoxButtons.YesNo);
                //连接病毒清除模块
                if (result == DialogResult.Yes)
                {
                    // 用户选择了“是”
                    richTextBox1.AppendText("\n病毒文件清除情况:\n");
                    foreach (string virus_path in virus_path_List)
                    {

                        bool ret = UserControl3.CleanFile(virus_path);
                        if (ret)
                        {
                            richTextBox1.AppendText("成功删除病毒文件：" + virus_path + "\n");
                        }
                        else
                        {
                            richTextBox1.AppendText("失败删除病毒文件：" + virus_path + "\n");
                        }
                    }
                }
            }
            else
            {
                MessageBox.Show(ScanReport, "扫描结果", MessageBoxButtons.OK);
            }

            //设置状态
            CurBtn.BackColor = Color.FromName("CornflowerBlue"); //恢复当前按钮颜色
            btnStop.Visible = false;
            GlobalVariables.IsScanner_Runing = false;
            System.Diagnostics.Trace.WriteLine("[R3]正常结束Scanner线程...");
            return 0;
        }

        //自定义扫描
        private void btnDiy_Click(object sender, EventArgs e)
        {

            if (!GlobalVariables.IsScanner_Runing)
            {
                //清空输出区
                
                STimeOut.Text = "";
                SNumOut.Text = "";
                richTextBox1.Text = "";
                SDirOut.Text = "未开始扫描";

                System.Windows.Forms.FolderBrowserDialog folderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
                DialogResult result = folderBrowserDialog.ShowDialog();

                if (result == System.Windows.Forms.DialogResult.OK)
                {
                    string[] RootPaths = new string[] {folderBrowserDialog.SelectedPath};
                    SDirOut.Text = RootPaths[0];

                    // 清除之前的任务（如果存在）
                    if (cancelTokenSource1 != null)
                    {
                        cancelTokenSource1.Dispose();
                    }
                    // 创建新的 CancellationTokenSource
                    cancelTokenSource1 = new CancellationTokenSource();

                    //新建一个Task来运行R0消息接收器
                    int ret;
                    Task.Run(() =>
                    {
                        ret = ScanTask(RootPaths);
                        // 处理 result 的逻辑
                    }, cancelTokenSource1.Token);

                    GlobalVariables.IsScanner_Runing = true;
                    btnStop.Visible = true;
                    //加深颜色
                    CurBtn = btnDiy;
                    CurBtn.BackColor = Color.FromName("RoyalBlue");
                }
            }


        }

        //快速扫描 - 扫描System32目录，应用、下载、文档、启动目录
        private void btnRapid_Click(object sender, EventArgs e)
        {
            if (!GlobalVariables.IsScanner_Runing)
            {
                //清空输出区
                STimeOut.Text = "";
                SNumOut.Text = "";
                richTextBox1.Text = "";

                string systemPath = @"C:\Windows\System32";
                string appX64Path = @"C:\Program Files";
                string appX86Path = @"C:\Program Files (x86)";
                string userPath = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
                string documentsPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
                string downloadsPath = Path.Combine(userPath, "Downloads");
                string desktopPath = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
                string startupPath = Environment.GetFolderPath(Environment.SpecialFolder.Startup);
                
                string[] RootPaths = new string[] { systemPath, appX64Path, appX86Path, downloadsPath, startupPath, documentsPath  };
                SDirOut.Text = "正在扫描易被感染文件夹...";

                // 清除之前的任务（如果存在）
                if (cancelTokenSource1 != null)
                {
                    cancelTokenSource1.Dispose();
                }
                // 创建新的 CancellationTokenSource
                cancelTokenSource1 = new CancellationTokenSource();

                //新建一个Task来运行R0消息接收器
                int ret;
                Task.Run(() =>
                {
                    ret = ScanTask(RootPaths);
                    // 处理 result 的逻辑
                }, cancelTokenSource1.Token);

                GlobalVariables.IsScanner_Runing = true;
                btnStop.Visible = true;
                //加深颜色
                CurBtn = btnRapid;
                CurBtn.BackColor = Color.FromName("RoyalBlue");
            }
        }

        //全盘扫描
        private void btnAll_Click(object sender, EventArgs e)
        {

            if (!GlobalVariables.IsScanner_Runing)
            {
                //清空输出区
                STimeOut.Text = "";
                SNumOut.Text = "";
                richTextBox1.Text = "";

                List<string> stringList = new List<string>();
                DriveInfo[] driveInfo = DriveInfo.GetDrives();
                foreach (DriveInfo drive in driveInfo)
                {
                    if (drive.IsReady)
                    {
                        stringList.Add(drive.Name);
                        //richTextBox1.Text += drive.Name + Environment.NewLine;
                    }
                }

                string[] RootPaths = stringList.ToArray();
                SDirOut.Text = "正在进行全盘扫描...";

                // 清除之前的任务（如果存在）
                if (cancelTokenSource1 != null)
                {
                    cancelTokenSource1.Dispose();
                }
                // 创建新的 CancellationTokenSource
                cancelTokenSource1 = new CancellationTokenSource();

                //新建一个Task来运行R0消息接收器
                int ret;
                Task.Run(() =>
                {
                    ret = ScanTask(RootPaths);
                    // 处理 result 的逻辑
                }, cancelTokenSource1.Token);

                GlobalVariables.IsScanner_Runing = true;
                btnStop.Visible = true;
                //加深颜色
                CurBtn = btnAll;
                CurBtn.BackColor = Color.FromName("RoyalBlue");

            }
        }
        
        //扫描终止按钮
        private void btnStop_Click(object sender, EventArgs e)
        {
            cancelTokenSource1.Cancel();
            GlobalVariables.IsScanner_Runing = false;

        }

        //获取文件hash串
        public static string GetMD5ByHashAlgorithm(string path)
        {
            if (!File.Exists(path)) return "";

            // 获取文件大小（字节）
            FileInfo fileInfo = new FileInfo(path);
            long fileSize = fileInfo.Length;

            int bufferSize = 1024 * 16;//自定义缓冲区大小16K            
            byte[] buffer = new byte[bufferSize];
            Stream inputStream = File.Open(path, FileMode.Open, FileAccess.Read, FileShare.Read);
            HashAlgorithm hashAlgorithm = new MD5CryptoServiceProvider();
            int readLength = 0;//每次读取长度            
            var output = new byte[bufferSize];
            while ((readLength = inputStream.Read(buffer, 0, buffer.Length)) > 0)
            {
                //计算MD5                
                hashAlgorithm.TransformBlock(buffer, 0, readLength, output, 0);
            }
            //完成最后计算，必须调用(由于上一部循环已经完成所有运算，所以调用此方法时后面的两个参数都为0)            		  
            hashAlgorithm.TransformFinalBlock(buffer, 0, 0);
            string md5 = BitConverter.ToString(hashAlgorithm.Hash).Replace("-", "").ToLower();

            hashAlgorithm.Clear();
            inputStream.Close();
            string result = md5 + ":" + fileSize;
            return result;
        }

    }


    public class VirusSignatureMatcher
    {
        private static Dictionary<string, string> virusSignatures;

        public static void Initialize()
        {
            virusSignatures = new Dictionary<string, string>();

            string dbpath = @"C:\Users\awqhc\Desktop\myui\main.hdb"; //病毒md5特征库路径

            if (File.Exists(dbpath))
            {
                StreamReader sr = new StreamReader(dbpath, Encoding.Default);
                string line;

                while ((line = sr.ReadLine()) != null)
                {
                    string[] sArray = line.Split(':'); // 一定是单引 号
                                                       // sArray[0] 病毒md5 sArray[1] 病毒字节数 sArray[2]病毒名称
                    virusSignatures[sArray[0] + ":" + sArray[1]] = sArray[2];
                }

                sr.Close();
            }
            else
            {
                // 处理文件不存在的情况
                throw new FileNotFoundException("病毒数据库不存在!!!");
            }
        }

        public static string MatchSignature(string fHash)
        {

            if (virusSignatures == null)
            {
                // 未初始化，抛出异常或执行其他逻辑
                throw new InvalidOperationException("病毒数据库未初始化！");
            }

            
            string virus = null;
            string[] spinput = fHash.Split(':');

            if (spinput.Length < 2)
            {
                // 处理数组越界的情况，例如抛出异常或执行其他逻辑
                //throw new ArgumentException("fHash 格式不正确！");
                System.Diagnostics.Trace.WriteLine("fHash 格式不正确");
                return null;
            }

            string fMd5 = spinput[0];
            string fSize = spinput[1];

            if (virusSignatures.TryGetValue(fMd5 + ":" + fSize, out virus))
            {
                return virus;
            }

            return null;
        }
    }


}
