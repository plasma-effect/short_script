using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ShortScriptCsharp
{
    public interface Expression
    {
        dynamic ValueEval(Dictionary<string, dynamic> local, ScriptRunner runner);
        string ToString();
        string ToString(int v);
    }

    class Value : Expression
    {
        string name;
        CodeData data;
        public dynamic ValueEval(Dictionary<string, dynamic> local, ScriptRunner runner)
        {
            dynamic val = 0;
            if (local.TryGetValue(name, out val)) return val;
            if (runner.Global.TryGetValue(name, out val)) return val;
            throw new ArgumentNullException(data.ExceptionMessage(string.Format("valuename {0} was not found in this scope.", name)));
        }

        public string ToString(int v)
        {
            string ret = "";
            for (int i = 0; i < v; ++i)
                ret += " ";
            ret += string.Format("valuename:{0}", name);
            return ret;
        }

        public override string ToString()
        {
            return ToString(-1);
        }

        public Value(string name,CodeData data)
        {
            this.name = name;
            this.data = data;
        }
    }

    class Literal : Expression
    {
        dynamic val;
        public dynamic ValueEval(Dictionary<string, dynamic> local, ScriptRunner runner)
        {
            return val;
        }

        public string ToString(int v)
        {
            string ret = "";
            for (int i = 0; i < v; ++i)
                ret += " ";
            ret += string.Format("literal:{0}", val);
            return ret;
        }

        public override string ToString()
        {
            return ToString(-1);
        }

        public Literal(dynamic val)
        {
            this.val = val;
        }
    }

    class Function : Expression
    {
        Func<dynamic[], CodeData, dynamic> func;
        Expression[] exprs;
        CodeData data;
        public dynamic ValueEval(Dictionary<string, dynamic> local, ScriptRunner runner)
        {
            var v = (from e in exprs select e.ValueEval(local, runner)).ToArray();
            return this.func(v, data);
        }

        public string ToString(int v)
        {
            string ret = "";
            for (int i = 0; i < v; ++i)
                ret += " ";
            ret += string.Format("function");
            foreach(var u in exprs)
            {
                ret += "\n" + u.ToString(v + 1);
            }
            return ret;
        }

        public override string ToString()
        {
            return ToString(-1);
        }

        public Function(Func<dynamic[],CodeData,dynamic> func,Expression[] exprs,CodeData data)
        {
            this.func = func;
            this.exprs = exprs;
            this.data = data;
        }
    }
}
