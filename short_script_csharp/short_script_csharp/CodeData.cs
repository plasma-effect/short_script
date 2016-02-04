using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace short_script_csharp
{
    public class CodeData
    {
        int line;
        int column;
        string filename;

        public int Line
        {
            get
            {
                return line;
            }
        }
        public int Column
        {
            get
            {
                return column;
            }
        }

        public string Filename
        {
            get
            {
                return filename;
            }
        }

        public CodeData(int line,int column,string filename)
        {
            this.line = line;
            this.column = column;
            this.filename = filename;
        }

        public string ExceptionMessage(string message)
        {
            return string.Format("{0} {1}-{2}: ", filename, line + 1, column + 1) + message;
        }
        static public string ExceptionMessage(string message, string filename, int line, int column)
        {
            return string.Format("{0} {1}-{2}: ", filename, line + 1, column + 1) + message;
        }
    }
}
