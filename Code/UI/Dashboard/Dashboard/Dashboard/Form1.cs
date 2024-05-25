using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;


namespace Dashboard
{
    public partial class Form1 : Form
    {
        private UserControl1 f1; //创建用户控件一变量
        private UserControl2 f2;
        private UserControl3 f3;
        private UserControl4 f4;
        private UserControl currentControl; // 用于跟踪当前显示的控件

        


        [DllImport("Gdi32.dll", EntryPoint = "CreateRoundRectRgn")]
        private static extern IntPtr CreateRoundRectRgn
            (
                int nLeftRect,
                int nTopRect,
                int nRightRect,
                int nBottomRect,
                int nWidthEllipse,
                int nHeightEllipse

            );

        private static bool IsDragging = false; //用于指示当前是不是在拖拽状态
        private Point StartPoint = new Point(0, 0); //记录鼠标按下去的坐标, new是为了拿到空间, 两个0无所谓的
        //记录动了多少距离,然后给窗体Location赋值,要设置Location,必须用一个Point结构体,不能直接给Location的X,Y赋值
        private Point OffsetPoint = new Point(0, 0);


        private void Form1_MouseDown(object sender, MouseEventArgs e)
        {
            //如果按下去的按钮不是左键就return,节省运算资源
            if (e.Button != MouseButtons.Left)
            {
                return;
            }
            //按下鼠标后,进入拖动状态:
            IsDragging = true;
            //保存刚按下时的鼠标坐标
            StartPoint.X = e.X;
            StartPoint.Y = e.Y;
        }

        private void Form1_MouseMove(object sender, MouseEventArgs e)
        {
            //鼠标移动时调用,检测到IsDragging为真时
            if (IsDragging == true)
            {
                //用当前坐标减去起始坐标得到偏移量Offset
                OffsetPoint.X = e.X - StartPoint.X;
                OffsetPoint.Y = e.Y - StartPoint.Y;
                //将Offset转化为屏幕坐标赋值给Location,设置Form在屏幕中的位置,如果不作PointToScreen转换,你自己看看效果就好
                Location = PointToScreen(OffsetPoint);
            }
        }

        private void Form1_MouseUp(object sender, MouseEventArgs e)
        {
            //左键抬起时,及时把拖动判定设置为false,否则,你也可以试试效果
            IsDragging = false;
        }
        

        public Form1()
        {
            InitializeComponent();
            Region = System.Drawing.Region.FromHrgn(CreateRoundRectRgn(0, 0, Width, Height, 25, 25));

            pnlNav.Height = btnDefense.Height;
            pnlNav.Top = btnDefense.Top;
            pnlNav.Left = btnDefense.Left;

            //安装驱动
            //FileScanner、KernelCallBacks、ForceDelete、SelfProtect通过inf文件安装

            //ProcessBlock较危险 通过程序主动安装(卸载)
            SysDLL.InstallDriver(GlobalVariables.PB_sysPathBytes, GlobalVariables.PB_serviceNameBytes, GlobalVariables.PB_displayNameBytes);

            System.Diagnostics.Trace.WriteLine("[R3]安装所有驱动成功...");
        }

        private void Form1_Load(object sender, EventArgs e)
        {

            //添加鼠标移动窗体的处理函数
            MouseDown += Form1_MouseDown;
            MouseMove += Form1_MouseMove;
            MouseUp += Form1_MouseUp;

            //实例化不同选项对应的页面
            f1 = new UserControl1();    //实例化f1
            f2 = new UserControl2();    //实例化f2
            f3 = new UserControl3();    //实例化f3
            f4 = new UserControl4();    //实例化f4
            panelPageContainer.Controls.Add(f1); // 将 f1 添加到 panelPageContainer 中
            panelPageContainer.Controls.Add(f2); // 将 f2 添加到 panelPageContainer 中
            panelPageContainer.Controls.Add(f3); // 将 f3 添加到 panelPageContainer 中
            panelPageContainer.Controls.Add(f4); // 将 f3 添加到 panelPageContainer 中
            f1.Hide();
            f2.Hide();
            f3.Hide();
            f4.Hide();

            currentControl = f1; // 设置初始显示的控件为 f1
            currentControl.Show(); // 显示 f1

        }

        private void btnDefense_Click(object sender, EventArgs e)
        {
            pnlNav.Height = btnDefense.Height;
            pnlNav.Top = btnDefense.Top;
            btnDefense.BackColor = Color.FromArgb(46, 51, 73);

            currentControl.Hide(); // 隐藏当前显示的控件
            currentControl = f1; // 更新当前显示的控件为 f1
            currentControl.Show(); // 显示 f1
            
        }

        private void btnScan_Click(object sender, EventArgs e)
        {
            pnlNav.Height = btnScan.Height;
            pnlNav.Top = btnScan.Top;
            btnScan.BackColor = Color.FromArgb(46, 51, 73);

            currentControl.Hide(); // 隐藏当前显示的控件
            currentControl = f2; // 更新当前显示的控件为 f2
            currentControl.Show(); // 显示 f2

        }

        private void btnContactUs_Click(object sender, EventArgs e)
        {
            pnlNav.Height = btnContactUs.Height;
            pnlNav.Top = btnContactUs.Top;
            btnContactUs.BackColor = Color.FromArgb(46, 51, 73);

            currentControl.Hide(); // 隐藏当前显示的控件
            currentControl = f3; // 更新当前显示的控件为 f3
            currentControl.Show(); // 显示 f3
        }

        private void btnsettings_Click(object sender, EventArgs e)
        {
            pnlNav.Height = btnsettings.Height;
            pnlNav.Top = btnsettings.Top;
            btnsettings.BackColor = Color.FromArgb(46, 51, 73);

            currentControl.Hide(); // 隐藏当前显示的控件
            currentControl = f4; // 更新当前显示的控件为 f4
            currentControl.Show(); // 显示 f4
        }

        private void btnDefense_Leave(object sender, EventArgs e)
        {
            btnDefense.BackColor = Color.FromArgb(24, 30, 54);
        }

        private void btnScan_Leave(object sender, EventArgs e)
        {
            btnScan.BackColor = Color.FromArgb(24, 30, 54);
        }

        private void btnContactUs_Leave(object sender, EventArgs e)
        {
            btnContactUs.BackColor = Color.FromArgb(24, 30, 54);
        }

        private void btnsettings_Leave(object sender, EventArgs e)
        {
            btnsettings.BackColor = Color.FromArgb(24, 30, 54);
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            if (MessageBox.Show("是否确认退出程序？", "退出", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) == DialogResult.OK)
            {
                // 终止程序
                Application.Exit();
            }
        }

        //主窗口关闭时
        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            System.Diagnostics.Trace.WriteLine("[R3]尝试卸载所有驱动...");

            //还没停止主动防御模块的驱动
            if (GlobalVariables.IsAD_Runing) 
            {
                //停止FS驱动
                bool ret = SysDLL.Stop(GlobalVariables.FS_serviceNameBytes);
                if (ret)
                {
                    //输出成功
                    System.Diagnostics.Trace.WriteLine("[R3]FS驱动停止成功...");
                }
                else
                {
                    //输出失败
                    System.Diagnostics.Trace.WriteLine("[R3]FS驱动停止失败...");
                    return;
                }

                //停止KC驱动
                ret = SysDLL.Stop(GlobalVariables.KC_serviceNameBytes);
                if (ret)
                {
                    //输出成功
                    System.Diagnostics.Trace.WriteLine("[R3]KC驱动停止成功...");
                }
                else
                {
                    //输出失败
                    System.Diagnostics.Trace.WriteLine("[R3]KC驱动停止失败...");
                    return;
                }
            }

            //还没停止PB驱动
            if (GlobalVariables.IsPB_Runing)
            {
                bool ret = SysDLL.Stop(GlobalVariables.PB_serviceNameBytes);
                if (ret)
                {
                    //输出成功
                    System.Diagnostics.Trace.WriteLine("[R3]PB驱动停止成功...");
                }
                else
                {
                    //输出失败
                    System.Diagnostics.Trace.WriteLine("[R3]PB驱动停止失败...");
                    return;
                }

            }

            //还没停止FD驱动
            if (GlobalVariables.IsFD_Runing)
            {
                bool ret = SysDLL.Stop(GlobalVariables.FD_serviceNameBytes);
                if (ret)
                {
                    //输出成功
                    System.Diagnostics.Trace.WriteLine("[R3]FD驱动停止成功...");
                }
                else
                {
                    //输出失败
                    System.Diagnostics.Trace.WriteLine("[R3]FD驱动停止失败...");
                    return;
                }

            }

            //还没停止SP驱动
            if (GlobalVariables.IsSP_Runing)
            {
                bool ret = SysDLL.Stop(GlobalVariables.SP_serviceNameBytes);
                if (ret)
                {
                    //输出成功
                    System.Diagnostics.Trace.WriteLine("[R3]SP驱动停止成功...");
                }
                else
                {
                    //输出失败
                    System.Diagnostics.Trace.WriteLine("[R3]SP驱动停止失败...");
                    return;
                }

            }

            SysDLL.Remove(GlobalVariables.PB_serviceNameBytes);
            System.Diagnostics.Trace.WriteLine("[R3]卸载所有驱动成功...");
        }

        private void button2_Click(object sender, EventArgs e)
        {
            //隐藏窗体
            this.Visible = false;
            //图标显示在托盘区
            notifyIcon1.Visible = true;
            //notifyIcon1.ShowBalloonTip(2000, "提示", "双击图标恢复", ToolTipIcon.Info);
        }

        private void notifyIcon1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            // 显示窗体
            this.Visible = true;
            //激活窗体并给予它焦点
            this.Activate();
            //托盘区图标隐藏
            notifyIcon1.Visible = false;
        }

        private void 退出ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("是否确认退出程序？", "退出", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) == DialogResult.OK)
            {
                // 终止程序
                Application.Exit();
            }
        }

        private void 还原ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Visible = true;
        }

        private void lbltitle_Click(object sender, EventArgs e)
        {

        }

        private void panelPageContainer_Paint(object sender, PaintEventArgs e)
        {

        }
    }

    public static class GlobalVariables
    {
        /* 病毒功能模块 */
        //病毒扫描器是否启动
        public static bool IsScanner_Runing;      //扫描器启动标志

        /* 主动防御模块驱动 */
        public static bool IsAD_Runing;      //主动防御ActiveDefense启动标志

        //FileScanner安装参数
        private static string FS_serviceName = "FileScanner";
        private static string FS_displayName = "FileScanner";
        private static string FS_sysPath = ".\\FileScanner.sys";
        private static string FS_Altitude = "265000";
        public static byte[] FS_serviceNameBytes = Encoding.ASCII.GetBytes(FS_serviceName);

        //KernelCallBacks安装参数
        private static string KC_serviceName = "KernelCallBacks";
        public static byte[] KC_serviceNameBytes = Encoding.ASCII.GetBytes(KC_serviceName);

        //ProcessBlock安装参数
        public static bool IsPB_Runing;      //PB启动标志
        private static string PB_serviceName = "ProcessBlock";
        private static string PB_displayName = "ProcessBlock";
        private static string PB_sysPath = ".\\ProcessBlock.sys";
        public static byte[] PB_serviceNameBytes = Encoding.ASCII.GetBytes(PB_serviceName);
        public static byte[] PB_displayNameBytes = Encoding.ASCII.GetBytes(PB_displayName);
        public static byte[] PB_sysPathBytes = Encoding.ASCII.GetBytes(PB_sysPath);

        /*病毒清除模块*/
        //ForceDelete安装参数
        public static bool IsFD_Runing;
        private static string FD_serviceName = "ForceDelete";
        public static byte[] FD_serviceNameBytes = Encoding.ASCII.GetBytes(FD_serviceName);

        /*自我保护模块*/
        //SelfProtect安装参数
        public static bool IsSP_Runing;
        private static string SP_serviceName = "SelfProtect";
        public static byte[] SP_serviceNameBytes = Encoding.ASCII.GetBytes(SP_serviceName);
        public static string SP_RegPath = @"SYSTEM\ControlSet001\Services\SelfProtect";
        public static string SP_ModeKey = "Mode";
        public static string SP_PidKey = "Pid";

       
        
        
    }
}

   

