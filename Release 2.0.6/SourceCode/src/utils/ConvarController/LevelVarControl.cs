using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ConvarController
{
    partial class LevelVarControl : UserControl, IVarControl
    {
        public LevelVarControl()
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
        private const int minHeight = 14, fullHeight = 255, borderHeight = 8;
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
                    this.Size = new Size(this.Size.Width, fullHeight);

                    if (lblDesc.Text == string.Empty)
                        levelVarGraph.Top = lblDesc.Top;
                    else
                        levelVarGraph.Top = lblDesc.Bottom + borderHeight;
                    this.Size = new Size(this.Size.Width, levelVarGraph.Bottom);
                    if (Expanded != null)
                        Expanded(this, EventArgs.Empty);
                }
            }
        }

        public event EventHandler Changed, Expanded;

        private Level_Variable var;
        public Level_Variable Var
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
                lblDesc.Text = var.Description;
                txtBase.Text = var.Base.Value;
                txtScale.Text = var.Scale.Value;
                txtPower.Text = var.Power.Value;
                txtDefaultBase.Text = var.Base.Default;
                txtDefaultScale.Text = var.Scale.Default;
                txtDefaultPower.Text = var.Power.Default;

                txtBase.BackColor = SystemColors.Window;
                txtScale.BackColor = SystemColors.Window;
                txtPower.BackColor = SystemColors.Window;

                groupBox1.Visible = true;
                UpdateLevelVarGraph();
                Compressed = false; // recalculate
            }
        }

        private void CheckDefault()
        {
            if (var != null)
                if (txtBase.Text == var.Base.Default && txtScale.Text == var.Scale.Default && txtPower.Text == var.Power.Default)
                    groupBox1.Text = var.Name;
                else
                    groupBox1.Text = var.Name + " *";

        }

        private void txtBase_TextChanged(object sender, EventArgs e)
        {
            if (Validate(txtBase) && var != null)
            {
                var.Base.Value = txtBase.Text;
                if (Changed != null)
                    Changed(this, EventArgs.Empty);
                UpdateLevelVarGraph();
            }
        }

        private void txtScale_TextChanged(object sender, EventArgs e)
        {
            if (Validate(txtScale) && var != null)
            {
                var.Scale.Value = txtScale.Text;
                if (Changed != null)
                    Changed(this, EventArgs.Empty);
                UpdateLevelVarGraph();
            }
        }

        private void txtPower_TextChanged(object sender, EventArgs e)
        {
            if (Validate(txtPower) && var != null)
            {
                var.Power.Value = txtPower.Text;
                if (Changed != null)
                    Changed(this, EventArgs.Empty);
                UpdateLevelVarGraph();
            }
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

        public float GetValue(int level)
        {
            if ( var == null )
                return 0;
            return Level_Variable.Combine(var.Base.Value, var.Scale.Value, var.Power.Value, level);
        }

        private void txtDefaultBase_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            txtBase.Text = txtDefaultBase.Text;
        }

        private void txtDefaultScale_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            txtScale.Text = txtDefaultScale.Text;
        }

        private void txtDefaultPower_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            txtPower.Text = txtDefaultPower.Text;
        }

        private void UpdateLevelVarGraph()
        {
            levelVarGraph.Values.Clear();
            for (int i = 1; i <= 10; i++)
                levelVarGraph.Values.Add(GetValue(i));
            levelVarGraph.Visible = true;
            levelVarGraph.Invalidate();
        }
    }
}
