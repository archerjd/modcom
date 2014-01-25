using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Runtime.InteropServices;

namespace ResourceExtractor
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        string steamAppsDir = null, exportDir = null;
#region ui stuff, setup
        private void btnBrowseSteamApps_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            fbd.Description = "Select SteamApps directory:";
            fbd.ShowNewFolderButton = false;
            if (fbd.ShowDialog() != DialogResult.OK)
                return;

            lblSteamAppDir.Text = fbd.SelectedPath;
            steamAppsDir = fbd.SelectedPath;
            CheckReady();
        }

        private void btnBrowseExport_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            fbd.Description = "Select export directory:";
            fbd.ShowNewFolderButton = true;
            if (fbd.ShowDialog() != DialogResult.OK)
                return;

            lblExportDir.Text = fbd.SelectedPath;
            exportDir = fbd.SelectedPath;
            CheckReady();
        }

        private void CheckReady()
        {
            btnExtract.Enabled = steamAppsDir != null && exportDir != null;
        }
#endregion

        const string ep2GCF = "episodic 2007 shared.gcf";
        const string fileList = "extract.txt";

        static bool exportErrors = false;
        uint uiPackage = HLLib.HL_ID_INVALID;
        uint uiMode = (uint)HLLib.HLFileMode.HL_MODE_INVALID;

        private void btnExtract_Click(object sender, EventArgs e)
        {
            exportErrors = false;

            if ( !File.Exists(fileList) )
            {
                MessageBox.Show("", "File not found", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if ( !LoadPackage(steamAppsDir + "\\" + ep2GCF) )
                return;

            string[] filesToExtract = File.ReadAllLines(fileList);
            foreach ( string file in filesToExtract )
                ExtractItem(file, exportDir);

            ClosePackage();

            if (exportErrors)
                MessageBox.Show("Errors occured during export", "Failed", MessageBoxButtons.OK, MessageBoxIcon.Information);
            else
                MessageBox.Show("Export complete", "Succeeded", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

#region load / close / extract
        static readonly uint MAX_PATH_SIZE = 512;

        static string GetPath(IntPtr pItem)
        {
            string sPath = string.Empty;
            IntPtr lpPath = IntPtr.Zero;
            try
            {
                lpPath = Marshal.AllocHGlobal((int)MAX_PATH_SIZE);
                HLLib.hlItemGetPath(pItem, lpPath, MAX_PATH_SIZE);
                sPath = Marshal.PtrToStringAnsi(lpPath);
            }
            finally
            {
                if (lpPath != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(lpPath);
                }
            }
            return sPath;
        }

        private bool LoadPackage(string sPackage)
        {
            HLLib.HLPackageType ePackageType = HLLib.HLPackageType.HL_PACKAGE_NONE; // may as well let it get this from the file, even though we know its gcf.
            bool bFileMapping = true, bQuickFileMapping = false, bOverwriteFiles = false;
            bool bVolatileAccess = true, bWriteAccess = false;

            if (!File.Exists(sPackage))
            {
                MessageBox.Show(string.Format("Cannot locate required package: {0}", ep2GCF), "File not found", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            HLLib.hlInitialize();

            if (HLLib.hlGetUnsignedInteger(HLLib.HLOption.HL_VERSION) < HLLib.HL_VERSION_NUMBER)
            {
                MessageBox.Show(string.Format("Wrong HLLib version: v{0}.", HLLib.hlGetString(HLLib.HLOption.HL_VERSION)), "HLLib error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            // Keep the delegates alive so they don't get garbage collected.
            HLLib.HLExtractItemStartProc HLExtractItemStartProc = new HLLib.HLExtractItemStartProc(ExtractItemStartCallback);
            HLLib.HLExtractItemEndProc HLExtractItemEndProc = new HLLib.HLExtractItemEndProc(ExtractItemEndCallback);
            HLLib.HLExtractFileProgressProc HLExtractFileProgressProc = new HLLib.HLExtractFileProgressProc(FileProgressCallback);
            HLLib.HLValidateFileProgressProc HLValidateFileProgressProc = new HLLib.HLValidateFileProgressProc(FileProgressCallback);
            HLLib.HLDefragmentFileProgressExProc HLDefragmentFileProgressProc = new HLLib.HLDefragmentFileProgressExProc(DefragmentProgressCallback);
            
            HLLib.hlSetBoolean(HLLib.HLOption.HL_OVERWRITE_FILES, bOverwriteFiles);
            HLLib.hlSetVoid(HLLib.HLOption.HL_PROC_EXTRACT_ITEM_START, Marshal.GetFunctionPointerForDelegate(HLExtractItemStartProc));
            HLLib.hlSetVoid(HLLib.HLOption.HL_PROC_EXTRACT_ITEM_END, Marshal.GetFunctionPointerForDelegate(HLExtractItemEndProc));
            HLLib.hlSetVoid(HLLib.HLOption.HL_PROC_EXTRACT_FILE_PROGRESS, Marshal.GetFunctionPointerForDelegate(HLExtractFileProgressProc));
            HLLib.hlSetVoid(HLLib.HLOption.HL_PROC_VALIDATE_FILE_PROGRESS, Marshal.GetFunctionPointerForDelegate(HLValidateFileProgressProc));
            HLLib.hlSetVoid(HLLib.HLOption.HL_PROC_DEFRAGMENT_PROGRESS_EX, Marshal.GetFunctionPointerForDelegate(HLDefragmentFileProgressProc));

#region determine package type
            ePackageType = HLLib.hlGetPackageTypeFromName(sPackage);
            // If the above fails, try getting the package type from the data at the start of the file.
            if (ePackageType == HLLib.HLPackageType.HL_PACKAGE_NONE && File.Exists(sPackage))
            {
                FileStream Reader = null;
                try
                {
                    byte[] lpBuffer = new byte[HLLib.HL_DEFAULT_PACKAGE_TEST_BUFFER_SIZE];
                    Reader = new FileStream(sPackage, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                    int iBytesRead = Reader.Read(lpBuffer, 0, lpBuffer.Length);
                    if (iBytesRead > 0)
                    {
                        IntPtr lpBytesRead = Marshal.AllocHGlobal(iBytesRead);
                        try
                        {
                            Marshal.Copy(lpBuffer, 0, lpBytesRead, iBytesRead);
                            ePackageType = HLLib.hlGetPackageTypeFromMemory(lpBytesRead, (uint)iBytesRead);
                        }
                        finally
                        {
                            Marshal.FreeHGlobal(lpBytesRead);
                        }
                    }
                }
                finally
                {
                    if (Reader != null)
                    {
                        Reader.Close();
                    }
                }
            }
#endregion

            if (ePackageType == HLLib.HLPackageType.HL_PACKAGE_NONE)
            {
                MessageBox.Show(string.Format("Error loading {0}:", sPackage), "HLLib error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                HLLib.hlShutdown();
                return false;
            }

            // Create a package element, the element is allocated by the library and cleaned
            // up by the library.  An ID is generated which must be bound to apply operations
            // to the package.
            if (!HLLib.hlCreatePackage(ePackageType, out uiPackage))
            {
                MessageBox.Show(string.Format("Error loading {0}:{1}{2}", sPackage, Environment.NewLine, HLLib.hlGetString(HLLib.HLOption.HL_ERROR_SHORT_FORMATED)), "HLLib error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                HLLib.hlShutdown();
                return false;
            }

            HLLib.hlBindPackage(uiPackage);

            uiMode = (uint)HLLib.HLFileMode.HL_MODE_READ;
            uiMode |= !bFileMapping ? (uint)HLLib.HLFileMode.HL_MODE_NO_FILEMAPPING : 0;
            uiMode |= bQuickFileMapping ? (uint)HLLib.HLFileMode.HL_MODE_QUICK_FILEMAPPING : 0;
            uiMode |= bVolatileAccess ? (uint)HLLib.HLFileMode.HL_MODE_VOLATILE : 0;
            uiMode |= bWriteAccess ? (uint)HLLib.HLFileMode.HL_MODE_WRITE : 0;

            // Open the package.
            // Of the above modes, only HL_MODE_READ is required.  HL_MODE_WRITE is present
            // only for future use.  File mapping is recommended as an efficient way to load
            // packages.  Quick file mapping maps the entire file (instead of bits as they are
            // needed) and thus should only be used in Windows 2000 and up (older versions of
            // Windows have poor virtual memory management which means large files won't be able
            // to find a continues block and will fail to load).  Volatile access allows HLLib
            // to share files with other applications that have those file open for writing.
            // This is useful for, say, loading .gcf files while Steam is running.
            if (!HLLib.hlPackageOpenFile(sPackage, uiMode))
            {
                MessageBox.Show(string.Format("Error loading {0}:{1}{2}", sPackage, Environment.NewLine, HLLib.hlGetString(HLLib.HLOption.HL_ERROR_SHORT_FORMATED)), "HLLib error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                HLLib.hlShutdown();
                return false;
            }
/*
            // If we have a .ncf file, the package file data is stored externally.  In order to
            // validate the file data etc., HLLib needs to know where to look.  Tell it where.
            if (ePackageType == HLLib.HLPackageType.HL_PACKAGE_NCF && sNCFRootPath.Length > 0)
            {
                HLLib.hlNCFFileSetRootPath(sNCFRootPath);
            }
*/
            return true;
        }

        private void ClosePackage()
        {
            HLLib.hlPackageClose();

            // Free up the allocated memory.
            HLLib.hlDeletePackage(uiPackage);
            HLLib.hlShutdown();
        }

        private void ExtractItem(string itemName, string exportPath)
        {
            IntPtr root = HLLib.hlPackageGetRoot();

            IntPtr pSubItem;
            if (String.Equals(itemName, ".", StringComparison.CurrentCultureIgnoreCase))
                pSubItem = root;
            else
                pSubItem = HLLib.hlFolderGetItemByName(root, itemName, HLLib.HLFindType.HL_FIND_ALL);

            if (pSubItem != IntPtr.Zero)
            {
                // Extract the item.
                // Item is extracted to cDestination\Item->GetName().

                HLLib.hlItemExtract(pSubItem, exportPath);
            }
            else
            {
                MessageBox.Show(string.Format("{0} not found.", itemName), "File not found", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                exportErrors = true;
            }
        }
#endregion

#region internal callbacks
        static void ExtractItemStartCallback(int iItem)
        {
            IntPtr pItem = new IntPtr(iItem);
            if (HLLib.hlItemGetType(pItem) == HLLib.HLDirectoryItemType.HL_ITEM_FILE)
                ProgressStart();
        }


        static void FileProgressCallback(int iFile, uint uiBytesExtracted, uint uiBytesTotal, ref bool pCancel)
        {
            ProgressUpdate((UInt64)uiBytesExtracted, (UInt64)uiBytesTotal);
        }

        static void ExtractItemEndCallback(int iItem, bool bSuccess)
        {
            IntPtr pItem = new IntPtr(iItem);
            if (!bSuccess)
            {
                exportErrors = true;
                if (HLLib.hlItemGetType(pItem) == HLLib.HLDirectoryItemType.HL_ITEM_FILE)
                {
                    Console.WriteLine("  Error extracting {0}:", GetPath(pItem));
                    Console.WriteLine("    {0}", HLLib.hlGetString(HLLib.HLOption.HL_ERROR_SHORT_FORMATED));
                }
                else
                {
                    Console.WriteLine("  Error extracting {0}.", GetPath(pItem));
                }
            }
        }

        static void DefragmentProgressCallback(int iFile, uint uiFilesDefragmented, uint uiFilesTotal, UInt64 uiBytesDefragmented, UInt64 uiBytesTotal, ref bool pCancel)
        {
            ProgressUpdate(uiBytesDefragmented, uiBytesTotal);
        }
#endregion

#region progress control
        static uint uiProgressLast;

        static void ProgressStart()
        {
            uiProgressLast = 0;
            Console.Write("0%");
        }

        static void ProgressUpdate(UInt64 uiBytesDone, UInt64 uiBytesTotal)
        {
            uint uiProgress = uiBytesTotal == 0 ? 100 : (uint)(uiBytesDone * 100 / uiBytesTotal);
            while (uiProgress >= uiProgressLast + 10)
            {
                uiProgressLast += 10;
                if (uiProgressLast == 100)
                {
                    Console.Write("100% ");
                }
                else if (uiProgressLast == 50)
                {
                    Console.Write("50%");
                }
                else
                {
                    Console.Write(".");
                }
            }
        }
#endregion
    }
}
