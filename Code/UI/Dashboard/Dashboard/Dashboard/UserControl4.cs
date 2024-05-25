using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Win32;

namespace Dashboard
{
    public partial class UserControl4 : UserControl
    {
        public UserControl4()
        {
            InitializeComponent();

            //一开始未启动
            GlobalVariables.IsPB_Runing = false;
            GlobalVariables.IsSP_Runing = false;
        }

        private void ChangeRegister(string subkeyPath, string valueName, int newValue)
        {

            try
            {
                RegistryKey hklm = Registry.LocalMachine;
                // 打开指定的注册表项
                using (RegistryKey key = hklm.OpenSubKey(subkeyPath, true))
                {
                    if (key != null)
                    {
                        // 修改键值
                        key.SetValue(valueName, newValue);

                        System.Diagnostics.Trace.WriteLine("注册表键值修改成功！");
                    }
                    else
                    {
                        System.Diagnostics.Trace.WriteLine("无法打开注册表项！");
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine("出现异常: " + ex.Message);
            }
        }

        //注意：radioButton.Checked的值是在执行完radioButton_CheckedChanged后才被改变的
        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            //按钮现在还没被选中，即将被选中
            if (radioButton2.Checked)
            {
                System.Diagnostics.Trace.WriteLine("[R3]严格模式按钮被选中了...");
                if (!GlobalVariables.IsPB_Runing)
                {
                    //启动ProcessBlock驱动
                    bool ret = SysDLL.Start(GlobalVariables.PB_serviceNameBytes);
                    if (ret)
                    {
                        //输出成功
                        System.Diagnostics.Trace.WriteLine("[R3]ProcessBlock驱动启动成功...");
                        GlobalVariables.IsPB_Runing = true;
                    }
                    else
                    {
                        //输出失败
                        System.Diagnostics.Trace.WriteLine("[R3]ProcessBlock驱动启动失败...");
                    }
                }
                else
                {
                    System.Diagnostics.Trace.WriteLine("[R3]ProcessBlock驱动已经启动...");
                }
                 
            }
            //按钮现在处于选中状态中，即将被取消选中
            else
            {
                System.Diagnostics.Trace.WriteLine("[R3]严格模式按钮被取消选中...");
                if (GlobalVariables.IsPB_Runing)
                {
                    //停止ProcessBlock驱动
                    bool ret = SysDLL.Stop(GlobalVariables.PB_serviceNameBytes);
                    if (ret)
                    {
                        //输出成功
                        System.Diagnostics.Trace.WriteLine("[R3]ProcessBlock驱动停止成功...");
                        GlobalVariables.IsPB_Runing = false;
                    }
                    else
                    {
                        //输出失败
                        System.Diagnostics.Trace.WriteLine("[R3]ProcessBlock驱动停止失败...");
                    }
                }
                else
                {
                    System.Diagnostics.Trace.WriteLine("[R3]ProcessBlock驱动已经停止...");
                }
            }
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            //按钮现在还没被选中，即将被选中
            if (radioButton1.Checked)
            {
                System.Diagnostics.Trace.WriteLine("[R3]普通模式按钮被选中了...");
            }
        }

        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            bool ret;
            //按钮现在还没被选中，即将被选中
            if (radioButton3.Checked)
            {
                System.Diagnostics.Trace.WriteLine("[R3]开启进程隐藏...");
                if (!GlobalVariables.IsSP_Runing)
                {
                    //修改注册表对应键值为0
                    ChangeRegister(GlobalVariables.SP_RegPath, GlobalVariables.SP_ModeKey, 0);
                    //修改注册表对应键值为PID
                    System.Diagnostics.Trace.WriteLine($"当前进程pid = ({Process.GetCurrentProcess().Id})");
                    ChangeRegister(GlobalVariables.SP_RegPath, GlobalVariables.SP_PidKey, Process.GetCurrentProcess().Id);

                    ret = SysDLL.Start(GlobalVariables.SP_serviceNameBytes);
                    if (ret)
                    {
                        //输出成功
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动启动成功...");
                        GlobalVariables.IsSP_Runing = true;

                    }
                    else
                    {
                        //输出失败
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动启动失败...");
                    }
                }
            }
            else
            {
                System.Diagnostics.Trace.WriteLine("[R3]关闭进程隐藏...");
                if (GlobalVariables.IsSP_Runing)
                {
                    ret = SysDLL.Stop(GlobalVariables.SP_serviceNameBytes);
                    if (ret)
                    {
                        //输出成功
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动停止成功...");
                        GlobalVariables.IsSP_Runing = false;
                    }
                    else
                    {
                        //输出失败
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动停止失败...");
                    }
                }   
            }
        }

        private void radioButton4_CheckedChanged(object sender, EventArgs e)
        {
            bool ret;
            //按钮现在还没被选中，即将被选中
            if (radioButton4.Checked)
            {
                System.Diagnostics.Trace.WriteLine("[R3]开启进程防杀...");
                if (!GlobalVariables.IsSP_Runing)
                { 
                    //修改注册表对应键值为1
                    ChangeRegister(GlobalVariables.SP_RegPath, GlobalVariables.SP_ModeKey, 1);
                    //修改注册表对应键值为PID
                    System.Diagnostics.Trace.WriteLine($"当前进程pid = ({Process.GetCurrentProcess().Id})");
                    ChangeRegister(GlobalVariables.SP_RegPath, GlobalVariables.SP_PidKey, Process.GetCurrentProcess().Id);

                    ret = SysDLL.Start(GlobalVariables.SP_serviceNameBytes);
                    if (ret)
                    {
                        //输出成功
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动启动成功...");
                        GlobalVariables.IsSP_Runing = true;
                    }
                    else
                    {
                        //输出失败
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动启动失败...");
                    }
                } 
            }
            else
            {
                System.Diagnostics.Trace.WriteLine("[R3]关闭进程隐藏...");
                if (GlobalVariables.IsSP_Runing)
                {
                    ret = SysDLL.Stop(GlobalVariables.SP_serviceNameBytes);
                    if (ret)
                    {
                        //输出成功
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动停止成功...");
                        GlobalVariables.IsSP_Runing = false;
                    }
                    else
                    {
                        //输出失败
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动停止失败...");
                    }
                }
            }
        }

        private void radioButton5_CheckedChanged(object sender, EventArgs e)
        {
            bool ret;
            //按钮现在还没被选中，即将被选中
            if (radioButton4.Checked)
            {
                System.Diagnostics.Trace.WriteLine("[R3]关闭保护模式...");
                if (GlobalVariables.IsSP_Runing)
                {
                    ret = SysDLL.Stop(GlobalVariables.SP_serviceNameBytes);
                    if (ret)
                    {
                        //输出成功
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动停止成功...");
                        GlobalVariables.IsSP_Runing = false;
                    }
                    else
                    {
                        //输出失败
                        System.Diagnostics.Trace.WriteLine("[R3]SelfProtect驱动停止失败...");
                    }
                }     
            }
        }

    }
}
