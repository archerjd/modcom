using System;
using System.Collections.Generic;
using System.Text;

namespace ConvarController
{
    interface IVarControl
    {
        bool Compressed { get; set; }
        bool AllowCompress { get; set; }
        System.Drawing.Size Size { get; set; }
    }
}
