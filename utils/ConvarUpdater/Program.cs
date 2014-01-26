using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Text.RegularExpressions;
using System.Net;

namespace ConvarUpdater
{
    class Program
    {
        const string validConvarNameRegex = "[a-zA-Z0-9_]*";
        const string validConvarValueRegex = "-?[0-9]+.?[0-9]*";
        static string[] whitespace = new string[] { " ", "	" };
        static bool anyErrors = false;
        static void Main(string[] args)
        {
#if DEBUG
            string codeFilePath = "C:\\Documents and Settings\\andrew\\My Documents\\Visual Studio 2008\\Projects\\mcsrc-trunk\\shared\\modcom\\mcconvar.cpp";
#else
            if (args.Length == 0)
            {
                Console.WriteLine("Failed: Local file path not provided!");
                return;
            }

            string codeFilePath = args[0];
#endif
            if (!File.Exists(codeFilePath))
            {
                Console.WriteLine("Failed: Local file not found: " + codeFilePath);
                return;
            }

            Uri updateURI = new Uri("http://www.modularcombatsource.com/dev/convar_changes.cfg");
            string[] convarFileLines;
            try
            {
                WebRequest req = WebRequest.Create(updateURI);
                WebResponse resp = req.GetResponse();
                Stream stream = resp.GetResponseStream();
                StreamReader sr = new StreamReader(stream);
                convarFileLines = sr.ReadToEnd().Split(new string[] { Environment.NewLine }, StringSplitOptions.RemoveEmptyEntries);
            }
            catch (WebException)
            {
                Console.WriteLine("Failed: Unable to load remote convar file");
                return;
            }
            
            string codeFileContents = File.ReadAllText(codeFilePath);

            foreach (string line in convarFileLines)
            {
                string s = line;
                int commentPos = s.IndexOf("//");
                if (commentPos > -1)
                    s = s.Substring(0, commentPos);
                s = s.Trim();
                if (s == string.Empty)
                    continue;

                string[] split = s.Split(whitespace, StringSplitOptions.RemoveEmptyEntries);
                if (split.Length < 2)
                {
                    Console.WriteLine("Invalid line in convar file: " + s);
                    Console.WriteLine("  Comment lines should start with //");
                    anyErrors = true;
                    continue;
                }

                ChangeDefault(ref codeFileContents, split[0], split[1]);
            }

            File.WriteAllText(codeFilePath, codeFileContents);
            if (anyErrors)
                Console.WriteLine("Errors occurred, unable to process all convar changes listed!");
            else
                Console.WriteLine("Convar update successful");

#if DEBUG
            Console.ReadKey();
#endif
        }

        public static bool ChangeDefault(ref string fileContents, string convarName, string newValue)
        {
            // check that convarName is a valid convar name (contains only a-z, A-Z, 0-9 & _
            if (!Regex.IsMatch(convarName, validConvarNameRegex))
            {
                Console.WriteLine(convarName + " is an invalid convar name!");
                anyErrors = true;
                return false;
            }

            if (!Regex.IsMatch(newValue, validConvarValueRegex))
            {
                Console.WriteLine(convarName + " - attempting to specify an invalid value: " + newValue);
                anyErrors = true;
                return false;
            }

            string pattern; // whichever pattern we use, want to replace group 2 with the new value, putting it between the unmodified group 1 and group 3
            if (convarName.EndsWith("_base"))
            {
                string shortName = convarName.Substring(0, convarName.Length - 5);
                pattern = string.Format("\\b(LEVEL_VARIABLE\\s*\\(\\s*{0}\\s*,[a-zA-Z_\\| ]+\\s*,\\s*)\"({1})(\",\\s*\"{1}\",\\s*\"{1}\")", shortName, validConvarValueRegex);
            }
            else if (convarName.EndsWith("_scale"))
            {
                string shortName = convarName.Substring(0, convarName.Length - 6);
                pattern = string.Format("\\b(LEVEL_VARIABLE\\s*\\(\\s*{0}\\s*,[a-zA-Z_\\| ]+\\s*,\\s*\"{1}\",\\s*)\"({1})(\",\\s*\"{1}\")", shortName, validConvarValueRegex);
            }
            else if (convarName.EndsWith("_power"))
            {
                string shortName = convarName.Substring(0, convarName.Length - 6);
                pattern = string.Format("\\b(LEVEL_VARIABLE\\s*\\(\\s*{0}\\s*,[a-zA-Z_\\| ]+\\s*,\\s*\"{1}\",\\s*\"{1}\",\\s*)\"({1})(\")", shortName, validConvarValueRegex);
            }
            else
                pattern = string.Format("\\b(McConVar\\s+[a-zA-Z0-9_]+\\s*\\(\\s*\"{0}\"\\s*,\\s*)\"({1})(\")", convarName, validConvarValueRegex);


            Regex r = new Regex(pattern);
            Match m = r.Match(fileContents);
            if (!m.Success)
            {
                Console.WriteLine(convarName + " not found in mcconvar.cpp!");
                anyErrors = true;
                return false;
            }

            fileContents = fileContents.Remove(m.Index, m.Length).Insert(m.Index, m.Result(string.Format("$1\"{0}$3", newValue.Replace("$", "$$"))));
            //fileContents = Regex.Replace(fileContents, pattern, string.Format("$1\"{0}$3", newValue.Replace("$", "$$")));

#if DEBUG
            Console.WriteLine("Set " + convarName + " to " + newValue);
            Console.WriteLine("Match: " + m.Value);
            Console.WriteLine();
#endif
            return true;
        }
    }
}
