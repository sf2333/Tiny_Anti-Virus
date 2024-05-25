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

namespace Dashboard
{
   
    public partial class UserControl1 : UserControl
    {
       
        private Bitmap start_img = global::Dashboard.Properties.Resources.stopping;
        private Bitmap end_img = global::Dashboard.Properties.Resources.running;
        private CancellationTokenSource cancelTokenSource1;

       
        public UserControl1()
        {
            InitializeComponent();

            //一开始未启动
            GlobalVariables.IsAD_Runing = false;
            button1.Text = "开启主动防御";
            pictureBox2.Image = start_img;
        }

        private void UserControl1_Load(object sender, EventArgs e)
        {

            
   
        }

        private void button1_Click(object sender, EventArgs e)
        {
            bool FS_status = false, KC_status = false;
            //主动防御模块 需要启动 FileScanner+KernelCallBacks
            if (!GlobalVariables.IsAD_Runing) 
            {
                //启动FileScanner驱动
                FS_status = SysDLL.Start(GlobalVariables.FS_serviceNameBytes);
                KC_status = SysDLL.Start(GlobalVariables.KC_serviceNameBytes);
                //启动
                if (FS_status && KC_status)
                {
                    //输出成功
                    System.Diagnostics.Trace.WriteLine("[R3]FileScanner+KernelCallBacks驱动启动成功...");

                    // 清除之前的任务（如果存在）
                    if (cancelTokenSource1 != null)
                    {
                        cancelTokenSource1.Dispose();
                    }
                    // 创建新的 CancellationTokenSource
                    cancelTokenSource1 = new CancellationTokenSource();

                    //新建一个Task来运行R0消息接收器
                    Task.Run(new Func<Task<int>>(MiniCoomTask), cancelTokenSource1.Token);

                    GlobalVariables.IsAD_Runing = true;
                    button1.Text = "关闭主动防御";
                    pictureBox2.Image = end_img;
                }

                else
                {
                    if(FS_status)
                        SysDLL.Stop(GlobalVariables.FS_serviceNameBytes);
                    else
                        System.Diagnostics.Trace.WriteLine("[R3]FileScanner驱动启动失败...");
                    if (KC_status)
                        SysDLL.Stop(GlobalVariables.FS_serviceNameBytes);
                    else
                        System.Diagnostics.Trace.WriteLine("[R3]KernelCallBacks驱动启动失败...");

                    //弹窗启示重启系统
                    string Error_text = "主动防御模块存在故障，无法进行防护!\n为防止将您的系统暴露在危险之中，请立即重启系统!";
                    MessageBox.Show(Error_text, "严重错误");

                }
            }

            else //主动防御模块已启动
            {

                //停止并卸载驱动
                FS_status = SysDLL.Stop(GlobalVariables.FS_serviceNameBytes);
                KC_status = SysDLL.Stop(GlobalVariables.KC_serviceNameBytes);

                if (FS_status && KC_status)
                {
                    //输出成功
                    System.Diagnostics.Trace.WriteLine("[R3]主动防御模块停止成功...");
                    //通过Token取消任务——R0消息接收器
                    System.Diagnostics.Trace.WriteLine("[R3]开始取消FS_R0消息接收器线程...");
                    cancelTokenSource1.Cancel();

                    GlobalVariables.IsAD_Runing = false;
                    button1.Text = "开启主动防御";
                    pictureBox2.Image = start_img;
                }
                else
                {
                    //输出失败
                    System.Diagnostics.Trace.WriteLine("[R3]主动防御模块停止失败...");
                }
            }
        }

        
        //测试方法
        private async Task<int> MiniCoomTask()
        {
            bool ret;
            int MyPort;
            System.Diagnostics.Trace.WriteLine("[R3]开始执行mytask线程...");

            string rulesFilePath = "clamav-ad.yara";
            IntPtr compiler = IntPtr.Zero;
            IntPtr compiledRules = IntPtr.Zero;
            int rett;

            // 初始化扫描器
            rett = YaraApi.InitializeYaraScanner(rulesFilePath, out compiler, out compiledRules);
            if (rett == 1)
            {
                System.Diagnostics.Trace.WriteLine("[主动防御]YARA扫描器初始化失败...");
                return -1;
            }
            System.Diagnostics.Trace.WriteLine("[主动防御]YARA扫描器初始化成功...");

            //建立连接并处理R0消息
            MyPort = AdUserDLL.ScanMain(5, 10, compiledRules);  //第一个参数是最大请求数，第二个参数是线程数

            //关闭端口连接
            ret = AdUserDLL.EndConnect();
            if (ret)
            {
                //输出成功
                System.Diagnostics.Trace.WriteLine("[R3]关闭FileScanner通信端口成功...");
            }
            else
            {
                //输出失败
                System.Diagnostics.Trace.WriteLine("[R3]关闭FileScanner通信端口失败...");
                return -1;
            }

            //关闭扫描器
            YaraApi.CleanupYaraScanner(compiler, compiledRules);
            System.Diagnostics.Trace.WriteLine("[主动防御]YARA扫描器关闭...");

            System.Diagnostics.Trace.WriteLine("[R3]取消my线程成功...");
            return 0;
        }
        

    }

}
