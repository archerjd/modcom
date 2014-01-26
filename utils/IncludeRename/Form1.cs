using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace IncludeRename
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            DirectoryInfo d;
            try
            {
                d = new DirectoryInfo(textBox1.Text);
                if (!d.Exists)
                {
                    MessageBox.Show("Directory not found");
                    return;
                }
            }
            catch
            {
                MessageBox.Show("Directory not valid");
                return;
            }

            IncludeRename.Run(d);
            MessageBox.Show("Done");
        }
    }
}
