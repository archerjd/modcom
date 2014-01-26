using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Text.RegularExpressions;
using System.Runtime.InteropServices;
using System.Threading;

namespace MCSIDE
{
    public partial class Form1 : Form
    {
        enum SaveState
        {
            NoData,
            SavedData,
            UnsavedData,
        }
        SaveState currentState;
        string currentFile = null;
        string storedFormattedText;
        Thread testing = null;

        public Form1()
        {
            InitializeComponent();
            txtMain.Font = defaultFont;
            txtMain.ForeColor = defaultColor;

            string[] kw = { "int", "rand", "if", "else", "elseif", "end", "wait", "while", "monster", "spawn", "struct", "award", "exp", "pvm", "module", "=", "vote", "descrip", "type", "voteresult", "run", "null"};
            foreach ( string k in kw )
                keywords.Add(k, 0);

            string[] vars = { "true", "false" };
            foreach ( string k in vars )
                gameVars.Add(k, 0);

            currentState = SaveState.NoData;
            StateChanged();

            MCS_EngineInterface.StartScriptEngine();
        }

        static Color defaultColor = Color.Black;
        static Color keywordColor = Color.Blue;
        static Color commentColor = Color.Green;
        static Color stringColor = Color.Brown;
        static Color varColor = Color.Purple;
        static Color errorColor = Color.DarkRed;
        
        static Font defaultFont = new Font("Courier New", 10, FontStyle.Regular);
        static Font boldFont = new Font("Courier New", 10, FontStyle.Bold);

        const string commentSeperator = "#";
        SortedList<string, int> keywords = new SortedList<string,int>();
        SortedList<string, int> gameVars = new SortedList<string, int>();

        static char[] seperators = { ' ', '	', '.', '[', ']', '(', ')', '+', '-', '*', '/' };
        static char[] brackets = { '[', ']' };
        

        // fancy stuff to stop this from updating all these selection changes; reduce flicker
        private const int WM_SETREDRAW = 0x000B;
        private const int WM_USER = 0x400;
        private const int EM_GETEVENTMASK = (WM_USER + 59);
        private const int EM_SETEVENTMASK = (WM_USER + 69);
        [DllImport("user32", CharSet = CharSet.Auto)]
        private extern static IntPtr SendMessage(IntPtr hWnd, int msg, int wParam, IntPtr lParam);

        // hopefully can catch the start of a paste operation here
        int keyPressLine = 0;
        private void txtMain_KeyPress(object sender, KeyPressEventArgs e)
        {
            keyPressLine = txtMain.GetLineFromCharIndex(txtMain.SelectionStart);
        }

        private void txtMain_TextChanged(object sender, EventArgs e)
        {
            // this doesn't need to handle the whole text, or even the whole line
            // just from the start of the word you're editing to the end of the line
            int keyReleaseLine = txtMain.GetLineFromCharIndex(txtMain.SelectionStart);

            IntPtr eventMask = IntPtr.Zero;
            try
            {
                // Stop redrawing:
                SendMessage(txtMain.Handle, WM_SETREDRAW, 0, IntPtr.Zero);
                // Stop sending of events:
                eventMask = SendMessage(txtMain.Handle, EM_GETEVENTMASK, 0, IntPtr.Zero);
                
                for ( int i=keyPressLine; i<= keyReleaseLine; i++ )
                    RecolorLine(i);
            }
            finally
            {
                // turn on events
                SendMessage(txtMain.Handle, EM_SETEVENTMASK, 0, eventMask);
                // turn on redrawing
                SendMessage(txtMain.Handle, WM_SETREDRAW, 1, IntPtr.Zero);
                txtMain.Refresh();
            }

            if (currentState != SaveState.UnsavedData)
            {
                currentState = SaveState.UnsavedData;
                StateChanged();
            }
        }

        private void RecolorLine(int num)
        {
            int caratPos = txtMain.SelectionStart;

            int startOfLine = txtMain.GetFirstCharIndexFromLine(num);
            string line = txtMain.TextLength == 0 ? "" : txtMain.Lines[num/*txtMain.GetLineFromCharIndex(txtMain.SelectionStart)*/];

            // these values both refer to position in LINE, NOT position in the textbox
            //int whiteSpacePos = line.LastIndexOfAny(whitespace, txtMain.SelectionStart - startOfLine-1) + 1; // +1 also works for the case when it wasn't found, puts the -1 to 0
            int endOfLine = line.IndexOf(commentSeperator);
            if (endOfLine == -1)
                endOfLine = line.Length;
            else
            { // color comment text
                txtMain.SelectionStart = endOfLine + startOfLine;
                txtMain.SelectionLength = line.Length - endOfLine;

                txtMain.SelectionFont = defaultFont;
                txtMain.SelectionColor = commentColor;
            }

            string[] tokens = line.Substring(0, endOfLine).ToLower().Split(seperators, StringSplitOptions.None);
            int index = startOfLine;

            foreach (string token in tokens)
            {
                if (token.Length > 0)
                {
                    txtMain.SelectionStart = index;
                    txtMain.SelectionLength = token.Length;
                    decimal ignoreMe;

                    // handle keywords
                    if (keywords.ContainsKey(token))
                    {
                        txtMain.SelectionFont = boldFont;
                        txtMain.SelectionColor = keywordColor;
                    }
                    // handle game vars
                    else if (gameVars.ContainsKey(token) || decimal.TryParse(token, out ignoreMe))
                    {
                        txtMain.SelectionFont = defaultFont;
                        txtMain.SelectionColor = varColor;
                    }
                    // default color / font
                    else
                    {
                        txtMain.SelectionFont = defaultFont;
                        txtMain.SelectionColor = defaultColor;
                    }
                }
                index += token.Length + 1;
            }

            // now handle square brackets seperately, as they're considered a seperator by the previous part.
            int seekPos = line.IndexOfAny(brackets);
            while (seekPos != -1)
            {
                txtMain.SelectionStart = seekPos + startOfLine;
                txtMain.SelectionLength = 1;

                txtMain.SelectionFont = boldFont;
                seekPos = line.IndexOfAny(brackets, seekPos + 1);
            }

            // lastly, handle strings seperately - as these can overwrite the rules used previously, they must be last
            seekPos = 0;
            while ( true )
            {
                int stringOpen = line.IndexOf('"', seekPos);
                if (stringOpen != -1)
                {
                    int stringClose = line.IndexOf('"', stringOpen + 1);
                    if (stringClose != -1)
                    {
                        txtMain.SelectionStart = stringOpen + startOfLine;
                        txtMain.SelectionLength = stringClose - stringOpen + 1;

                        txtMain.SelectionFont = defaultFont;
                        txtMain.SelectionColor = stringColor;

                        seekPos = stringClose + 1;
                    }
                    else
                    {
                        txtMain.SelectionStart = stringOpen + startOfLine;
                        txtMain.SelectionLength = line.Length - stringOpen;

                        txtMain.SelectionFont = boldFont;
                        txtMain.SelectionColor = defaultColor;
                        break;
                    }
                }
                else
                    break;
            }

            txtMain.SelectionStart = caratPos;
            txtMain.SelectionLength = 0;
        }

        private void RecolorAll()
        {
            IntPtr eventMask = IntPtr.Zero;
            try
            {
                // Stop redrawing:
                SendMessage(txtMain.Handle, WM_SETREDRAW, 0, IntPtr.Zero);
                // Stop sending of events:
                eventMask = SendMessage(txtMain.Handle, EM_GETEVENTMASK, 0, IntPtr.Zero);

                for (int i = 0; i < txtMain.Lines.Length; i++)
                    RecolorLine(i);
            }
            finally
            {
                // turn on events
                SendMessage(txtMain.Handle, EM_SETEVENTMASK, 0, eventMask);
                // turn on redrawing
                SendMessage(txtMain.Handle, WM_SETREDRAW, 1, IntPtr.Zero);
                txtMain.Refresh();
            }
        }

        private void StateChanged()
        {
            switch ( currentState )
            {
                case SaveState.NoData:
                    btnNew.Enabled = btnSave.Enabled = btnSaveAs.Enabled = btnTest.Enabled = false;
                    btnOpen.Enabled = true;
                    Text = "MCS Editor - new script";
                    break;
                case SaveState.SavedData:
                    btnSave.Enabled = false;
                    btnNew.Enabled = btnSaveAs.Enabled = btnOpen.Enabled = btnTest.Enabled = true;
                    Text = "MCS Editor - " + currentFile.Substring(currentFile.LastIndexOf('\\') + 1);
                    break;

                case SaveState.UnsavedData:
                    btnSave.Enabled = btnNew.Enabled = btnSaveAs.Enabled = btnOpen.Enabled = true;
                    if (currentFile == null)
                    {
                        btnTest.Enabled = false;
                        Text = "MCS Editor - new script *";
                    }
                    else
                    {
                        btnTest.Enabled = true;
                        Text = "MCS Editor - " + currentFile.Substring(currentFile.LastIndexOf('\\') + 1) + " *";
                    }
                    break;
            }
        }

        private bool UnsavedDataOk()
        {
            if (currentState == SaveState.UnsavedData)
                return MessageBox.Show("You have unsaved changes, if you continue these will be lost! Do you wish to continue?", "Unsaved data", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes;
            return true;
        }

        private void btnNew_Click(object sender, EventArgs e)
        {
            if (!UnsavedDataOk())
                return;                

            txtMain.Text = string.Empty;
            currentState = SaveState.NoData;
            StateChanged();
            currentFile = null;
        }

        private void btnOpen_Click(object sender, EventArgs e)
        {
            if (!UnsavedDataOk())
                return;

            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "mcs files (*.mcs)|*.mcs|All files (*.*)|*.*";
            ofd.Title = "Select a file to open";
            if (ofd.ShowDialog() != DialogResult.OK)
                return;

            currentFile = ofd.FileName;
            txtMain.Visible = false;
            txtMain.TextChanged -= txtMain_TextChanged;

            txtMain.Text = File.ReadAllText(currentFile);
            currentState = SaveState.SavedData;
            StateChanged();
            RecolorAll();
            txtMain.Visible = true;

            txtMain.TextChanged += txtMain_TextChanged;
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            if (currentFile == null)
            {
                DoSaveAs();
                return;
            }

            File.WriteAllText(currentFile, txtMain.Text);
            currentState = SaveState.SavedData;
            StateChanged();
        }

        private void btnSaveAs_Click(object sender, EventArgs e)
        {
            DoSaveAs();
        }

        // lets us return false when rejected, needed for closing with an unsaved file open
        private bool DoSaveAs()
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "mcs files (*.mcs)|*.mcs|All files (*.*)|*.*";
            sfd.Title = "Enter filename to save";
            //sfd.InitialDirectory = ; // look up steam directory?
            sfd.CheckFileExists = false;
            if (sfd.ShowDialog() != DialogResult.OK)
                return false;

            currentFile = sfd.FileName;
            File.WriteAllText(currentFile, txtMain.Text);
            currentState = SaveState.SavedData;
            StateChanged();
            return true;
        }

        private void btnTest_Click(object sender, EventArgs e)
        {
            if (testing != null)
            {
                RestoreIDE();
                return;
            }
            if (currentFile == null) // need to have a saved file for MCS to load
                return;

            storedFormattedText = txtMain.Rtf;
            btnNew.Enabled = btnOpen.Enabled = btnSave.Enabled = btnSaveAs.Enabled = false;
            txtMain.TextChanged -= txtMain_TextChanged;
            txtMain.Text = "";
            txtMain.WordWrap = true;
            btnTest.Text = "Stop";

            testing = new Thread(new ThreadStart(StartTesting));
            testing.Start();
        }

        MCS_EngineInterface.OutputFunction commandOut, errorOut; // need to save these to stop them being garbage collected
        private void StartTesting()
        {
            OutputCommand("Initialising MCS dll...");
            commandOut = OutputCommand;
            errorOut = OutputError;
            MCS_EngineInterface.SetCommandOutputFunction(commandOut);
            MCS_EngineInterface.SetErrorOutputFunction(errorOut);

            if (MCS_EngineInterface.ParseThenRunScript(currentFile))
                OutputCommand("MCS initialised, thinking indefinately...");
            

            while (true)
            {
                MCS_EngineInterface.Think(0);
                Thread.Sleep(50);
            }

            // this line is unreachable... but fair enough
            FinishedTesting();
        }

        private void OutputCommand(string s)
        {
            OutputDelegate output = new OutputDelegate(WriteTestOutput);
            BeginInvoke(output, false, s);
        }

        private void OutputError(string s)
        {
            OutputDelegate output = new OutputDelegate(WriteTestOutput);
            BeginInvoke(output, true, s);
        }

        protected delegate void OutputDelegate(bool isError, string text);
        private void WriteTestOutput(bool isError, string text)
        {
            if (testing == null)
                return;

            int startPos = txtMain.TextLength;
            txtMain.AppendText(text + Environment.NewLine);
            txtMain.SelectionStart = startPos;
            txtMain.SelectionLength = text.Length;

            txtMain.SelectionFont = defaultFont;
            txtMain.SelectionColor = isError ? errorColor : defaultColor;

            txtMain.SelectionStart = txtMain.TextLength;
            txtMain.SelectionLength = 0;
            txtMain.ScrollToCaret();
        }

        protected delegate void EmptyDelegate();
        private void FinishedTesting()
        {
            EmptyDelegate dp = new EmptyDelegate(this.RestoreIDE);
            BeginInvoke(dp);
        }

        private void RestoreIDE()
        {
            testing.Abort();
            testing = null;
            txtMain.WordWrap = false;
            txtMain.Rtf = storedFormattedText;
            txtMain.TextChanged += txtMain_TextChanged;
            btnTest.Text = "Test";
            StateChanged();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (testing != null) // can't close during a test run
                e.Cancel = true;
            else if (currentState == SaveState.UnsavedData && MessageBox.Show("You have unsaved changes, save your changes before exiting?", "Unsaved data", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
                if (!DoSaveAs())
                    e.Cancel = true;
        }
    }
}
