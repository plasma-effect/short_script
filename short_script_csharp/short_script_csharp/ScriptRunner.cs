using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace ShortScriptCsharp
{
    public class ScriptRunner
    {
        private Dictionary<string, dynamic> global;
        private Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>> function;
        private string filename;
        private List<Syntax> code;
        private Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command;
        private IEnumerable<Func<string, Expression>> literal_checker;
        private List<string> imported_filename;

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
        }

        public Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>> Functions
        {
            get
            {
                return function;
            }
        }

        public Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> SystemCommand
        {
            get
            {
                return system_command;
            }
        }

        public IEnumerable<Func<string, Expression>> LiteralChecker
        {
            get
            {
                return literal_checker;
            }
        }

        public List<string> ImportedFilename
        {
            get
            {
                return imported_filename;
            }
        }

        public dynamic Run(IEnumerable<Syntax> syntaxs, Dictionary<string, dynamic> local)
        {
            foreach(var s in syntaxs)
            {
                var r = s.Run(ref local, this);
                if (r != null)
                    return r;
            }
            return null;
        }

        /// <summary>
        /// ファイル名を指定してshort scriptのコードをコンパイルします
        /// </summary>
        /// <param name="filename">ロードするファイル名です</param>
        /// <param name="system_command">コンパイル時に使う特殊コマンドです</param>
        /// <param name="function">デフォルトで定義された関数です</param>
        /// <param name="literal_checker">リテラルであるかどうかチェックする関数です</param>
        public ScriptRunner(
            string filename,
            Dictionary<string,Func<dynamic[],ScriptRunner,dynamic>> system_command,
            Dictionary<string,Tuple<int,Func<dynamic[],CodeData,dynamic>>> function,
            IEnumerable<Func<string,Expression>> literal_checker)
        {
            this.filename = filename;
            this.function = function;
            this.global = new Dictionary<string, dynamic>();
            this.code = new List<Syntax>();
            this.system_command = system_command;
            this.literal_checker = literal_checker;
            this.imported_filename = new List<string> { filename };
            int i = 0;

            List<TokenTree> trees = new List<TokenTree>();
            using (var stream = new StreamReader(filename))
            {
                while(stream.Peek()>=0)
                {
                    var str = stream.ReadLine();
                    var tree = new Tree(str, new CodeData(i, 0, this.Filename));
                    trees.Add(tree);
                    ++i;
                }
            }
            this.code = SyntaxParse(trees, filename, system_command, literal_checker).Item1;
            
        }

        public ScriptRunner(
            TextReader reader,
            string filename,
            Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command,
            Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>> function,
            IEnumerable<Func<string, Expression>> literal_checker)
        {
            this.filename = filename;
            this.function = function;
            this.global = new Dictionary<string, dynamic>();
            this.code = new List<Syntax>();
            this.system_command = system_command;
            this.literal_checker = literal_checker;
            int i = 0;
            List<TokenTree> trees = new List<TokenTree>();
            while(reader.Peek()>=0)
            {
                var str = reader.ReadLine();
                var tree = new Tree(str, new CodeData(i, 0, this.Filename));
                trees.Add(tree);
                ++i;
            }
            this.code = SyntaxParse(trees, filename, system_command, literal_checker).Item1;
        }

        enum ReturnFlag
        {
            None, Else, Elif, Endif,Next,Loop
        }

        Tuple<List<Syntax>,int,ReturnFlag> SyntaxParse(
            IEnumerable<TokenTree> trees,
            string filename,
            Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command,
            IEnumerable<Func<string,Expression>> literal_checker)
        {
            List<Syntax> code = new List<Syntax>();
            int i = 0;
            int skip = 0;
            foreach (var token in trees)
            {
                ++i;
                if(skip>0)
                {
                    --skip;
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
                    throw new Exception(CodeData.ExceptionMessage("invalid command", filename, i, 0));
                }
                Func<dynamic[], ScriptRunner, dynamic> sys;
                if (system_command.TryGetValue(com, out sys))
                {
                    var ar = from e in vec.Skip(1) select ExpressionParse(e, literal_checker);
                    var v = from e in ar select e.ValueEval(new Dictionary<string, dynamic>(), this);
                    sys(v.ToArray(), this);
                    continue;
                }
                if (com == "for") SyntaxParseFor(ref i, ref skip, code, token, vec, trees, filename, system_command, literal_checker);

                else if (com == "next") return Tuple.Create(code, i, ReturnFlag.Next);
                else if (com == "while") SyntaxParsingWhile(ref i, ref skip, code, token, vec, trees, filename, system_command, literal_checker);
                else if (com == "loop") return Tuple.Create(code, i, ReturnFlag.Loop);
                else if (com == "if") SyntaxParsingIf(ref i, ref skip, code, token, vec, trees, filename, system_command, literal_checker);
                else if (com == "elif") return Tuple.Create(code, i, ReturnFlag.Elif);
                else if (com == "else") return Tuple.Create(code, i, ReturnFlag.Else);
                else if (com == "endif") return Tuple.Create(code, i, ReturnFlag.Endif);
                else if (com == "def") SyntaxParsingDef(ref i, ref skip, code, token, vec, trees, filename, system_command, literal_checker);
                else if (com == "let") SyntaxParsingLet(ref i, ref skip, code, token, vec, trees, filename, system_command, literal_checker);
                else if (com == "global") SyntaxParsingGlobal(ref i, ref skip, code, token, vec, trees, filename, system_command, literal_checker);
                else if (com == "return")
                {
                    Expression expr = vec.Length == 1 ? new Literal(null) : ExpressionParse(new Tree(vec.Skip(1).ToArray(), vec[1].GetData()), literal_checker);
                    code.Add(new Return(expr));
                }
                else code.Add(new Expr(ExpressionParse(token, literal_checker)));
            }
            return Tuple.Create(code, i, ReturnFlag.None);
        }

        void SyntaxParseFor(ref int i, ref int skip, List<Syntax> code, TokenTree token, TokenTree[] vec, IEnumerable<TokenTree> trees, string filename, Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command, IEnumerable<Func<string,Expression>> literal_checker)
        {
            if (vec.Length < 4)
                throw new Exception(token.GetData().ExceptionMessage("for parameter is too few"));
            var valname = vec[1].GetToken();
            if (valname == null)
                throw new Exception(token.GetData().ExceptionMessage("invalid value name"));
            var first = ExpressionParse(vec[2], literal_checker);
            var last = ExpressionParse(vec[3], literal_checker);
            var step = vec.Length > 4 ? ExpressionParse(new Tree(vec.Skip(4).ToArray(), vec[4].GetData()), literal_checker) : new Literal(1);
            var c = SyntaxParse(trees.Skip(i), filename, system_command, literal_checker);
            if (c.Item3 != ReturnFlag.Next)
            {
                throw new Exception(token.GetData().ExceptionMessage("next corresponding to the for can not be found"));
            }
            code.Add(new For(token.GetData(), c.Item1, valname, first, last, step));
            skip += c.Item2;
        }

        void SyntaxParsingWhile(ref int i, ref int skip, List<Syntax> code, TokenTree token, TokenTree[] vec, IEnumerable<TokenTree> trees, string filename, Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command, IEnumerable<Func<string,Expression>> literal_checker)
        {
            if (vec.Length == 1)
                throw new Exception(token.GetData().ExceptionMessage("while parameter is too few"));
            var expr = ExpressionParse(new Tree(vec.Skip(1).ToArray(), vec[1].GetData()), literal_checker);
            var c = SyntaxParse(trees.Skip(i), filename, system_command, literal_checker);
            if (c.Item3 != ReturnFlag.Loop)
            {
                throw new Exception(token.GetData().ExceptionMessage("endif corresponding to the if can not be found"));
            }
            code.Add(new While(token.GetData(), c.Item1, expr));
            skip += c.Item2;
        }

        void SyntaxParsingIf(ref int i, ref int skip, List<Syntax> code, TokenTree token, TokenTree[] vec, IEnumerable<TokenTree> trees, string filename, Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command, IEnumerable<Func<string,Expression>> literal_checker)
        {
            if (vec.Length == 1)
                throw new Exception(token.GetData().ExceptionMessage("if parameter is too few"));
            var syns = new List<Tuple<Expression, List<Syntax>>>();
            while (true)
            {
                var v = SyntaxParse(trees.Skip(i + skip), filename, system_command, literal_checker);
                var expr = ExpressionParse(new Tree(trees.ElementAt(i + skip - 1).GetTree().Skip(1).ToArray(), vec[1].GetData()), literal_checker);

                if (v.Item3 == ReturnFlag.Elif)
                {
                    syns.Add(Tuple.Create(expr, v.Item1));
                    skip += v.Item2;
                    if (trees.ElementAt(i + skip - 1).GetTree().Length == 1) 
                    {
                        throw new Exception(token.GetData().ExceptionMessage("elif parameter is too few"));
                    }
                }
                else if (v.Item3 == ReturnFlag.Else)
                {
                    syns.Add(Tuple.Create(expr, v.Item1));
                    skip += v.Item2;
                    var u = SyntaxParse(trees.Skip(i + skip), filename, system_command, literal_checker);
                    if (u.Item3 != ReturnFlag.Endif)
                    {
                        throw new Exception(token.GetData().ExceptionMessage("endif corresponding to the if can not be found"));
                    }
                    code.Add(new If(token.GetData(), syns, u.Item1));
                    skip += u.Item2;
                    break;
                }
                else if (v.Item3 == ReturnFlag.Endif)
                {
                    syns.Add(Tuple.Create(expr, v.Item1));
                    code.Add(new If(token.GetData(), syns, new List<Syntax>()));
                    skip += v.Item2;
                    break;
                }
                else
                {
                    throw new Exception(token.GetData().ExceptionMessage("endif corresponding to the if can not be found"));
                }
            }
        }

        void SyntaxParsingDef(ref int i, ref int skip, List<Syntax> code, TokenTree token, TokenTree[] vec, IEnumerable<TokenTree> trees, string filename, Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command, IEnumerable<Func<string,Expression>> literal_checker)
        {
            if (vec.Length == 1)
                throw new Exception(token.GetData().ExceptionMessage("def parameter is too few"));
            var funcname = vec[1].GetToken();
            if (funcname == null)
                throw new Exception(token.GetData().ExceptionMessage("invalid function name"));
            List<string> names = new List<string>();
            var index = code.Count;

            foreach (var t in vec.Skip(2))
            {
                var str = t.GetToken();
                if (str == null)
                    throw new Exception(t.GetData().ExceptionMessage("invalid function parameter"));
                names.Add(str);
            }
            Func<dynamic[], CodeData, dynamic> func = (dy, data) =>
            {
                Dictionary<string, dynamic> local = new Dictionary<string, dynamic>();
                for (int u = 0; u < names.Count; ++u)
                {
                    local[names[u]] = dy[u];
                }
                return this.Run(code.Skip(index), local);
            };
            function.Add(funcname, Tuple.Create(vec.Length - 2, func));
        }

        void SyntaxParsingLet(ref int i, ref int skip, List<Syntax> code, TokenTree token, TokenTree[] vec, IEnumerable<TokenTree> trees, string filename, Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command, IEnumerable<Func<string,Expression>> literal_checker)
        {
            if (vec.Length < 3)
                throw new Exception(token.GetData().ExceptionMessage("let argument is too few"));
            var name = vec[1].GetToken();
            if (name == null)
                throw new Exception(token.GetData().ExceptionMessage("invalid value name"));
            code.Add(new Let(name, ExpressionParse(new Tree(vec.Skip(2).ToArray(), vec[2].GetData()), literal_checker)));
        }

        void SyntaxParsingGlobal(ref int i, ref int skip, List<Syntax> code, TokenTree token, TokenTree[] vec, IEnumerable<TokenTree> trees, string filename, Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> system_command, IEnumerable<Func<string,Expression>> literal_checker)
        {
            if (vec.Length < 3)
                throw new Exception(token.GetData().ExceptionMessage("global argument is too few"));
            var name = vec[1].GetToken();
            if (name == null)
                throw new Exception(token.GetData().ExceptionMessage("invalid value name"));
            code.Add(new Global(name, ExpressionParse(new Tree(vec.Skip(2).ToArray(), vec[2].GetData()), literal_checker)));
        }



        Expression ExpressionParse(
            TokenTree tree,
            IEnumerable<Func<string,Expression>> literal_checker)
        {
            var val = tree.GetToken();
            var vec = tree.GetTree();
            if (val != null)
            {
                foreach (var func in literal_checker)
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
                    throw new Exception(tree.GetData().ExceptionMessage("null expression is invalid"));
                var name = vec[0].GetToken();
                if (name == null)
                    throw new Exception(tree.GetData().ExceptionMessage("invalid expression"));
                if (vec.Length == 1)
                    return ExpressionParse(vec[0], literal_checker);

                Tuple<int, Func<dynamic[], CodeData, dynamic>> set;
                if (!function.TryGetValue(name, out set))
                    throw new Exception(tree.GetData().ExceptionMessage(string.Format("function {0} was not found", name)));
                if (set.Item1 >= vec.Length)
                    throw new Exception(tree.GetData().ExceptionMessage(string.Format("function {0} requires {1} argument, but provided was {2}", name, set.Item1, vec.Length - 1)));
                if (set.Item1 != -1)
                {
                    var exprs = new Expression[set.Item1];
                    for (int i = 1; i < set.Item1; ++i)
                    {
                        exprs[i - 1] = ExpressionParse(vec[i], literal_checker);
                    }
                    exprs[set.Item1 - 1] = ExpressionParse(new Tree(vec.Skip(set.Item1).ToArray(), vec[set.Item1].GetData()), literal_checker);
                    return new Function(set.Item2, exprs, tree.GetData());
                }
                else
                {
                    return new Function(set.Item2, (from e in vec.Skip(1) select ExpressionParse(e, literal_checker)).ToArray(), tree.GetData());
                }
            }
        }

        /// <summary>
        /// エントリーポイントから実行します
        /// </summary>
        /// <param name="entry_point">エントリーポイントです</param>
        /// <param name="argument">エントリーポイントに渡される引数です</param>
        /// <returns>実行の結果の返り値です</returns>
        public dynamic ScriptRun(string entry_point, dynamic[] argument)
        {
            Tuple<int, Func<dynamic[], CodeData, dynamic>> v;
            if (!function.TryGetValue(entry_point, out v))
            {
                throw new Exception(CodeData.ExceptionMessage(string.Format("entrypoint {0} was not found", entry_point), filename, 0, 0));
            }
            if (v.Item1 >= 3 || v.Item1 < 0)
            {
                throw new Exception(CodeData.ExceptionMessage(string.Format("entry point {0} parameter is invalid"), filename, 0, 0));
            }
            return v.Item2(new dynamic[] { argument, argument.Length }, new CodeData(0, 0, Filename));
        }
    }
}
