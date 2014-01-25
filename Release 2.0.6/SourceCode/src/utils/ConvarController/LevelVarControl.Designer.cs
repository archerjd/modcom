namespace ConvarController
{
    partial class LevelVarControl
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.levelVarGraph = new ConvarController.BarGraphPanel();
            this.lblDesc = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.txtDefaultPower = new System.Windows.Forms.TextBox();
            this.txtPower = new System.Windows.Forms.TextBox();
            this.txtDefaultScale = new System.Windows.Forms.TextBox();
            this.txtDefaultBase = new System.Windows.Forms.TextBox();
            this.txtScale = new System.Windows.Forms.TextBox();
            this.txtBase = new System.Windows.Forms.TextBox();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.levelVarGraph);
            this.groupBox1.Controls.Add(this.lblDesc);
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.txtDefaultPower);
            this.groupBox1.Controls.Add(this.txtPower);
            this.groupBox1.Controls.Add(this.txtDefaultScale);
            this.groupBox1.Controls.Add(this.txtDefaultBase);
            this.groupBox1.Controls.Add(this.txtScale);
            this.groupBox1.Controls.Add(this.txtBase);
            this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox1.Location = new System.Drawing.Point(0, 0);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(220, 255);
            this.groupBox1.TabIndex = 13;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "varname";
            // 
            // levelVarGraph
            // 
            this.levelVarGraph.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.levelVarGraph.Location = new System.Drawing.Point(3, 141);
            this.levelVarGraph.Name = "levelVarGraph";
            this.levelVarGraph.Size = new System.Drawing.Size(214, 111);
            this.levelVarGraph.TabIndex = 15;
            // 
            // lblDesc
            // 
            this.lblDesc.AutoSize = true;
            this.lblDesc.Location = new System.Drawing.Point(6, 90);
            this.lblDesc.MaximumSize = new System.Drawing.Size(215, 50);
            this.lblDesc.Name = "lblDesc";
            this.lblDesc.Size = new System.Drawing.Size(149, 13);
            this.lblDesc.TabIndex = 14;
            this.lblDesc.Text = "Variable description goes here";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(6, 61);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(44, 13);
            this.label8.TabIndex = 13;
            this.label8.Text = "Default:";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(6, 35);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(44, 13);
            this.label7.TabIndex = 13;
            this.label7.Text = "Current:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(167, 16);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(37, 13);
            this.label6.TabIndex = 12;
            this.label6.Text = "Power";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(115, 16);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(34, 13);
            this.label5.TabIndex = 12;
            this.label5.Text = "Scale";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(64, 16);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(31, 13);
            this.label1.TabIndex = 12;
            this.label1.Text = "Base";
            // 
            // txtDefaultPower
            // 
            this.txtDefaultPower.Location = new System.Drawing.Point(162, 58);
            this.txtDefaultPower.Name = "txtDefaultPower";
            this.txtDefaultPower.ReadOnly = true;
            this.txtDefaultPower.Size = new System.Drawing.Size(47, 20);
            this.txtDefaultPower.TabIndex = 8;
            this.txtDefaultPower.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.txtDefaultPower.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.txtDefaultPower_MouseDoubleClick);
            // 
            // txtPower
            // 
            this.txtPower.Location = new System.Drawing.Point(162, 32);
            this.txtPower.Name = "txtPower";
            this.txtPower.Size = new System.Drawing.Size(47, 20);
            this.txtPower.TabIndex = 8;
            this.txtPower.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.txtPower.TextChanged += new System.EventHandler(this.txtPower_TextChanged);
            // 
            // txtDefaultScale
            // 
            this.txtDefaultScale.Location = new System.Drawing.Point(109, 58);
            this.txtDefaultScale.Name = "txtDefaultScale";
            this.txtDefaultScale.ReadOnly = true;
            this.txtDefaultScale.Size = new System.Drawing.Size(47, 20);
            this.txtDefaultScale.TabIndex = 8;
            this.txtDefaultScale.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.txtDefaultScale.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.txtDefaultScale_MouseDoubleClick);
            // 
            // txtDefaultBase
            // 
            this.txtDefaultBase.Location = new System.Drawing.Point(56, 58);
            this.txtDefaultBase.Name = "txtDefaultBase";
            this.txtDefaultBase.ReadOnly = true;
            this.txtDefaultBase.Size = new System.Drawing.Size(47, 20);
            this.txtDefaultBase.TabIndex = 8;
            this.txtDefaultBase.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.txtDefaultBase.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.txtDefaultBase_MouseDoubleClick);
            // 
            // txtScale
            // 
            this.txtScale.Location = new System.Drawing.Point(109, 32);
            this.txtScale.Name = "txtScale";
            this.txtScale.Size = new System.Drawing.Size(47, 20);
            this.txtScale.TabIndex = 8;
            this.txtScale.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.txtScale.TextChanged += new System.EventHandler(this.txtScale_TextChanged);
            // 
            // txtBase
            // 
            this.txtBase.Location = new System.Drawing.Point(56, 32);
            this.txtBase.Name = "txtBase";
            this.txtBase.Size = new System.Drawing.Size(47, 20);
            this.txtBase.TabIndex = 8;
            this.txtBase.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.txtBase.TextChanged += new System.EventHandler(this.txtBase_TextChanged);
            // 
            // LevelVarControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBox1);
            this.Name = "LevelVarControl";
            this.Size = new System.Drawing.Size(220, 255);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtPower;
        private System.Windows.Forms.TextBox txtDefaultScale;
        private System.Windows.Forms.TextBox txtDefaultBase;
        private System.Windows.Forms.TextBox txtBase;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox txtScale;
        private System.Windows.Forms.TextBox txtDefaultPower;
        private System.Windows.Forms.Label lblDesc;
        private BarGraphPanel levelVarGraph;
    }
}
