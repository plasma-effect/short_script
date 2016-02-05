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

        public Function(Func<dynamic[],CodeData,dynamic> func,Expression[] exprs,CodeData data)
        {
            this.func = func;
            this.exprs = exprs;
            this.data = data;
        }
    }
}
