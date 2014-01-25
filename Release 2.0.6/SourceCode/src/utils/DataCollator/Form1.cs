using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using FTPLib;
using System.IO;
using System.Collections;
using System.Threading;
using System.Data.SQLite;

/*
This project has to...
OK FTP onto my server using mc server login details        
OK download all .db files to a temporary folder
 * collate the data from these into a single database file, ensuring no game id collisions occur
 * display some basic information on the contents of this database, and provide an interface to query it
*/

namespace DataCollator
{
    public partial class Form1 : Form
    {

        // ideally a way of selecting the username used (eg mc_server_178@...) would be a nice addition
        public Form1()
        {
            InitializeComponent();
        }

        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
            if ( Form2.moduleListFilePath != null ) // if we didn't set this, we're being forced to close. Don't try to show the next bit
                Owner.Show();
        }

        FTP ftplib;
        private void btnDownload_Click(object sender, EventArgs e)
        {
            if (File.Exists(defaultConnectionFilename))
                connectionFilename = defaultConnectionFilename;
            else
            {
                OpenFileDialog ofd = new OpenFileDialog();
                ofd.Filter = "All files (*.*)|*.*";
                ofd.Title = "Select connection detail file";
                if (ofd.ShowDialog() == DialogResult.OK)
                    connectionFilename = ofd.FileName;
                else
                    return;
            }

            btnSkip.Visible = false;
            btnDownload.Enabled = false;
            progressBar1.Visible = true;
            
            Thread oThread = new Thread(new ThreadStart(FTPDownload));
            oThread.Start();
        }

        public delegate void IntSendingEvent(object sender, int value);

        const string defaultConnectionFilename = "connection.txt";
        const string defaultModuleListFilename = "modules.txt";
        string connectionFilename = null;
        
        protected delegate void EmptyDelegate();
        protected delegate void IntDelegate(int value);

        void SetProgressRange(int value)
        {
            progressBar1.Value = progressBar1.Minimum = 0;
            progressBar1.Maximum = value;
        }

        void SetProgressValue(int value)
        {
            progressBar1.Value = value;
        }

        private void FTPDownload()
        {
            try
            {
                ftplib = new FTP();
                string[] authentication = File.ReadAllLines(connectionFilename);
                if (authentication.Length >= 4)
                    ftplib.Connect(authentication[0], 21, authentication[1], authentication[2]);
                else
                    throw new Exception(connectionFilename + " does not contain server name / user name / password / filename!");

                string targetFile = authentication[3]; // filename should be like "2.0.1.db"

                ftplib.OpenDownload(targetFile, SqlInterface.filename, false);
                int progressRange = (int)ftplib.FileSize;
                BeginInvoke(new IntDelegate(SetProgressRange), progressRange);
                while (ftplib.DoDownload() > 0)
                    BeginInvoke(new IntDelegate(SetProgressValue), (int)ftplib.BytesTotal);

                // ensure we finish at 100%
                BeginInvoke(new IntDelegate(SetProgressValue), progressRange);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error downloading server data: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            DownloadFinished();
        }

        private void DownloadFinished()
        {
            if (File.Exists(defaultModuleListFilename))
                Form2.moduleListFilePath = defaultModuleListFilename;
            else
            {
                // ask for module list file to use (so that old versions can be used)
                OpenFileDialog ofd = new OpenFileDialog();
                ofd.Filter = "All files (*.*)|*.*";
                ofd.Title = "Select module list file to open";
                if (ofd.ShowDialog() == DialogResult.OK)
                    Form2.moduleListFilePath = ofd.FileName;
            }
            SqlInterface.db = new SQLiteConnection(string.Format("Data Source={0};Version=3;", SqlInterface.filename));

            // close this form, load Form2
            BeginInvoke(new EmptyDelegate(Close));
        }

        private void btnSkip_Click(object sender, EventArgs e)
        {
            DownloadFinished();
        }
    }
}
