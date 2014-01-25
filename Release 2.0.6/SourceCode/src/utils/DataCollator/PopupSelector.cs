using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

// the first time you click on popup, it stops being able to close.
// with the changes I've made, if you click again, it gains this ability again.

namespace DataCollator
{
    public partial class PopupSelector : UserControl
    {
        public string ButtonText
        {
            get { return button1.Text; }
            set { button1.Text = value; }
        }
        public PopupSelector()
        {
            InitializeComponent();
        }

        private void checkedListBox1_Leave(object sender, EventArgs e)
        {
            //dialog_Closing(this, EventArgs.Empty);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            OpenDialog();
        }

        private void checkedListBox1_MouseLeave(object sender, EventArgs e)
        {
            //dialog_Closing(this, EventArgs.Empty);
        }

        frmDialog dialog = null;
        MouseEventHandler popupHandler = null;
        private void OpenDialog()
        {
            if (dialog != null)
                return;

            Point location = Parent.PointToScreen(new Point(this.Right, this.Top));
            //get the dialog
            dialog = new frmDialog(location);
            popupHandler = new MouseEventHandler(dialog.CheckShouldClose);
            Controls.Remove(checkedListBox1);
            checkedListBox1.Parent = dialog;
            dialog.Controls.Add(checkedListBox1);
            checkedListBox1.Visible = true;
            checkedListBox1.MouseUp += popupHandler;
            //checkedListBox1.MouseClick += popupHandler;
            checkedListBox1.ItemCheck += new ItemCheckEventHandler(checkedListBox1_ItemCheck);
            dialog.PopupClosing += new EventHandler(dialog_Closing);
            dialog.Capture = true;
            dialog.Location = location;
            dialog.Show();
            checkedListBox1.SelectedIndices.Clear();
            checkedListBox1.Capture = true;
            /*
            checkedListBox1.Visible = true;
            checkedListBox1.Focus();
            BringToFront();*/
            //button1.FlatAppearance.BorderSize = 2;
        }

        void checkedListBox1_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            //if ( dialog != null )
                //dialog.Capture = true;
        }

        void dialog_Closing(object sender, EventArgs e)
        {
            checkedListBox1.Visible = false;
            dialog.Controls.Remove(checkedListBox1);
            checkedListBox1.Parent = this;
            Controls.Add(checkedListBox1);
            checkedListBox1.MouseUp -= popupHandler;
            checkedListBox1.SelectedIndices.Clear();
            //checkedListBox1.MouseClick -= popupHandler;
            
            //SendToBack();
            //button1.FlatAppearance.BorderSize = 1;
            dialog = null;
        }

        public CheckedListBox.ObjectCollection Items
        {
            get { return checkedListBox1.Items; }
        }

        public CheckedListBox.CheckedItemCollection CheckedItems
        {
            get { return checkedListBox1.CheckedItems; }
        }

        public CheckedListBox.CheckedIndexCollection CheckedIndices
        {
            get { return checkedListBox1.CheckedIndices; }
        }

        public void SetItemChecked(int index, bool value)
        {
            checkedListBox1.SetItemChecked(index, value);
        }

        public class frmDialog : Form
        {
            public frmDialog(Point startLocation)
            {
                InitializeComponent();

                //Remove title bar and set edge-style
                this.Text = string.Empty;
                this.FormBorderStyle = FormBorderStyle.FixedDialog;

                //Disable normal window functions
                this.MinimizeBox = false;
                this.MaximizeBox = false;
                this.ControlBox = false;
                this.ShowInTaskbar = false;
                this.TopMost = true; //make it appear on the very top

                //this.Capture = true; //allows mouse events to be triggered no matter where the mouse clicks

                //Match the position to the parent control
                this.Left = startLocation.X;
                this.Top = startLocation.Y;

            }

            public event EventHandler PopupClosing;
            
            protected override void OnMouseDown(MouseEventArgs e)
            {
                CheckShouldClose(this, e);
                base.OnMouseDown(e); //normal mouse behavior
            }

            public void CheckShouldClose(object sender, MouseEventArgs e)
            {
                //Check to see if the click is inside the drop-down Form
                if (!this.RectangleToScreen(this.ClientRectangle).Contains(Cursor.Position))
                {
                    PopupClosing(this, EventArgs.Empty);
                    this.Close(); //close the drop-down
                }
                else //if (sender != this)
                    /*((Control)sender).*/Capture = true;
            }

            /// <summary>
            /// Required method for Designer support - do not modify
            /// the contents of this method with the code editor.
            /// </summary>
            private void InitializeComponent()
            {
                this.SuspendLayout();
                this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
                this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                this.ClientSize = new System.Drawing.Size(124, 160);
                this.Name = "frmDialog";
                this.Text = "frmDialog";
                this.ResumeLayout(false);
                this.PerformLayout();

            }
        }
    }
}
