
namespace Dashboard
{
    partial class UserControl2
    {
        /// <summary> 
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region 组件设计器生成的代码

        /// <summary> 
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.btnDiy = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.btnStop = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.STimeOut = new System.Windows.Forms.Label();
            this.SNumOut = new System.Windows.Forms.Label();
            this.SDirOut = new System.Windows.Forms.Label();
            this.panel1 = new System.Windows.Forms.Panel();
            this.richTextBox1 = new System.Windows.Forms.RichTextBox();
            this.panel2 = new System.Windows.Forms.Panel();
            this.panel3 = new System.Windows.Forms.Panel();
            this.btnRapid = new System.Windows.Forms.Button();
            this.btnAll = new System.Windows.Forms.Button();
            this.panel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.panel3.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnDiy
            // 
            this.btnDiy.BackColor = System.Drawing.Color.CornflowerBlue;
            this.btnDiy.Cursor = System.Windows.Forms.Cursors.Default;
            this.btnDiy.FlatAppearance.BorderColor = System.Drawing.Color.Black;
            this.btnDiy.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
            this.btnDiy.Font = new System.Drawing.Font("华文琥珀", 10F);
            this.btnDiy.ForeColor = System.Drawing.Color.White;
            this.btnDiy.Image = global::Dashboard.Properties.Resources.安全扫描2;
            this.btnDiy.Location = new System.Drawing.Point(313, 13);
            this.btnDiy.Name = "btnDiy";
            this.btnDiy.Size = new System.Drawing.Size(103, 100);
            this.btnDiy.TabIndex = 0;
            this.btnDiy.Text = "自定义扫描";
            this.btnDiy.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
            this.btnDiy.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
            this.btnDiy.UseMnemonic = false;
            this.btnDiy.UseVisualStyleBackColor = false;
            this.btnDiy.Click += new System.EventHandler(this.btnDiy_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label1.Font = new System.Drawing.Font("宋体", 10F);
            this.label1.ForeColor = System.Drawing.SystemColors.ButtonFace;
            this.label1.Location = new System.Drawing.Point(15, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(51, 16);
            this.label1.TabIndex = 1;
            this.label1.Text = "文件数";
            // 
            // btnStop
            // 
            this.btnStop.BackColor = System.Drawing.Color.CornflowerBlue;
            this.btnStop.FlatAppearance.BorderColor = System.Drawing.Color.Black;
            this.btnStop.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
            this.btnStop.Font = new System.Drawing.Font("华文琥珀", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.btnStop.ForeColor = System.Drawing.SystemColors.ButtonHighlight;
            this.btnStop.Location = new System.Drawing.Point(341, 10);
            this.btnStop.Name = "btnStop";
            this.btnStop.Size = new System.Drawing.Size(75, 32);
            this.btnStop.TabIndex = 4;
            this.btnStop.Text = "终止";
            this.btnStop.UseVisualStyleBackColor = false;
            this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label2.Font = new System.Drawing.Font("宋体", 10F);
            this.label2.ForeColor = System.Drawing.SystemColors.ButtonFace;
            this.label2.Location = new System.Drawing.Point(15, 35);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(37, 16);
            this.label2.TabIndex = 5;
            this.label2.Text = "耗时";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label3.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.label3.Font = new System.Drawing.Font("宋体", 9F);
            this.label3.ForeColor = System.Drawing.SystemColors.ButtonFace;
            this.label3.Location = new System.Drawing.Point(15, 28);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(61, 14);
            this.label3.TabIndex = 6;
            this.label3.Text = "扫描结果:";
            // 
            // STimeOut
            // 
            this.STimeOut.AutoSize = true;
            this.STimeOut.Font = new System.Drawing.Font("宋体", 10F);
            this.STimeOut.ForeColor = System.Drawing.SystemColors.ButtonFace;
            this.STimeOut.Location = new System.Drawing.Point(80, 35);
            this.STimeOut.Name = "STimeOut";
            this.STimeOut.Size = new System.Drawing.Size(21, 14);
            this.STimeOut.TabIndex = 7;
            this.STimeOut.Text = "无";
            // 
            // SNumOut
            // 
            this.SNumOut.AutoSize = true;
            this.SNumOut.Font = new System.Drawing.Font("宋体", 10F);
            this.SNumOut.ForeColor = System.Drawing.SystemColors.ButtonFace;
            this.SNumOut.Location = new System.Drawing.Point(87, 10);
            this.SNumOut.Name = "SNumOut";
            this.SNumOut.Size = new System.Drawing.Size(14, 14);
            this.SNumOut.TabIndex = 8;
            this.SNumOut.Text = "0";
            // 
            // SDirOut
            // 
            this.SDirOut.AutoSize = true;
            this.SDirOut.Font = new System.Drawing.Font("宋体", 9F);
            this.SDirOut.ForeColor = System.Drawing.SystemColors.ButtonFace;
            this.SDirOut.Location = new System.Drawing.Point(13, 10);
            this.SDirOut.Name = "SDirOut";
            this.SDirOut.Size = new System.Drawing.Size(65, 12);
            this.SDirOut.TabIndex = 10;
            this.SDirOut.Text = "未开始扫描";
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(24)))), ((int)(((byte)(30)))), ((int)(((byte)(70)))));
            this.panel1.Controls.Add(this.richTextBox1);
            this.panel1.Controls.Add(this.label3);
            this.panel1.Controls.Add(this.SDirOut);
            this.panel1.Location = new System.Drawing.Point(14, 210);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(674, 190);
            this.panel1.TabIndex = 11;
            // 
            // richTextBox1
            // 
            this.richTextBox1.Location = new System.Drawing.Point(15, 45);
            this.richTextBox1.Name = "richTextBox1";
            this.richTextBox1.ReadOnly = true;
            this.richTextBox1.Size = new System.Drawing.Size(645, 133);
            this.richTextBox1.TabIndex = 11;
            this.richTextBox1.Text = "";
            this.richTextBox1.WordWrap = false;
            // 
            // panel2
            // 
            this.panel2.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(24)))), ((int)(((byte)(30)))), ((int)(((byte)(70)))));
            this.panel2.Controls.Add(this.label2);
            this.panel2.Controls.Add(this.btnStop);
            this.panel2.Controls.Add(this.STimeOut);
            this.panel2.Controls.Add(this.label1);
            this.panel2.Controls.Add(this.SNumOut);
            this.panel2.Location = new System.Drawing.Point(14, 147);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(437, 57);
            this.panel2.TabIndex = 12;
            // 
            // panel3
            // 
            this.panel3.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(24)))), ((int)(((byte)(30)))), ((int)(((byte)(70)))));
            this.panel3.Controls.Add(this.btnRapid);
            this.panel3.Controls.Add(this.btnAll);
            this.panel3.Controls.Add(this.btnDiy);
            this.panel3.Location = new System.Drawing.Point(14, 13);
            this.panel3.Name = "panel3";
            this.panel3.Size = new System.Drawing.Size(437, 128);
            this.panel3.TabIndex = 13;
            // 
            // btnRapid
            // 
            this.btnRapid.BackColor = System.Drawing.Color.CornflowerBlue;
            this.btnRapid.Cursor = System.Windows.Forms.Cursors.Default;
            this.btnRapid.FlatAppearance.BorderColor = System.Drawing.Color.Black;
            this.btnRapid.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
            this.btnRapid.Font = new System.Drawing.Font("华文琥珀", 10F);
            this.btnRapid.ForeColor = System.Drawing.Color.White;
            this.btnRapid.Image = global::Dashboard.Properties.Resources.安全扫描2;
            this.btnRapid.Location = new System.Drawing.Point(20, 13);
            this.btnRapid.Name = "btnRapid";
            this.btnRapid.Size = new System.Drawing.Size(111, 100);
            this.btnRapid.TabIndex = 2;
            this.btnRapid.Text = "快速扫描";
            this.btnRapid.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
            this.btnRapid.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
            this.btnRapid.UseMnemonic = false;
            this.btnRapid.UseVisualStyleBackColor = false;
            this.btnRapid.Click += new System.EventHandler(this.btnRapid_Click);
            // 
            // btnAll
            // 
            this.btnAll.BackColor = System.Drawing.Color.CornflowerBlue;
            this.btnAll.Cursor = System.Windows.Forms.Cursors.Default;
            this.btnAll.FlatAppearance.BorderColor = System.Drawing.Color.Black;
            this.btnAll.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
            this.btnAll.Font = new System.Drawing.Font("华文琥珀", 10F);
            this.btnAll.ForeColor = System.Drawing.Color.White;
            this.btnAll.Image = global::Dashboard.Properties.Resources.安全扫描2;
            this.btnAll.Location = new System.Drawing.Point(169, 13);
            this.btnAll.Name = "btnAll";
            this.btnAll.Size = new System.Drawing.Size(109, 100);
            this.btnAll.TabIndex = 1;
            this.btnAll.Text = "全盘扫描";
            this.btnAll.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
            this.btnAll.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
            this.btnAll.UseMnemonic = false;
            this.btnAll.UseVisualStyleBackColor = false;
            this.btnAll.Click += new System.EventHandler(this.btnAll_Click);
            // 
            // UserControl2
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.Khaki;
            this.BackgroundImage = global::Dashboard.Properties.Resources.wallhaven_01wow3_1280x7201;
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.Controls.Add(this.panel3);
            this.Controls.Add(this.panel2);
            this.Controls.Add(this.panel1);
            this.DoubleBuffered = true;
            this.Name = "UserControl2";
            this.Size = new System.Drawing.Size(700, 400);
            this.Load += new System.EventHandler(this.UserControl2_Load);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.panel2.ResumeLayout(false);
            this.panel2.PerformLayout();
            this.panel3.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button btnDiy;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnStop;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label STimeOut;
        private System.Windows.Forms.Label SNumOut;
        private System.Windows.Forms.Label SDirOut;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Panel panel3;
        private System.Windows.Forms.Button btnRapid;
        private System.Windows.Forms.Button btnAll;
        private System.Windows.Forms.RichTextBox richTextBox1;
    }
}
