using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ShortScriptCsharp
{ 
    public interface Syntax
    {
        dynamic Run(ref Dictionary<string, dynamic> local, ScriptRunner runner);
    }

    class For : Syntax
    {
        CodeData data;
        List<Syntax> codes;
        string counter;
        Expression first;
        Expression last;
        Expression step;
        public dynamic Run(ref Dictionary<string, dynamic> local,ScriptRunner runner)
        {
            dynamic first = this.first.ValueEval(local, runner);
            dynamic last = this.last.ValueEval(local, runner);
            dynamic step = this.step.ValueEval(local, runner);
            if (local.ContainsKey(counter))
                throw new ArgumentException(data.ExceptionMessage(string.Format("valuename {0} has been defined in this scopr", counter)));
            for (var d = first;d!= last;d+=step)
            {
                local[counter] = d;
                runner.Run(codes, local);
                d = local[counter];
            }
            local.Remove(counter);
            return null;
        }
        public For(CodeData data,List<Syntax>codes,string counter,Expression first,Expression last,Expression step)
        {
            this.data = data;
            this.codes = codes;
            this.counter = counter;
            this.first = first;
            this.last = last;
            this.step = step;
        }
    }

    class While : Syntax
    {
        CodeData data;
        List<Syntax> codes;
        Expression cond;

        public dynamic Run(ref Dictionary<string, dynamic> local, ScriptRunner runner)
        {
            var v = cond.ValueEval(local, runner);
            while(v)
            {
                runner.Run(codes, local);
                v = cond.ValueEval(local, runner);
            }
            return null;
        }

        public While(CodeData data,List<Syntax>codes,Expression cond)
        {
            this.data = data;
            this.codes = codes;
            this.cond = cond;
        }
    }

    class If:Syntax
    {
        CodeData data;
        IEnumerable<Tuple<Expression, List<Syntax>>> codes;
        List<Syntax> elsecode;

        public dynamic Run(ref Dictionary<string, dynamic> local, ScriptRunner runner)
        {
            foreach(var p in codes)
            {
                if (p.Item1.ValueEval(local, runner))
                    return runner.Run(p.Item2, local);
            }
            return runner.Run(elsecode, local);
        }

        public If(CodeData data, IEnumerable<Tuple<Expression, List<Syntax>>> codes,List<Syntax>elsecode)
        {
            this.data = data;
            this.codes = codes;
            this.elsecode = elsecode;
        }
    }

    class Global : Syntax
    {
        string name;
        Expression expr;
        public dynamic Run(ref Dictionary<string, dynamic> local, ScriptRunner runner)
        {
            runner.Global[name] = expr.ValueEval(local, runner);
            return null;
        }

        public Global(string name,Expression expr)
        {
            this.name = name;
            this.expr = expr;
        }
    }

    class Let : Syntax
    {
        string name;
        Expression expr;
        public dynamic Run(ref Dictionary<string, dynamic> local,ScriptRunner runner)
        {
            local[name] = expr.ValueEval(local, runner);
            return null;
        }

        public Let(string name,Expression expr)
        {
            this.name = name;
            this.expr = expr;
        }
    }

    class Return : Syntax
    {
        Expression expr;
        public dynamic Run(ref Dictionary<string, dynamic> local, ScriptRunner runner)
        {
            return expr.ValueEval(local, runner);
        }

        public Return(Expression expr)
        {
            this.expr = expr;
        }
    }

    class Expr : Syntax
    {
        Expression expr;
        public dynamic Run(ref Dictionary<string, dynamic> local, ScriptRunner runner)
        {
            expr.ValueEval(local, runner);
            return null;
        }

        public Expr(Expression expr)
        {
            this.expr = expr;
        }
    }
    
}
