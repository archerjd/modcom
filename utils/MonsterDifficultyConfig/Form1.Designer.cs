namespace MonsterDifficultyConfig
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
            this.lstMonsterTypes = new System.Windows.Forms.ListBox();
            this.btnRefresh = new System.Windows.Forms.Button();
            this.btnSave = new System.Windows.Forms.Button();
            this.lstFields = new System.Windows.Forms.ListBox();
            this.btnScaleAll = new System.Windows.Forms.Button();
            this.btnAddAll = new System.Windows.Forms.Button();
            this.btnAddLevel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.graphPanel1 = new MonsterDifficultyConfig.BarGraphPanel();
            this.SuspendLayout();
            // 
            // lstMonsterTypes
            // 
            this.lstMonsterTypes.FormattingEnabled = true;
            this.lstMonsterTypes.Location = new System.Drawing.Point(2, 3);
            this.lstMonsterTypes.Name = "lstMonsterTypes";
            this.lstMonsterTypes.Size = new System.Drawing.Size(144, 134);
            this.lstMonsterTypes.Sorted = true;
            this.lstMonsterTypes.TabIndex = 0;
            this.lstMonsterTypes.SelectedIndexChanged += new System.EventHandler(this.lstMonsterTypes_SelectedIndexChanged);
            // 
            // btnRefresh
            // 
            this.btnRefresh.Location = new System.Drawing.Point(152, 3);
            this.btnRefresh.Name = "btnRefresh";
            this.btnRefresh.Size = new System.Drawing.Size(74, 23);
            this.btnRefresh.TabIndex = 1;
            this.btnRefresh.Text = "Refresh list";
            this.btnRefresh.UseVisualStyleBackColor = true;
            this.btnRefresh.Click += new System.EventHandler(this.btnRefresh_Click);
            // 
            // btnSave
            // 
            this.btnSave.Enabled = false;
            this.btnSave.Location = new System.Drawing.Point(229, 3);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(74, 23);
            this.btnSave.TabIndex = 2;
            this.btnSave.Text = "Save Changes";
            this.btnSave.UseVisualStyleBackColor = true;
            this.btnSave.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // lstFields
            // 
            this.lstFields.FormattingEnabled = true;
            this.lstFields.Location = new System.Drawing.Point(152, 29);
            this.lstFields.Name = "lstFields";
            this.lstFields.Size = new System.Drawing.Size(151, 108);
            this.lstFields.TabIndex = 3;
            this.lstFields.SelectedIndexChanged += new System.EventHandler(this.lstFields_SelectedIndexChanged);
            // 
            // btnScaleAll
            // 
            this.btnScaleAll.Location = new System.Drawing.Point(158, 287);
            this.btnScaleAll.Name = "btnScaleAll";
            this.btnScaleAll.Size = new System.Drawing.Size(72, 23);
            this.btnScaleAll.TabIndex = 11;
            this.btnScaleAll.Text = "Scale all";
            this.btnScaleAll.UseVisualStyleBackColor = true;
            this.btnScaleAll.Click += new System.EventHandler(this.btnScaleAll_Click);
            // 
            // btnAddAll
            // 
            this.btnAddAll.Location = new System.Drawing.Point(80, 287);
            this.btnAddAll.Name = "btnAddAll";
            this.btnAddAll.Size = new System.Drawing.Size(72, 23);
            this.btnAddAll.TabIndex = 11;
            this.btnAddAll.Text = "Add to all";
            this.btnAddAll.UseVisualStyleBackColor = true;
            this.btnAddAll.Click += new System.EventHandler(this.btnAddAll_Click);
            // 
            // btnAddLevel
            // 
            this.btnAddLevel.Location = new System.Drawing.Point(2, 287);
            this.btnAddLevel.Name = "btnAddLevel";
            this.btnAddLevel.Size = new System.Drawing.Size(72, 23);
            this.btnAddLevel.TabIndex = 11;
            this.btnAddLevel.Text = "Set range";
            this.btnAddLevel.UseVisualStyleBackColor = true;
            this.btnAddLevel.Click += new System.EventHandler(this.btnAddLevel_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(-1, 140);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(297, 13);
            this.label1.TabIndex = 13;
            this.label1.Text = "Click to edit a single value, or use the buttons below to edit all";
            // 
            // graphPanel1
            // 
            this.graphPanel1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.graphPanel1.Location = new System.Drawing.Point(2, 143);
            this.graphPanel1.Name = "graphPanel1";
            this.graphPanel1.Size = new System.Drawing.Size(348, 138);
            this.graphPanel1.TabIndex = 12;
            this.graphPanel1.MouseClick += new System.Windows.Forms.MouseEventHandler(this.graphPanel1_MouseClick);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(352, 319);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.graphPanel1);
            this.Controls.Add(this.btnAddLevel);
            this.Controls.Add(this.btnAddAll);
            this.Controls.Add(this.btnScaleAll);
            this.Controls.Add(this.lstFields);
            this.Controls.Add(this.btnSave);
            this.Controls.Add(this.btnRefresh);
            this.Controls.Add(this.lstMonsterTypes);
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(360, 353);
            this.MinimumSize = new System.Drawing.Size(360, 353);
            this.Name = "Form1";
            this.Text = "Monster Config";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox lstMonsterTypes;
        private System.Windows.Forms.Button btnRefresh;
        private System.Windows.Forms.Button btnSave;
        private System.Windows.Forms.ListBox lstFields;
        private System.Windows.Forms.Button btnScaleAll;
        private System.Windows.Forms.Button btnAddAll;
        private System.Windows.Forms.Button btnAddLevel;
        private BarGraphPanel graphPanel1;
        private System.Windows.Forms.Label label1;

    }
}

