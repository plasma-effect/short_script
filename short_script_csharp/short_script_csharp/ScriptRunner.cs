using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace short_script_csharp
{
    public class ScriptRunner
    {
        private Dictionary<string, dynamic> global;
        private Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>> function;
        private string filename;
        private List<Syntax> code;

        public string Filename
        {
            get
            {
                return filename;
            }
        }

        public Dictionary<string, dynamic> Global
        {
            get
            {
                return global;
            }

            set
            {
                global = value;
            }
        }

        public dynamic Run(IEnumerable<Syntax> syntaxs, Dictionary<string, dynamic> local)
        {
            foreach(var s in syntaxs)
            {
                var r = s.Run(local, this);
                if (r != null)
                    return r;
            }
            return null;
        }

        public ScriptRunner(
            string filename,
            string entry_point,
            Dictionary<string,Func<dynamic[],ScriptRunner,dynamic>> system_command,
            Dictionary<string,Tuple<int,Func<dynamic[],CodeData,dynamic>>> function,
            Func<string,Expression>[] literal_checker)
        {
            this.filename = filename;
            this.function = function;
            this.global = new Dictionary<string, dynamic>();
            this.code = new List<Syntax>();
            int i = 0;

            List<TokenTree> trees = new List<TokenTree>();
            using (var stream = new StreamReader(filename))
            {
                while(stream.Peek()>=0)
                {
                    var str = stream.ReadLine();
                    trees.Add(new Tree(str, new CodeData(i, 0, this.Filename)));
                }
            }
            this.code = SyntaxParse(trees, filename, entry_point, system_command, function, literal_checker).Item1;
        }

        public Tuple<List<Syntax>,int> SyntaxParse(
            IEnumerable<TokenTree> trees,
            string filename,
            string entry_point,
            Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command,
            Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>> function,
            Func<string, Expression>[] literal_checker)
        {
            List<Syntax> code = new List<Syntax>();
            int i = 0;
            int skip = 0;
            foreach(var token in trees)
            {
                ++i;
                if(skip>0)
                {
                    --skip;;
                    continue;
                }
                var vec = token.GetTree();

                if (vec.Length == 0)
                {
                    continue;
                }
                var com = vec[0].GetToken();
                if (com == null)
                {
                    throw new ArgumentException(CodeData.ExceptionMessage("invalid command", filename, i, 0));
                }
                Func<dynamic[], ScriptRunner, dynamic> sys;
                if (system_command.TryGetValue(com, out sys))
                {
                    var ar = from e in vec select ExpressionParse(e, literal_checker);
                    var v = from e in ar select e.ValueEval(new Dictionary<string, dynamic>(), this);
                    sys(v.ToArray(), this);
                    continue;
                }
                if (com == "for")
                {
                    if (vec.Length < 4)
                        throw new ArgumentException(token.GetData().ExceptionMessage("for parameter is too few"));
                    var valname = vec[1].GetToken();
                    if (valname == null)
                        throw new ArgumentException(token.GetData().ExceptionMessage("invalid value name"));
                    var first = ExpressionParse(vec[2], literal_checker);
                    var last = ExpressionParse(vec[3], literal_checker);
                    var step = vec.Length > 4 ? ExpressionParse(new Tree(vec.Skip(4).ToArray(), vec[4].GetData()), literal_checker) : new Literal(1);
                    var c = SyntaxParse(trees.Skip(i), filename, entry_point, system_command, function, literal_checker);
                    code.Add(new For(token.GetData(), c.Item1, valname, first, last, step));
                    skip = c.Item2;
                    continue;
                }
                if(com=="next")
                {
                    return new Tuple<List<Syntax>, int>(code, i);
                }
                if(com=="while")
                {
                    if(vec.Length==1)
                        throw new ArgumentException(token.GetData().ExceptionMessage("while parameter is too few"));
                    var expr = ExpressionParse(new Tree(vec.Skip(1).ToArray(), vec[1].GetData()), literal_checker);
                    var c = SyntaxParse(trees.Skip(i), filename, entry_point, system_command, function, literal_checker);
                    code.Add(new While(token.GetData(), c.Item1, expr));
                    skip = c.Item2;
                    continue;
                }
            }
        }

        public Expression ExpressionParse(
            TokenTree tree,
            Func<string, Expression>[] literal_checker)
        {
            var val = tree.GetToken();
            var vec = tree.GetTree();
            if (val != null) 
            {
                foreach(var func in literal_checker)
                {
                    var expr = func(val);
                    if (expr != null)
                        return expr;
                }
                Tuple<int, Func<dynamic[], CodeData, dynamic>> set;
                if (function.TryGetValue(val, out set) && set.Item1 == 0)
                    return new Function(set.Item2, new Expression[0], tree.GetData());
                return new Value(val, tree.GetData());
            }
            else
            {
                if (vec.Length == 0)
                    throw new ArgumentException(tree.GetData().ExceptionMessage("null expression is invalid"));
                var name = vec[0].GetToken();
                if (name == null)
                    throw new ArgumentException(tree.GetData().ExceptionMessage("invalid expression"));
                Tuple<int, Func<dynamic[], CodeData, dynamic>> set;
                if (!function.TryGetValue(name, out set))
                    throw new ArgumentException(tree.GetData().ExceptionMessage(string.Format("function {0} was not found", name)));
                if (set.Item1 >= vec.Length)
                    throw new ArgumentException(tree.GetData().ExceptionMessage(string.Format("function {0} requires {1} argument, but provided was {2}", name, set.Item1, vec.Length - 1)));
                var exprs = new Expression[set.Item1];
                for (int i = 1; i < set.Item1 - 1; ++i)
                {
                    exprs[i - 1] = ExpressionParse(vec[i], literal_checker);
                }
                exprs[set.Item1 - 1] = ExpressionParse(new Tree(vec.Skip(set.Item1).ToArray(), vec[set.Item1].GetData()), literal_checker);
                return new Function(set.Item2, exprs,tree.GetData());
            }
        }
    }
}
