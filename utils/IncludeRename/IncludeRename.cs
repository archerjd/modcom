using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace IncludeRename
{
    class IncludeRename
    {
        public static void Run(DirectoryInfo dir)
        {
            FileInfo[] allHeaders = dir.GetFiles("*.h", SearchOption.AllDirectories);
            FileInfo[] allCppFiles = dir.GetFiles("*.cpp", SearchOption.AllDirectories);
            FileInfo[] allVcprojFiles = dir.GetFiles("*.vcproj", SearchOption.AllDirectories);
            List<string> allHeaderPaths = new List<string>();
            foreach (FileInfo header in allHeaders)
            { // Create a list of all paths of headers
                allHeaderPaths.Add(Path.GetFullPath(header.FullName));
            }

            Regex rVcproj = new Regex("AdditionalIncludeDirectories=\"(.+?)\"", RegexOptions.IgnoreCase);
            Regex rInclude = new Regex("#include\\s+(\"|<)(.+\\.h)(\"|>)", RegexOptions.IgnoreCase); // regex is #include\s+("|<)(.+/)*(.+\)*(.+?)(\.h)?("|>) ... i think that covers it?

            // now search through every CPP and H file for all matches of this expression (that's inefficient!)
            // for every match, if the headerNames array contains a match of the group (ignoring case),
            // then replace that with the correctly-cased version from headerNames array. And save the file when done.
            List<string> additionalIncludeDirectories = GetAdditionalIncludeDirectories(allVcprojFiles, rVcproj);
            SearchFilesForMatchesAndReplace(allCppFiles, allHeaders, rInclude, additionalIncludeDirectories, allHeaderPaths);
            SearchFilesForMatchesAndReplace(allHeaders, allHeaders, rInclude, additionalIncludeDirectories, allHeaderPaths);
        }

        const int includeTypeGroupNum = 1, includePathGroupNum = 2;
        static void SearchFilesForMatchesAndReplace(FileInfo[] fileList, FileInfo[] allHeaders, Regex r, List<string> additionalIncludeDirectories, List<string> allHeaderPaths)
        {
            foreach (FileInfo currentFile in fileList)
            {
                bool hasChanges = false;
                string currentFileText = File.ReadAllText(currentFile.FullName, Encoding.Default);
                MatchCollection matches = r.Matches(currentFileText);

                foreach (Match m in matches)
                {
                    Group nameGroup = m.Groups[includePathGroupNum];
                    bool isSpecialInclude = (nameGroup.Value == "<");
                    string fullHeaderString = nameGroup.Value;
                    List<string> fullPathsOfHeaderDirectories = GetHeaderDirectory(currentFile, fullHeaderString, isSpecialInclude, additionalIncludeDirectories, allHeaderPaths);
                    string iCompilerHeaderDirectory = fullPathsOfHeaderDirectories[0];

                    if (string.IsNullOrEmpty(iCompilerHeaderDirectory))
                    { // Skip this header. Its probably precompiled.
                        continue;
                    }

                    Uri currentDirPath = new Uri(iCompilerHeaderDirectory + Path.AltDirectorySeparatorChar);

                    if (fullPathsOfHeaderDirectories.Count > 1)
                    { // List has returned more results than expected.
                        // Do something clever to select the correct folder from the list.
                        // Something like "iCompilerHeaderDirectory = fullPathsOfHeaderDirectories[1];"
                        // throw new NotImplementedException();
                    }
                    foreach (string header in allHeaderPaths)
                    { // Loop through all header files.
                        Uri pathOfRealHeader = new Uri(header);
                        string relativePathOfRealHeader = currentDirPath.MakeRelativeUri(pathOfRealHeader).ToString();
                        if (relativePathOfRealHeader.ToLower() == fullHeaderString.ToLower())
                        {
                            if (relativePathOfRealHeader != fullHeaderString)
                            { // Header's case of path or name is wrong, fix it.
                                hasChanges = true;
                                // Replace header path here...
                                StringBuilder sb = new StringBuilder();
                                sb.Append(currentFileText.Substring(0, nameGroup.Index));
                                sb.Append(relativePathOfRealHeader);
                                sb.Append(currentFileText.Substring(nameGroup.Index + nameGroup.Length));
                                currentFileText = sb.ToString();
                            }
                        }
                    }
                }
                if (hasChanges)
                {
                    File.WriteAllText(currentFile.FullName, currentFileText, Encoding.Default);
                }
            }
        }

        public static List<string> GetHeaderDirectory(FileInfo currentFile, string fullHeaderString, bool isSpecialInclude, List<string> additionalIncludeDirectories, List<string> allHeaderPaths)
        {
            List<string> result = new List<string>();
            if (!isSpecialInclude)
            {   // Found a quoted include, 
                // locate the one in the same  directory as the curent file.
                //
                // ================================================================================
                //
                // This form instructs the preprocessor to look for include files in the same directory
                // of the file that contains the #include statement, and then in the directories of any
                // files that include (#include) that file. The preprocessor then searches along the 
                // path specified by the /I compiler option, 
                // then along paths specified by the INCLUDE environment variable.
                //
                // ================================================================================

                // Search for the header at the current directory
                if (allHeaderPaths.Contains(Path.GetFullPath(Path.Combine(currentFile.Directory.FullName, fullHeaderString)), StringComparer.OrdinalIgnoreCase))
                { // Found the header exists.
                    result.Add(currentFile.Directory.FullName);
                    return result;
                }
                else
                { // Could not find header in the current dir, file must be in one of the paths defined by the /I compiler switch.
                    foreach (string includeDirectory in additionalIncludeDirectories)
                    { // Loop through all additional directories & find the correct path
                        string tempPath = Path.GetFullPath(Path.Combine(includeDirectory, fullHeaderString));
                        if (allHeaderPaths.Contains(tempPath, StringComparer.OrdinalIgnoreCase))
                        { // Found the correct path of the included header.
                            result.Add(includeDirectory);
                        }
                    }
                    if (result.Count == 0)
                    {
                        // Could not find a header that matches. 
                        // Some headers are precompiled proviced by the OS.
                        result.Add(string.Empty);
                    }
                    return result;

                }// We should not get this far, something went wrong...
                throw new NotImplementedException();
            }
            else
            {   // Found a bracket include.
                //
                // ================================================================================
                //
                // This form instructs the preprocessor to search for include files first along 
                // the path specified by the /I compiler option, then, when compiling from the command line, 
                // along the path specified by the INCLUDE environment variable.
                //
                // ================================================================================
                //
                foreach (string includeDirectory in additionalIncludeDirectories)
                { // Loop through all additional directories & find the correct path
                    string tempPath = Path.GetFullPath(Path.Combine(includeDirectory, fullHeaderString));
                    if (allHeaderPaths.Contains(tempPath, StringComparer.OrdinalIgnoreCase))
                    { // Found the correct path of the included header.
                        result.Add(includeDirectory);
                    }
                }
                if (result.Count == 0)
                {
                    // Could not find a header that matches. 
                    // Some headers are precompiled proviced by the OS.
                    result.Add(string.Empty);
                }
                return result;
            } // We should not get this far, something went wrong...
            throw new NotImplementedException();
        }

        public static List<string> GetAdditionalIncludeDirectories(FileInfo[] allVcprojFiles, Regex rVcproj)
        {  // Loop through all vcproj files
            List<string> allAdditionalIncludeDirectories = new List<string>();            
            foreach (FileInfo fi in allVcprojFiles)
            { // Read each file to a string
                string fileText = File.ReadAllText(fi.FullName, Encoding.Default);
                // Find all maches defined by the regex
                MatchCollection matches = rVcproj.Matches(fileText);
                // Loop through all matches & extract the paths from the matched values
                foreach (Match m in matches)
                {
                    Group nameGroup = m.Groups[includeTypeGroupNum];
                    // Split all paths to an array so we can access each one.
                    string[] AdditionalIncludeDirectories = Regex.Split(nameGroup.Value, ";|,");
                    // Loop through each path & convert to full path.
                    foreach (string fPath in AdditionalIncludeDirectories)
                    {
                        string fullPath = Path.GetFullPath(Path.Combine(fi.Directory.FullName, fPath));
                        allAdditionalIncludeDirectories.Add(fullPath);
                    }
                }
            } // return a list of our findings that are unique.
            return allAdditionalIncludeDirectories.Distinct().ToList();
        }

        /// <summary>
        /// When passed in a list of FileInfos, each named with the same matching case
        /// </summary>
        /// <param name="options"></param>
        /// <param name="fileDirectory"></param>
        /// <param name="includePrefix"></param>
        /// <param name="isSpecialInclude"></param>
        /// <returns></returns>
    }
}
