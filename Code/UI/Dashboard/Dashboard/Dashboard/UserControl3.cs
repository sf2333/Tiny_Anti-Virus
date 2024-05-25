using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.WindowsAPICodePack.Dialogs;
using System.IO;

namespace Dashboard
{

    public partial class UserControl3 : UserControl
    {
        private FileDropHandler FileDroper; //全局变量

        public UserControl3()
        {
            InitializeComponent();

            //启动ForceDelete驱动
            bool ret = SysDLL.Start(GlobalVariables.FD_serviceNameBytes);
            if (ret)
            {
                //输出成功
                System.Diagnostics.Trace.WriteLine("[R3]ForceDelete驱动启动成功...");
                GlobalVariables.IsFD_Runing = true;
            }
            else
            {
                //输出失败
                System.Diagnostics.Trace.WriteLine("[R3]ForceDelete驱动启动失败...");
                GlobalVariables.IsFD_Runing = false;
                return;
            }
        }

        

        private void UserControl3_Load(object sender, EventArgs e)
        {

            //使panel2不会因为UAC而无法拖拽
            FileDroper = new FileDropHandler(panel2); 
        }

        
        public static bool CleanFile(string filePath)
        {
            if (File.Exists(filePath))
            {
                // 处理存在的文件
                bool ret = AdUserDLL.SendFileName(filePath);
                if (ret)
                {
                    if (!File.Exists(filePath))
                    {
                        System.Diagnostics.Trace.WriteLine("[病毒清除]文件清除成功!");
                        return true;
                    }
                    else
                    {
                        System.Diagnostics.Trace.WriteLine("[病毒清除]文件清除失败!");
                    }
                }
                else
                {
                    System.Diagnostics.Trace.WriteLine("[病毒清除]模块故障，请重启软件!");
                }
                    
            }
            else
            {
                System.Diagnostics.Trace.WriteLine("[病毒清除]文件不存在!");
            }

            return false;
        }
        private void btnDiy_Click(object sender, EventArgs e)
        {
            if (GlobalVariables.IsFD_Runing)
            {
                richTextBox1.Text = "";

                //在VS里打开Package Manager Console后输入Install-Package WindowsAPICodePack-Shell https://www.nuget.org/packages/WindowsAPICodePack-Shell/
                CommonOpenFileDialog openFileDialog = new CommonOpenFileDialog();
                openFileDialog.Title = "选择文件";
                openFileDialog.Filters.Add(new CommonFileDialogFilter("所有文件", "*.*")); // 添加文件过滤器，可选
                openFileDialog.InitialDirectory = Environment.CurrentDirectory; // 设置初始目录，可选

                if (openFileDialog.ShowDialog() == CommonFileDialogResult.Ok)
                {
                    string filePath = openFileDialog.FileName;
                    int lastIndex = filePath.LastIndexOf('\\');
                    string fileName = filePath.Substring(lastIndex + 1);
                   

                    // 处理文件路径，例如读取文件内容或执行其他操作
                    bool ret = CleanFile(filePath);
                    if(ret)
                    {
                        string fmt = string.Format("成功删除文件：{0}!", fileName);
                        MessageBox.Show(fmt);
                        richTextBox1.Text = fmt;
                    }
                    else
                    {
                        string fmt = string.Format("失败删除文件：{0}!", fileName);
                        MessageBox.Show(fmt);
                        richTextBox1.Text = fmt;
                    }

                }
            }
            else
            {
                //richTextBox1.Text += "ForceDelete驱动启动错误!\n";
                System.Diagnostics.Trace.WriteLine("[病毒清除]ForceDelete驱动启动错误!");
            }
        }


        private void panel2_DragEnter(object sender, DragEventArgs e)
        {
            string[] paths = (string[])e.Data.GetData(typeof(string[]));
            // 打印每个文件的路径

            richTextBox1.Text = "";
            foreach (string path in paths)
            {
                int lastIndex = path.LastIndexOf('\\');
                string fileName = path.Substring(lastIndex + 1);
 
                // 处理文件路径，例如读取文件内容或执行其他操作
                bool ret = CleanFile(path);
                if (ret)
                {
                    string fmt = string.Format("成功删除文件：{0}!", fileName);
                    //不能调用MessageBox会卡住
                    richTextBox1.Text = fmt;
                }
                else
                {
                    string fmt = string.Format("失败删除文件：{0}!", fileName);
                    richTextBox1.Text = fmt;
                }
            }

            System.Diagnostics.Trace.WriteLine("[病毒清除]panel2_DragEnter调用结束!");

        }

        private void panel2_DragDrop(object sender, DragEventArgs e)
        {
            System.Diagnostics.Trace.WriteLine("[病毒清除]panel2_DragDrop被调用了!");
        }

    }
}
