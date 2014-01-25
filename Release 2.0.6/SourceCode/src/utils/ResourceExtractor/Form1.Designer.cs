namespace ResourceExtractor
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
            this.btnBrowseSteamApps = new System.Windows.Forms.Button();
            this.lblSteamAppDir = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.lblExportDir = new System.Windows.Forms.Label();
            this.btnBrowseExport = new System.Windows.Forms.Button();
            this.btnExtract = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btnBrowseSteamApps
            // 
            this.btnBrowseSteamApps.Location = new System.Drawing.Point(302, 22);
            this.btnBrowseSteamApps.Name = "btnBrowseSteamApps";
            this.btnBrowseSteamApps.Size = new System.Drawing.Size(75, 23);
            this.btnBrowseSteamApps.TabIndex = 1;
            this.btnBrowseSteamApps.Text = "Browse";
            this.btnBrowseSteamApps.UseVisualStyleBackColor = true;
            this.btnBrowseSteamApps.Click += new System.EventHandler(this.btnBrowseSteamApps_Click);
            // 
            // lblSteamAppDir
            // 
            this.lblSteamAppDir.AutoSize = true;
            this.lblSteamAppDir.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblSteamAppDir.Location = new System.Drawing.Point(25, 27);
            this.lblSteamAppDir.Name = "lblSteamAppDir";
            this.lblSteamAppDir.Size = new System.Drawing.Size(94, 13);
            this.lblSteamAppDir.TabIndex = 0;
            this.lblSteamAppDir.Text = "<not specified>";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(107, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "SteamApps directory:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 60);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(83, 13);
            this.label2.TabIndex = 0;
            this.label2.Text = "Export directory:";
            // 
            // lblExportDir
            // 
            this.lblExportDir.AutoSize = true;
            this.lblExportDir.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblExportDir.Location = new System.Drawing.Point(25, 78);
            this.lblExportDir.Name = "lblExportDir";
            this.lblExportDir.Size = new System.Drawing.Size(94, 13);
            this.lblExportDir.TabIndex = 0;
            this.lblExportDir.Text = "<not specified>";
            // 
            // btnBrowseExport
            // 
            this.btnBrowseExport.Location = new System.Drawing.Point(302, 73);
            this.btnBrowseExport.Name = "btnBrowseExport";
            this.btnBrowseExport.Size = new System.Drawing.Size(75, 23);
            this.btnBrowseExport.TabIndex = 2;
            this.btnBrowseExport.Text = "Browse";
            this.btnBrowseExport.UseVisualStyleBackColor = true;
            this.btnBrowseExport.Click += new System.EventHandler(this.btnBrowseExport_Click);
            // 
            // btnExtract
            // 
            this.btnExtract.Enabled = false;
            this.btnExtract.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btnExtract.Location = new System.Drawing.Point(302, 122);
            this.btnExtract.Name = "btnExtract";
            this.btnExtract.Size = new System.Drawing.Size(75, 23);
            this.btnExtract.TabIndex = 3;
            this.btnExtract.Text = "Extract";
            this.btnExtract.UseVisualStyleBackColor = true;
            this.btnExtract.Click += new System.EventHandler(this.btnExtract_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(389, 157);
            this.Controls.Add(this.btnExtract);
            this.Controls.Add(this.btnBrowseExport);
            this.Controls.Add(this.btnBrowseSteamApps);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.lblExportDir);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lblSteamAppDir);
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(397, 191);
            this.MinimumSize = new System.Drawing.Size(397, 191);
            this.Name = "Form1";
            this.Text = "Resource Extractor";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnBrowseSteamApps;
        private System.Windows.Forms.Label lblSteamAppDir;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label lblExportDir;
        private System.Windows.Forms.Button btnBrowseExport;
        private System.Windows.Forms.Button btnExtract;
    }
}

