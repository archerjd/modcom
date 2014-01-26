using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace MonsterDifficultyConfig
{
    public partial class EntryPopup : Form
    {
        public EntryPopup(string field, int level)
        {
            InitializeComponent();
            editingField = field;
            editingLevel = level;
        }

        string editingField;
        int editingLevel;

        public string Label
        {
            get { return label1.Text; }
            set { label1.Text = value; }
        }

        new public string Text
        {
            get { return textBox1.Text; }
            set { textBox1.Text = value; textBox1.SelectAll(); }
        }

        public string Text2
        {
            get { return textBox2.Text; }
            set { textBox2.Text = value; textBox2.SelectAll(); }
        }

        public EventHandler Changed;
        private void btnOK_Click(object sender, EventArgs e)
        {
            lastEnteredValue1 = textBox1.Text;
            lastEnteredValue2 = textBox2.Text;
            lastEditedField = editingField;
            lastEditedLevel = editingLevel;
            Close();
            Changed(this, e);
        }
        
        private void btnCancel_Click(object sender, EventArgs e)
        {
            Close();
        }

        public enum ModeType
        {
            Single,
            Double,
        }

        ModeType myMode;
        public ModeType Mode
        {
            get { return myMode; }
            set
            {
                myMode = value;
                if (myMode == ModeType.Single)
                {
                    textBox1.Width = 174;
                    textBox2.Visible = false;
                }
                else
                {
                    textBox1.Width = 79;
                    textBox2.Visible = true;
                }
            }
        }

        public static void Show(string label, string text1, string text2, string field, int level, EventHandler onChanged, Form parent)
        {
            EntryPopup p = new EntryPopup(field, level);
            p.Label = label;
            p.Text = text1;
            p.Text2 = text2;
            p.Changed += onChanged;
            p.Mode = level >= -2 ? ModeType.Single : ModeType.Double;
            p.Show(parent);
        }

        public static string lastEditedField, lastEnteredValue1, lastEnteredValue2;
        public static int lastEditedLevel;
    }
}
