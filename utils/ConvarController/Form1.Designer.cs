namespace ConvarController
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.lstMiscConvars = new System.Windows.Forms.CheckedListBox();
            this.lblDesc = new System.Windows.Forms.Label();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.lstAllModules = new System.Windows.Forms.CheckedListBox();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPageList = new System.Windows.Forms.TabPage();
            this.tabPageModules = new System.Windows.Forms.TabPage();
            this.moduleVarPanel = new System.Windows.Forms.FlowLayoutPanel();
            this.rbOne = new System.Windows.Forms.RadioButton();
            this.rbAll = new System.Windows.Forms.RadioButton();
            this.lblExpand = new System.Windows.Forms.Label();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.btnLoadConvars = new System.Windows.Forms.ToolStripButton();
            this.btnLoadValues = new System.Windows.Forms.ToolStripButton();
            this.btnSaveValues = new System.Windows.Forms.ToolStripButton();
            this.btnReset = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
            this.txtFilter = new System.Windows.Forms.ToolStripTextBox();
            this.conVarControl1 = new ConvarController.ConVarControl();
            this.levelVarControl1 = new ConvarController.LevelVarControl();
            this.tabControl1.SuspendLayout();
            this.tabPageList.SuspendLayout();
            this.tabPageModules.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // lstMiscConvars
            // 
            this.lstMiscConvars.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            this.lstMiscConvars.FormattingEnabled = true;
            this.lstMiscConvars.Location = new System.Drawing.Point(3, 3);
            this.lstMiscConvars.Name = "lstMiscConvars";
            this.lstMiscConvars.Size = new System.Drawing.Size(204, 274);
            this.lstMiscConvars.Sorted = true;
            this.lstMiscConvars.TabIndex = 1;
            this.toolTip1.SetToolTip(this.lstMiscConvars, "A check indicates that the ConVar is set to its default value");
            this.lstMiscConvars.SelectedIndexChanged += new System.EventHandler(this.checkedListBox1_SelectedIndexChanged);
            this.lstMiscConvars.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBox1_ItemCheck);
            // 
            // lblDesc
            // 
            this.lblDesc.AutoSize = true;
            this.lblDesc.Location = new System.Drawing.Point(239, 70);
            this.lblDesc.MaximumSize = new System.Drawing.Size(190, 130);
            this.lblDesc.Name = "lblDesc";
            this.lblDesc.Size = new System.Drawing.Size(0, 13);
            this.lblDesc.TabIndex = 4;
            // 
            // lstAllModules
            // 
            this.lstAllModules.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            this.lstAllModules.FormattingEnabled = true;
            this.lstAllModules.Location = new System.Drawing.Point(3, 3);
            this.lstAllModules.Name = "lstAllModules";
            this.lstAllModules.Size = new System.Drawing.Size(141, 289);
            this.lstAllModules.Sorted = true;
            this.lstAllModules.TabIndex = 2;
            this.toolTip1.SetToolTip(this.lstAllModules, "A check indicates that all variables for the Module are set to their default valu" +
                    "es");
            this.lstAllModules.SelectedIndexChanged += new System.EventHandler(this.lstAllModules_SelectedIndexChanged);
            this.lstAllModules.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBox1_ItemCheck);
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.tabPageList);
            this.tabControl1.Controls.Add(this.tabPageModules);
            this.tabControl1.Location = new System.Drawing.Point(0, 24);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(437, 307);
            this.tabControl1.TabIndex = 9;
            this.tabControl1.SelectedIndexChanged += new System.EventHandler(this.tabControl1_SelectedIndexChanged);
            // 
            // tabPageList
            // 
            this.tabPageList.Controls.Add(this.conVarControl1);
            this.tabPageList.Controls.Add(this.lstMiscConvars);
            this.tabPageList.Controls.Add(this.lblDesc);
            this.tabPageList.Controls.Add(this.levelVarControl1);
            this.tabPageList.Location = new System.Drawing.Point(4, 22);
            this.tabPageList.Name = "tabPageList";
            this.tabPageList.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageList.Size = new System.Drawing.Size(429, 281);
            this.tabPageList.TabIndex = 0;
            this.tabPageList.Text = "Misc.";
            this.tabPageList.UseVisualStyleBackColor = true;
            // 
            // tabPageModules
            // 
            this.tabPageModules.Controls.Add(this.moduleVarPanel);
            this.tabPageModules.Controls.Add(this.lstAllModules);
            this.tabPageModules.Location = new System.Drawing.Point(4, 22);
            this.tabPageModules.Name = "tabPageModules";
            this.tabPageModules.Size = new System.Drawing.Size(429, 306);
            this.tabPageModules.TabIndex = 3;
            this.tabPageModules.Text = "Modules";
            this.tabPageModules.UseVisualStyleBackColor = true;
            // 
            // moduleVarPanel
            // 
            this.moduleVarPanel.AutoScroll = true;
            this.moduleVarPanel.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.moduleVarPanel.Location = new System.Drawing.Point(150, 3);
            this.moduleVarPanel.Name = "moduleVarPanel";
            this.moduleVarPanel.Size = new System.Drawing.Size(269, 259);
            this.moduleVarPanel.TabIndex = 3;
            this.moduleVarPanel.WrapContents = false;
            // 
            // rbOne
            // 
            this.rbOne.AutoSize = true;
            this.rbOne.Checked = true;
            this.rbOne.Location = new System.Drawing.Point(345, 1);
            this.rbOne.Name = "rbOne";
            this.rbOne.Size = new System.Drawing.Size(45, 17);
            this.rbOne.TabIndex = 11;
            this.rbOne.TabStop = true;
            this.rbOne.Text = "One";
            this.rbOne.UseVisualStyleBackColor = true;
            this.rbOne.Visible = false;
            this.rbOne.CheckedChanged += new System.EventHandler(this.rbOne_CheckedChanged);
            // 
            // rbAll
            // 
            this.rbAll.AutoSize = true;
            this.rbAll.Location = new System.Drawing.Point(396, 1);
            this.rbAll.Name = "rbAll";
            this.rbAll.Size = new System.Drawing.Size(36, 17);
            this.rbAll.TabIndex = 11;
            this.rbAll.Text = "All";
            this.rbAll.UseVisualStyleBackColor = true;
            this.rbAll.Visible = false;
            this.rbAll.CheckedChanged += new System.EventHandler(this.rbAll_CheckedChanged);
            // 
            // lblExpand
            // 
            this.lblExpand.AutoSize = true;
            this.lblExpand.Location = new System.Drawing.Point(284, 3);
            this.lblExpand.Name = "lblExpand";
            this.lblExpand.Size = new System.Drawing.Size(58, 13);
            this.lblExpand.TabIndex = 12;
            this.lblExpand.Text = "Expanded:";
            this.lblExpand.Visible = false;
            // 
            // toolStrip1
            // 
            this.toolStrip1.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.btnLoadConvars,
            this.btnLoadValues,
            this.btnSaveValues,
            this.btnReset,
            this.toolStripSeparator1,
            this.toolStripLabel1,
            this.txtFilter});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(437, 25);
            this.toolStrip1.Stretch = true;
            this.toolStrip1.TabIndex = 13;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // btnLoadConvars
            // 
            this.btnLoadConvars.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.btnLoadConvars.Image = ((System.Drawing.Image)(resources.GetObject("btnLoadConvars.Image")));
            this.btnLoadConvars.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnLoadConvars.Name = "btnLoadConvars";
            this.btnLoadConvars.Size = new System.Drawing.Size(75, 22);
            this.btnLoadConvars.Text = "Load convars";
            this.btnLoadConvars.ToolTipText = "Click to load a list of all game convars, generated by the mc_save_convar_list co" +
                "mmand";
            this.btnLoadConvars.Click += new System.EventHandler(this.btnLoadConvars_Click);
            // 
            // btnLoadValues
            // 
            this.btnLoadValues.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.btnLoadValues.Image = ((System.Drawing.Image)(resources.GetObject("btnLoadValues.Image")));
            this.btnLoadValues.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnLoadValues.Name = "btnLoadValues";
            this.btnLoadValues.Size = new System.Drawing.Size(68, 22);
            this.btnLoadValues.Text = "Load values";
            this.btnLoadValues.ToolTipText = "Click to load ConVar values from a .cfg file";
            this.btnLoadValues.Click += new System.EventHandler(this.btnLoadValues_Click);
            // 
            // btnSaveValues
            // 
            this.btnSaveValues.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.btnSaveValues.Image = ((System.Drawing.Image)(resources.GetObject("btnSaveValues.Image")));
            this.btnSaveValues.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnSaveValues.Name = "btnSaveValues";
            this.btnSaveValues.Size = new System.Drawing.Size(69, 22);
            this.btnSaveValues.Text = "Save values";
            this.btnSaveValues.ToolTipText = "Click to save all modified ConVars to a .cfg file that can be loaded in-game";
            this.btnSaveValues.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // btnReset
            // 
            this.btnReset.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.btnReset.Image = ((System.Drawing.Image)(resources.GetObject("btnReset.Image")));
            this.btnReset.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnReset.Name = "btnReset";
            this.btnReset.Size = new System.Drawing.Size(39, 22);
            this.btnReset.Text = "Reset";
            this.btnReset.ToolTipText = "Click to set all ConVars to their default values";
            this.btnReset.Click += new System.EventHandler(this.btnReset_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripLabel1
            // 
            this.toolStripLabel1.Name = "toolStripLabel1";
            this.toolStripLabel1.Size = new System.Drawing.Size(35, 22);
            this.toolStripLabel1.Text = "Filter:";
            // 
            // txtFilter
            // 
            this.txtFilter.Name = "txtFilter";
            this.txtFilter.Size = new System.Drawing.Size(120, 25);
            this.txtFilter.ToolTipText = "Type in this box to filter the convars shown below";
            this.txtFilter.TextChanged += new System.EventHandler(this.txtFilter_TextChanged_1);
            // 
            // conVarControl1
            // 
            this.conVarControl1.AllowCompress = true;
            this.conVarControl1.Compressed = false;
            this.conVarControl1.Location = new System.Drawing.Point(209, 6);
            this.conVarControl1.Name = "conVarControl1";
            this.conVarControl1.Size = new System.Drawing.Size(220, 150);
            this.conVarControl1.TabIndex = 8;
            this.conVarControl1.Var = null;
            // 
            // levelVarControl1
            // 
            this.levelVarControl1.AllowCompress = true;
            this.levelVarControl1.Compressed = false;
            this.levelVarControl1.Location = new System.Drawing.Point(209, 6);
            this.levelVarControl1.Name = "levelVarControl1";
            this.levelVarControl1.Size = new System.Drawing.Size(220, 255);
            this.levelVarControl1.TabIndex = 10;
            this.levelVarControl1.Var = null;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(437, 331);
            this.Controls.Add(this.toolStrip1);
            this.Controls.Add(this.lblExpand);
            this.Controls.Add(this.rbAll);
            this.Controls.Add(this.rbOne);
            this.Controls.Add(this.tabControl1);
            this.Name = "Form1";
            this.Text = "Convar Controller v1.77";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.tabControl1.ResumeLayout(false);
            this.tabPageList.ResumeLayout(false);
            this.tabPageList.PerformLayout();
            this.tabPageModules.ResumeLayout(false);
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckedListBox lstMiscConvars;
        private System.Windows.Forms.Label lblDesc;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPageList;
        private System.Windows.Forms.TabPage tabPageModules;
        private System.Windows.Forms.CheckedListBox lstAllModules;
        private ConVarControl conVarControl1;
        private System.Windows.Forms.FlowLayoutPanel moduleVarPanel;
        private LevelVarControl levelVarControl1;
        private System.Windows.Forms.RadioButton rbOne;
        private System.Windows.Forms.RadioButton rbAll;
        private System.Windows.Forms.Label lblExpand;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton btnLoadConvars;
        private System.Windows.Forms.ToolStripButton btnLoadValues;
        private System.Windows.Forms.ToolStripButton btnSaveValues;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripLabel toolStripLabel1;
        private System.Windows.Forms.ToolStripTextBox txtFilter;
        private System.Windows.Forms.ToolStripButton btnReset;
    }
}

