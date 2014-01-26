using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace MCSIDE
{
    internal sealed class MCS_EngineInterface
    {
        [DllImport("MCS.dll")]
        public static extern void StartScriptEngine(); 

        [DllImport("MCS.dll")]
        public static extern bool ParseScript([MarshalAs(UnmanagedType.LPStr)] string file);

        [DllImport("MCS.dll")]
        public static extern bool ParseAndRunScript([MarshalAs(UnmanagedType.LPStr)] string file);

        [DllImport("MCS.dll")]
        public static extern bool ParseThenRunScript([MarshalAs(UnmanagedType.LPStr)] string file);

        [DllImport("MCS.dll")]
        public static extern bool RunScript([MarshalAs(UnmanagedType.LPStr)] string file);

        [DllImport("MCS.dll")]
        public static extern void Think(float currentTime);

        public delegate void OutputFunction([MarshalAs(UnmanagedType.LPStr)] string file);

        [DllImport("MCS.dll")]
        public static extern void SetErrorOutputFunction(OutputFunction func);

        [DllImport("MCS.dll")]
        public static extern void SetCommandOutputFunction(OutputFunction func);
    }
}
