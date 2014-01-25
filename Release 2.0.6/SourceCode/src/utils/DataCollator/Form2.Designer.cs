namespace DataCollator
{
    partial class Form2
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
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabGraph = new System.Windows.Forms.TabPage();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.dateTimePicker1 = new System.Windows.Forms.DateTimePicker();
            this.lstDateOlderNewer = new System.Windows.Forms.ComboBox();
            this.checkNeedKillsOrDeaths = new System.Windows.Forms.CheckBox();
            this.checkDateRange = new System.Windows.Forms.CheckBox();
            this.checkNeedsDuration = new System.Windows.Forms.CheckBox();
            this.txtMinDuration = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.ddlSplitBy = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.checkFillIQR = new System.Windows.Forms.CheckBox();
            this.lstIndependentVariable = new System.Windows.Forms.ComboBox();
            this.lstDependentVariable = new System.Windows.Forms.ComboBox();
            this.checkMean = new System.Windows.Forms.CheckBox();
            this.label6 = new System.Windows.Forms.Label();
            this.checkNumPoints = new System.Windows.Forms.CheckBox();
            this.txtYmax = new System.Windows.Forms.TextBox();
            this.checkStandardDeviation = new System.Windows.Forms.CheckBox();
            this.txtYmin = new System.Windows.Forms.TextBox();
            this.checkMedian = new System.Windows.Forms.CheckBox();
            this.lnkSave = new System.Windows.Forms.LinkLabel();
            this.checkQuartiles = new System.Windows.Forms.CheckBox();
            this.btnGo = new System.Windows.Forms.Button();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.checkLegend = new System.Windows.Forms.CheckBox();
            this.plotSurface2D1 = new NPlot.Windows.PlotSurface2D();
            this.tabQuery = new System.Windows.Forms.TabPage();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.txtQuery = new System.Windows.Forms.TextBox();
            this.btnDownload = new System.Windows.Forms.Button();
            this.btnUpdate = new System.Windows.Forms.Button();
            this.btnQuery = new System.Windows.Forms.Button();
            this.dataGridView1 = new System.Windows.Forms.DataGridView();
            this.chkNotCheats = new System.Windows.Forms.CheckBox();
            this.chkOnlyDefaults = new System.Windows.Forms.CheckBox();
            this.popLstMap = new DataCollator.PopupSelector();
            this.popLstVersion = new DataCollator.PopupSelector();
            this.popLstGameMode = new DataCollator.PopupSelector();
            this.popLstModules = new DataCollator.PopupSelector();
            this.tabControl1.SuspendLayout();
            this.tabGraph.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tabQuery.SuspendLayout();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabGraph);
            this.tabControl1.Controls.Add(this.tabQuery);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(846, 606);
            this.tabControl1.TabIndex = 0;
            // 
            // tabGraph
            // 
            this.tabGraph.Controls.Add(this.groupBox2);
            this.tabGraph.Controls.Add(this.groupBox1);
            this.tabGraph.Controls.Add(this.plotSurface2D1);
            this.tabGraph.Location = new System.Drawing.Point(4, 22);
            this.tabGraph.Name = "tabGraph";
            this.tabGraph.Padding = new System.Windows.Forms.Padding(3);
            this.tabGraph.Size = new System.Drawing.Size(838, 580);
            this.tabGraph.TabIndex = 1;
            this.tabGraph.Text = "Graph";
            this.tabGraph.UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.popLstMap);
            this.groupBox2.Controls.Add(this.ddlSplitBy);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Controls.Add(this.popLstVersion);
            this.groupBox2.Controls.Add(this.popLstGameMode);
            this.groupBox2.Controls.Add(this.popLstModules);
            this.groupBox2.Controls.Add(this.dateTimePicker1);
            this.groupBox2.Controls.Add(this.lstDateOlderNewer);
            this.groupBox2.Controls.Add(this.chkOnlyDefaults);
            this.groupBox2.Controls.Add(this.chkNotCheats);
            this.groupBox2.Controls.Add(this.checkNeedKillsOrDeaths);
            this.groupBox2.Controls.Add(this.checkDateRange);
            this.groupBox2.Controls.Add(this.checkNeedsDuration);
            this.groupBox2.Controls.Add(this.txtMinDuration);
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Location = new System.Drawing.Point(339, 0);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(494, 119);
            this.groupBox2.TabIndex = 17;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Filter";
            // 
            // dateTimePicker1
            // 
            this.dateTimePicker1.Location = new System.Drawing.Point(340, 42);
            this.dateTimePicker1.Name = "dateTimePicker1";
            this.dateTimePicker1.Size = new System.Drawing.Size(145, 20);
            this.dateTimePicker1.TabIndex = 15;
            // 
            // lstDateOlderNewer
            // 
            this.lstDateOlderNewer.FormattingEnabled = true;
            this.lstDateOlderNewer.Items.AddRange(new object[] {
            "newer than",
            "older than"});
            this.lstDateOlderNewer.Location = new System.Drawing.Point(386, 15);
            this.lstDateOlderNewer.Name = "lstDateOlderNewer";
            this.lstDateOlderNewer.Size = new System.Drawing.Size(99, 21);
            this.lstDateOlderNewer.TabIndex = 14;
            // 
            // checkNeedKillsOrDeaths
            // 
            this.checkNeedKillsOrDeaths.AutoSize = true;
            this.checkNeedKillsOrDeaths.Checked = true;
            this.checkNeedKillsOrDeaths.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkNeedKillsOrDeaths.Location = new System.Drawing.Point(96, 19);
            this.checkNeedKillsOrDeaths.Name = "checkNeedKillsOrDeaths";
            this.checkNeedKillsOrDeaths.Size = new System.Drawing.Size(174, 17);
            this.checkNeedKillsOrDeaths.TabIndex = 11;
            this.checkNeedKillsOrDeaths.Text = "Only records with exp or deaths";
            this.checkNeedKillsOrDeaths.UseVisualStyleBackColor = true;
            // 
            // checkDateRange
            // 
            this.checkDateRange.AutoSize = true;
            this.checkDateRange.Location = new System.Drawing.Point(302, 17);
            this.checkDateRange.Name = "checkDateRange";
            this.checkDateRange.Size = new System.Drawing.Size(85, 17);
            this.checkDateRange.TabIndex = 11;
            this.checkDateRange.Text = "Only records";
            this.checkDateRange.UseVisualStyleBackColor = true;
            // 
            // checkNeedsDuration
            // 
            this.checkNeedsDuration.AutoSize = true;
            this.checkNeedsDuration.Checked = true;
            this.checkNeedsDuration.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkNeedsDuration.Location = new System.Drawing.Point(96, 42);
            this.checkNeedsDuration.Name = "checkNeedsDuration";
            this.checkNeedsDuration.Size = new System.Drawing.Size(157, 17);
            this.checkNeedsDuration.TabIndex = 11;
            this.checkNeedsDuration.Text = "Only records with duration >";
            this.checkNeedsDuration.UseVisualStyleBackColor = true;
            // 
            // txtMinDuration
            // 
            this.txtMinDuration.Location = new System.Drawing.Point(253, 40);
            this.txtMinDuration.Name = "txtMinDuration";
            this.txtMinDuration.Size = new System.Drawing.Size(26, 20);
            this.txtMinDuration.TabIndex = 12;
            this.txtMinDuration.Text = "30";
            this.txtMinDuration.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(281, 43);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(12, 13);
            this.label8.TabIndex = 13;
            this.label8.Text = "s";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.checkFillIQR);
            this.groupBox1.Controls.Add(this.lstIndependentVariable);
            this.groupBox1.Controls.Add(this.lstDependentVariable);
            this.groupBox1.Controls.Add(this.checkMean);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.checkNumPoints);
            this.groupBox1.Controls.Add(this.txtYmax);
            this.groupBox1.Controls.Add(this.checkStandardDeviation);
            this.groupBox1.Controls.Add(this.txtYmin);
            this.groupBox1.Controls.Add(this.checkMedian);
            this.groupBox1.Controls.Add(this.lnkSave);
            this.groupBox1.Controls.Add(this.checkQuartiles);
            this.groupBox1.Controls.Add(this.btnGo);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.checkLegend);
            this.groupBox1.Location = new System.Drawing.Point(0, 0);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(333, 119);
            this.groupBox1.TabIndex = 16;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Plot";
            // 
            // ddlSplitBy
            // 
            this.ddlSplitBy.FormattingEnabled = true;
            this.ddlSplitBy.Items.AddRange(new object[] {
            "[Do not split]",
            "Module",
            "Game Mode",
            "Map",
            "Version"});
            this.ddlSplitBy.Location = new System.Drawing.Point(378, 93);
            this.ddlSplitBy.Name = "ddlSplitBy";
            this.ddlSplitBy.Size = new System.Drawing.Size(107, 21);
            this.ddlSplitBy.TabIndex = 14;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(304, 96);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(68, 13);
            this.label1.TabIndex = 13;
            this.label1.Text = "Split data by:";
            // 
            // checkFillIQR
            // 
            this.checkFillIQR.AutoSize = true;
            this.checkFillIQR.Checked = true;
            this.checkFillIQR.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkFillIQR.Enabled = false;
            this.checkFillIQR.Location = new System.Drawing.Point(200, 67);
            this.checkFillIQR.Name = "checkFillIQR";
            this.checkFillIQR.Size = new System.Drawing.Size(60, 17);
            this.checkFillIQR.TabIndex = 1;
            this.checkFillIQR.Text = "Fill IQR";
            this.checkFillIQR.UseVisualStyleBackColor = true;
            // 
            // lstIndependentVariable
            // 
            this.lstIndependentVariable.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.lstIndependentVariable.FormattingEnabled = true;
            this.lstIndependentVariable.Location = new System.Drawing.Point(181, 13);
            this.lstIndependentVariable.Name = "lstIndependentVariable";
            this.lstIndependentVariable.Size = new System.Drawing.Size(91, 21);
            this.lstIndependentVariable.TabIndex = 0;
            this.lstIndependentVariable.SelectedIndexChanged += new System.EventHandler(this.lstIndependentVariable_SelectedIndexChanged);
            // 
            // lstDependentVariable
            // 
            this.lstDependentVariable.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.lstDependentVariable.FormattingEnabled = true;
            this.lstDependentVariable.Location = new System.Drawing.Point(9, 13);
            this.lstDependentVariable.Name = "lstDependentVariable";
            this.lstDependentVariable.Size = new System.Drawing.Size(150, 21);
            this.lstDependentVariable.TabIndex = 0;
            // 
            // checkMean
            // 
            this.checkMean.AutoSize = true;
            this.checkMean.Checked = true;
            this.checkMean.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkMean.Location = new System.Drawing.Point(53, 43);
            this.checkMean.Name = "checkMean";
            this.checkMean.Size = new System.Drawing.Size(53, 17);
            this.checkMean.TabIndex = 1;
            this.checkMean.Text = "Mean";
            this.checkMean.UseVisualStyleBackColor = true;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(4, 96);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(47, 13);
            this.label6.TabIndex = 5;
            this.label6.Text = "Y range:";
            // 
            // checkNumPoints
            // 
            this.checkNumPoints.AutoSize = true;
            this.checkNumPoints.Location = new System.Drawing.Point(53, 67);
            this.checkNumPoints.Name = "checkNumPoints";
            this.checkNumPoints.Size = new System.Drawing.Size(74, 17);
            this.checkNumPoints.TabIndex = 1;
            this.checkNumPoints.Text = "No. points";
            this.checkNumPoints.UseVisualStyleBackColor = true;
            // 
            // txtYmax
            // 
            this.txtYmax.Location = new System.Drawing.Point(111, 93);
            this.txtYmax.Name = "txtYmax";
            this.txtYmax.Size = new System.Drawing.Size(52, 20);
            this.txtYmax.TabIndex = 4;
            this.txtYmax.TextChanged += new System.EventHandler(this.txtYmax_TextChanged);
            // 
            // checkStandardDeviation
            // 
            this.checkStandardDeviation.AutoSize = true;
            this.checkStandardDeviation.Location = new System.Drawing.Point(128, 67);
            this.checkStandardDeviation.Name = "checkStandardDeviation";
            this.checkStandardDeviation.Size = new System.Drawing.Size(71, 17);
            this.checkStandardDeviation.TabIndex = 1;
            this.checkStandardDeviation.Text = "Std. Dev.";
            this.checkStandardDeviation.UseVisualStyleBackColor = true;
            // 
            // txtYmin
            // 
            this.txtYmin.Location = new System.Drawing.Point(53, 92);
            this.txtYmin.Name = "txtYmin";
            this.txtYmin.Size = new System.Drawing.Size(52, 20);
            this.txtYmin.TabIndex = 4;
            this.txtYmin.TextChanged += new System.EventHandler(this.txtYmin_TextChanged);
            // 
            // checkMedian
            // 
            this.checkMedian.AutoSize = true;
            this.checkMedian.Location = new System.Drawing.Point(107, 43);
            this.checkMedian.Name = "checkMedian";
            this.checkMedian.Size = new System.Drawing.Size(61, 17);
            this.checkMedian.TabIndex = 1;
            this.checkMedian.Text = "Median";
            this.checkMedian.UseVisualStyleBackColor = true;
            // 
            // lnkSave
            // 
            this.lnkSave.AutoSize = true;
            this.lnkSave.Location = new System.Drawing.Point(263, 68);
            this.lnkSave.Name = "lnkSave";
            this.lnkSave.Size = new System.Drawing.Size(63, 13);
            this.lnkSave.TabIndex = 7;
            this.lnkSave.TabStop = true;
            this.lnkSave.Text = "Save image";
            this.lnkSave.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.lnkSave.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.lnkSave_LinkClicked);
            // 
            // checkQuartiles
            // 
            this.checkQuartiles.AutoSize = true;
            this.checkQuartiles.Location = new System.Drawing.Point(169, 43);
            this.checkQuartiles.Name = "checkQuartiles";
            this.checkQuartiles.Size = new System.Drawing.Size(67, 17);
            this.checkQuartiles.TabIndex = 1;
            this.checkQuartiles.Text = "Quartiles";
            this.checkQuartiles.UseVisualStyleBackColor = true;
            this.checkQuartiles.CheckedChanged += new System.EventHandler(this.checkQuartiles_CheckedChanged);
            // 
            // btnGo
            // 
            this.btnGo.Location = new System.Drawing.Point(278, 13);
            this.btnGo.Name = "btnGo";
            this.btnGo.Size = new System.Drawing.Size(48, 30);
            this.btnGo.TabIndex = 3;
            this.btnGo.Text = "Go";
            this.btnGo.UseVisualStyleBackColor = true;
            this.btnGo.Click += new System.EventHandler(this.btnGo_Click);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(6, 68);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(46, 13);
            this.label5.TabIndex = 8;
            this.label5.Text = "Options:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 44);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(37, 13);
            this.label4.TabIndex = 8;
            this.label4.Text = "Show:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(102, 95);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(12, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "/";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(161, 16);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(18, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "vs";
            // 
            // checkLegend
            // 
            this.checkLegend.AutoSize = true;
            this.checkLegend.Checked = true;
            this.checkLegend.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkLegend.Enabled = false;
            this.checkLegend.Location = new System.Drawing.Point(237, 43);
            this.checkLegend.Name = "checkLegend";
            this.checkLegend.Size = new System.Drawing.Size(62, 17);
            this.checkLegend.TabIndex = 1;
            this.checkLegend.Text = "Legend";
            this.checkLegend.UseVisualStyleBackColor = true;
            // 
            // plotSurface2D1
            // 
            this.plotSurface2D1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.plotSurface2D1.AutoScaleAutoGeneratedAxes = false;
            this.plotSurface2D1.AutoScaleTitle = false;
            this.plotSurface2D1.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.plotSurface2D1.DateTimeToolTip = false;
            this.plotSurface2D1.Legend = null;
            this.plotSurface2D1.LegendZOrder = -1;
            this.plotSurface2D1.Location = new System.Drawing.Point(0, 119);
            this.plotSurface2D1.Name = "plotSurface2D1";
            this.plotSurface2D1.RightMenu = null;
            this.plotSurface2D1.ShowCoordinates = true;
            this.plotSurface2D1.Size = new System.Drawing.Size(838, 461);
            this.plotSurface2D1.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            this.plotSurface2D1.TabIndex = 9;
            this.plotSurface2D1.Text = "plotSurface2D1";
            this.plotSurface2D1.Title = "";
            this.plotSurface2D1.TitleFont = new System.Drawing.Font("Arial", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel);
            this.plotSurface2D1.XAxis1 = null;
            this.plotSurface2D1.XAxis2 = null;
            this.plotSurface2D1.YAxis1 = null;
            this.plotSurface2D1.YAxis2 = null;
            // 
            // tabQuery
            // 
            this.tabQuery.Controls.Add(this.splitContainer1);
            this.tabQuery.Location = new System.Drawing.Point(4, 22);
            this.tabQuery.Name = "tabQuery";
            this.tabQuery.Padding = new System.Windows.Forms.Padding(3);
            this.tabQuery.Size = new System.Drawing.Size(838, 580);
            this.tabQuery.TabIndex = 0;
            this.tabQuery.Text = "SQL Query";
            this.tabQuery.UseVisualStyleBackColor = true;
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(3, 3);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.txtQuery);
            this.splitContainer1.Panel1.Controls.Add(this.btnDownload);
            this.splitContainer1.Panel1.Controls.Add(this.btnUpdate);
            this.splitContainer1.Panel1.Controls.Add(this.btnQuery);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.dataGridView1);
            this.splitContainer1.Size = new System.Drawing.Size(832, 574);
            this.splitContainer1.SplitterDistance = 139;
            this.splitContainer1.TabIndex = 3;
            // 
            // txtQuery
            // 
            this.txtQuery.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.txtQuery.HideSelection = false;
            this.txtQuery.Location = new System.Drawing.Point(0, 0);
            this.txtQuery.Multiline = true;
            this.txtQuery.Name = "txtQuery";
            this.txtQuery.Size = new System.Drawing.Size(769, 63);
            this.txtQuery.TabIndex = 3;
            // 
            // btnDownload
            // 
            this.btnDownload.Location = new System.Drawing.Point(12, 12);
            this.btnDownload.Name = "btnDownload";
            this.btnDownload.Size = new System.Drawing.Size(75, 23);
            this.btnDownload.TabIndex = 0;
            this.btnDownload.Text = "Download";
            this.btnDownload.UseVisualStyleBackColor = true;
            // 
            // btnUpdate
            // 
            this.btnUpdate.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnUpdate.Location = new System.Drawing.Point(775, 31);
            this.btnUpdate.Name = "btnUpdate";
            this.btnUpdate.Size = new System.Drawing.Size(57, 32);
            this.btnUpdate.TabIndex = 5;
            this.btnUpdate.Text = "Update";
            this.btnUpdate.UseVisualStyleBackColor = true;
            this.btnUpdate.Click += new System.EventHandler(this.btnUpdate_Click);
            // 
            // btnQuery
            // 
            this.btnQuery.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnQuery.Location = new System.Drawing.Point(775, 0);
            this.btnQuery.Name = "btnQuery";
            this.btnQuery.Size = new System.Drawing.Size(57, 32);
            this.btnQuery.TabIndex = 4;
            this.btnQuery.Text = "Query";
            this.btnQuery.UseVisualStyleBackColor = true;
            this.btnQuery.Click += new System.EventHandler(this.btnQuery_Click);
            // 
            // dataGridView1
            // 
            this.dataGridView1.AllowUserToAddRows = false;
            this.dataGridView1.AllowUserToDeleteRows = false;
            this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dataGridView1.Location = new System.Drawing.Point(0, 0);
            this.dataGridView1.Name = "dataGridView1";
            this.dataGridView1.ReadOnly = true;
            this.dataGridView1.Size = new System.Drawing.Size(832, 431);
            this.dataGridView1.TabIndex = 0;
            // 
            // chkNotCheats
            // 
            this.chkNotCheats.AutoSize = true;
            this.chkNotCheats.Checked = true;
            this.chkNotCheats.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkNotCheats.Location = new System.Drawing.Point(96, 65);
            this.chkNotCheats.Name = "chkNotCheats";
            this.chkNotCheats.Size = new System.Drawing.Size(180, 17);
            this.chkNotCheats.TabIndex = 11;
            this.chkNotCheats.Text = "Only games with cheats disabled";
            this.chkNotCheats.UseVisualStyleBackColor = true;
            // 
            // chkOnlyDefaults
            // 
            this.chkOnlyDefaults.AutoSize = true;
            this.chkOnlyDefaults.Checked = true;
            this.chkOnlyDefaults.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkOnlyDefaults.Location = new System.Drawing.Point(96, 88);
            this.chkOnlyDefaults.Name = "chkOnlyDefaults";
            this.chkOnlyDefaults.Size = new System.Drawing.Size(183, 17);
            this.chkOnlyDefaults.TabIndex = 11;
            this.chkOnlyDefaults.Text = "Only games using default settings";
            this.chkOnlyDefaults.UseVisualStyleBackColor = true;
            // 
            // popLstMap
            // 
            this.popLstMap.BackColor = System.Drawing.Color.Transparent;
            this.popLstMap.ButtonText = "Map";
            this.popLstMap.Location = new System.Drawing.Point(6, 90);
            this.popLstMap.Name = "popLstMap";
            this.popLstMap.Size = new System.Drawing.Size(80, 23);
            this.popLstMap.TabIndex = 19;
            // 
            // popLstVersion
            // 
            this.popLstVersion.BackColor = System.Drawing.Color.Transparent;
            this.popLstVersion.ButtonText = "Version";
            this.popLstVersion.Location = new System.Drawing.Point(6, 65);
            this.popLstVersion.Name = "popLstVersion";
            this.popLstVersion.Size = new System.Drawing.Size(80, 23);
            this.popLstVersion.TabIndex = 18;
            // 
            // popLstGameMode
            // 
            this.popLstGameMode.BackColor = System.Drawing.Color.Transparent;
            this.popLstGameMode.ButtonText = "Game Mode";
            this.popLstGameMode.Location = new System.Drawing.Point(6, 40);
            this.popLstGameMode.Name = "popLstGameMode";
            this.popLstGameMode.Size = new System.Drawing.Size(80, 23);
            this.popLstGameMode.TabIndex = 18;
            // 
            // popLstModules
            // 
            this.popLstModules.BackColor = System.Drawing.Color.Transparent;
            this.popLstModules.ButtonText = "Module";
            this.popLstModules.Enabled = false;
            this.popLstModules.Location = new System.Drawing.Point(6, 15);
            this.popLstModules.Name = "popLstModules";
            this.popLstModules.Size = new System.Drawing.Size(80, 23);
            this.popLstModules.TabIndex = 18;
            // 
            // Form2
            // 
            this.AcceptButton = this.btnGo;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(846, 606);
            this.Controls.Add(this.tabControl1);
            this.Name = "Form2";
            this.Text = "Data Collator";
            this.tabControl1.ResumeLayout(false);
            this.tabGraph.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.tabQuery.ResumeLayout(false);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel1.PerformLayout();
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabQuery;
        private System.Windows.Forms.TabPage tabGraph;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.TextBox txtQuery;
        private System.Windows.Forms.Button btnDownload;
        private System.Windows.Forms.Button btnUpdate;
        private System.Windows.Forms.Button btnQuery;
        private System.Windows.Forms.DataGridView dataGridView1;
        private System.Windows.Forms.ComboBox lstDependentVariable;
        private System.Windows.Forms.ComboBox lstIndependentVariable;
        private System.Windows.Forms.CheckBox checkQuartiles;
        private System.Windows.Forms.CheckBox checkMedian;
        private System.Windows.Forms.CheckBox checkFillIQR;
        private System.Windows.Forms.CheckBox checkNumPoints;
        private System.Windows.Forms.CheckBox checkMean;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnGo;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.CheckBox checkStandardDeviation;
        private NPlot.Windows.PlotSurface2D plotSurface2D1;
        private System.Windows.Forms.TextBox txtYmin;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox txtYmax;
        private System.Windows.Forms.LinkLabel lnkSave;
        private System.Windows.Forms.CheckBox checkNeedKillsOrDeaths;
        private System.Windows.Forms.CheckBox checkNeedsDuration;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox txtMinDuration;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox checkLegend;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ComboBox ddlSplitBy;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox lstDateOlderNewer;
        private System.Windows.Forms.CheckBox checkDateRange;
        private System.Windows.Forms.DateTimePicker dateTimePicker1;
        private System.Windows.Forms.Label label4;
        private PopupSelector popLstModules;
        private PopupSelector popLstGameMode;
        private PopupSelector popLstVersion;
        private System.Windows.Forms.CheckBox chkOnlyDefaults;
        private System.Windows.Forms.CheckBox chkNotCheats;
        private PopupSelector popLstMap;
    }
}