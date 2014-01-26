using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ConvarController
{
    partial class ConVarControl : UserControl, IVarControl
    {
        public ConVarControl()
        {
            InitializeComponent();
            Var = null;
            groupBox1.Click += new EventHandler(groupBox1_Click);
            AllowCompress = true;
        }

        public bool AllowCompress { get; set; }

        void groupBox1_Click(object sender, EventArgs e)
        {
            if (Compressed)
                Compressed = false;
            else if (AllowCompress)
                Compressed = true;
        }

        private bool compressed = false;
        private const int minHeight = 14, partialHeight = 65, borderHeight = 8;
        public bool Compressed
        {
            get { return compressed; }
            set
            {
                compressed = value;
                if (compressed)
                {
                    this.Size = new Size(this.Size.Width, minHeight);
                }
                else
                {
                    if ( lblDesc.Text == string.Empty )
                        this.Size = new Size(this.Size.Width, partialHeight + borderHeight);
                    else
                        this.Size = new Size(this.Size.Width, lblDesc.Top + borderHeight + lblDesc.Height);
                    if (Expanded != null)
                        Expanded(this, EventArgs.Empty);
                }
            }
        }

        public event EventHandler Changed, Expanded;

        private ConVar var;
        public ConVar Var
        {
            get { return var; }
            set
            {
                var = value;
                if (var == null)
                {
                    groupBox1.Visible = false;
                    return;
                }
                CheckDefault();
                lblDesc.Text = var.HelpText;
                txtValue.Text = var.Value;
                txtDefault.Text = var.Default;
                txtValue.BackColor = SystemColors.Window;

                groupBox1.Visible = true;
                Compressed = false; // recalculate 
            }
        }

        private void CheckDefault()
        {
            if (var != null)
                if (txtValue.Text == var.Default)
                    groupBox1.Text = var.Name;
                else
                    groupBox1.Text = var.Name + " *";
        }

        private bool Validate(TextBox sender)
        {
            float f;
            if (!float.TryParse(sender.Text, out f))
            {
                sender.BackColor = Color.Peru;
                return false;
            }

            sender.BackColor = SystemColors.Window;
            CheckDefault();
            return true;
        }

        private void txtDefault_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            txtValue.Text = txtDefault.Text;
        }

        private void txtValue_TextChanged(object sender, EventArgs e)
        {
            if (Validate(txtValue) && var != null)
            {
                var.Value = txtValue.Text;
                if (Changed != null)
                    Changed(this, EventArgs.Empty);
            }
        }
    }
}
