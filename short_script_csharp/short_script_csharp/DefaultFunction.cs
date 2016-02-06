using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ShortScriptCsharp
{
    public class DefaultFunction
    {
        private static Tuple<int, Func<dynamic[], CodeData, dynamic>> MakeTuple(int v, Func<dynamic[], CodeData, dynamic> func)
        {
            return new Tuple<int, Func<dynamic[], CodeData, dynamic>>(v, func);
        }

        public static dynamic OperatorPlus(dynamic[] arg,CodeData data)
        {
            dynamic ret = arg[0];
            try
            {
                foreach (var v in arg.Skip(1))
                {
                    ret += v;
                }
            }
            catch(Exception)
            {
                throw new Exception(data.ExceptionMessage("operator+ error(dont use those types)"));
            }
            return ret;
        }

        public static dynamic OperatorMulti(dynamic[] arg, CodeData data)
        {
            dynamic ret = arg[0];
            try
            {
                foreach (var v in arg.Skip(1))
                {
                    ret *= v;
                }
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator* error(dont use with those types)"));
            }
            return ret;
        }

        public static dynamic OperatorMinus(dynamic[]arg,CodeData data)
        {
            try
            {
                return arg[0] - arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator- error(dont use with those types)"));
            }
        }

        public static dynamic OperatorDiv(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] / arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator/ error(dont use with those types)"));
            }
        }

        public static dynamic OperatorMod(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] % arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator% error(dont use with those types)"));
            }
        }
        
        public static Dictionary<string,Tuple<int,Func<dynamic[],CodeData,dynamic>>> Get5BasicOperator()
        {
            var ret = new Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>>();
            ret.Add("+", MakeTuple(-1, OperatorPlus));
            ret.Add("-", MakeTuple(2, OperatorMinus));
            ret.Add("*", MakeTuple(-1, OperatorMulti));
            ret.Add("/", MakeTuple(2, OperatorDiv));
            ret.Add("%", MakeTuple(2, OperatorMod));
            return ret;
        }

        public static dynamic OperatorLeftShift(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] << arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator<< error(dont use with those types)"));
            }
        }

        public static dynamic OperatorRightShift(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] >> arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator>> error(dont use with those types)"));
            }
        }

        public static dynamic OperatorBitOr(dynamic[] arg, CodeData data)
        {
            dynamic ret = arg[0];
            try
            {
                foreach(var v in arg.Skip(1))
                {
                    ret |= v;
                }
                return ret;
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator| error(dont use with those types)"));
            }
        }

        public static dynamic OperatorBitAnd(dynamic[] arg, CodeData data)
        {
            dynamic ret = arg[0];
            try
            {
                foreach (var v in arg.Skip(1))
                {
                    ret &= v;
                }
                return ret;
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator& error(dont use with those types)"));
            }
        }

        public static dynamic OperatorBitXor(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] ^ arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator^ error(dont use with those types)"));
            }
        }

        public static dynamic OperatorBitNot(dynamic[] arg, CodeData data)
        {
            try
            {
                return !arg[0];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator! error(dont use with those types)"));
            }
        }

        public static Dictionary<string,Tuple<int,Func<dynamic[],CodeData,dynamic>>> Get6BitOperator()
        {
            var ret = new Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>>();
            ret.Add("<<", MakeTuple(2, OperatorLeftShift));
            ret.Add(">>", MakeTuple(2, OperatorRightShift));
            ret.Add("|", MakeTuple(-1, OperatorBitOr));
            ret.Add("&", MakeTuple(-1, OperatorBitAnd));
            ret.Add("^", MakeTuple(2, OperatorBitXor));
            ret.Add("!", MakeTuple(1, OperatorBitNot));
            return ret;
        }

        public static dynamic OperatorEqual(dynamic[] arg, CodeData data)
        {
            dynamic v = arg[0];
            try
            {
                foreach(var u in arg.Skip(1))
                {
                    if (v != u)
                        return false;
                }
                return true;
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator= error(dont use with those types)"));
            }
        }

        public static dynamic OperatorNotEqual(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] != arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator<> error(dont use with those types)"));
            }
        }

        public static dynamic OperatorLessThan(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] < arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator< error(dont use with those types)"));
            }
        }

        public static dynamic OperatorLessEqual(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] <= arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator<= error(dont use with those types)"));
            }
        }

        public static dynamic OperatorLargerThan(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] > arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator> error(dont use with those types)"));
            }
        }

        public static dynamic OperatorLargerEqual(dynamic[] arg, CodeData data)
        {
            try
            {
                return arg[0] <= arg[1];
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator>= error(dont use with those types)"));
            }
        }

        public static dynamic OperatorLogicAnd(dynamic[] arg, CodeData data)
        {
            try
            {
                foreach (var v in arg)
                {
                    if (!v)
                        return false;
                }
                return true;
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator&& error(dont use with those types)"));
            }
        }

        public static dynamic OperatorLogicOr(dynamic[] arg, CodeData data)
        {
            try
            {
                foreach(var v in arg)
                {
                    if (v)
                        return true;
                }
                return false;
            }
            catch (Exception)
            {
                throw new Exception(data.ExceptionMessage("operator|| error(dont use with those types)"));
            }
        }

        public static Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>> Get8LogicOperator()
        {
            var ret = new Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>>();
            ret.Add("=", MakeTuple(-1, OperatorEqual));
            ret.Add("<>", MakeTuple(2, OperatorNotEqual));
            ret.Add("<", MakeTuple(2, OperatorLessThan));
            ret.Add("<=", MakeTuple(2, OperatorLessEqual));
            ret.Add(">", MakeTuple(2, OperatorLargerThan));
            ret.Add(">=", MakeTuple(2, OperatorLargerEqual));
            ret.Add("&&", MakeTuple(-1, OperatorLogicAnd));
            ret.Add("||", MakeTuple(-1, OperatorLogicOr));
            return ret;
        }

        public static dynamic MakeArray(dynamic[] arg,CodeData data)
        {
            return arg;
        }

        public static dynamic ArrayAccess(dynamic[] arg,CodeData data)
        {
            try
            {
                return arg[0][arg[1]];
            }
            catch(Exception exp)
            {
                throw new Exception(data.ExceptionMessage("at error(privided error type)"));
            }
        }

        public static Dictionary<string,Tuple<int,Func<dynamic[],CodeData,dynamic>>> GetArrayTraits()
        {
            var ret = new Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>>();
            ret.Add("array", MakeTuple(-1, MakeArray));
            ret.Add("at", MakeTuple(2, ArrayAccess));
            return ret;
        }

        public static dynamic Print(dynamic[] arg,CodeData data)
        {
            Console.Write(arg[0]);
            return null;
        }

        public static dynamic PrintLine(dynamic[] arg,CodeData data)
        {
            Console.WriteLine(arg[0]);
            return null;
        }
        
        public static dynamic IntParse(dynamic[] arg,CodeData data)
        {
            try
            {
                return int.Parse(arg[0]);
            }
            catch(Exception)
            {
                throw new Exception(data.ExceptionMessage("invalid argument type"));
            }
        }

        public static dynamic TypeName(dynamic[]arg,CodeData data)
        {
            return arg[0].GetType().Name;
        }

        public static Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>> GetUtilityFunction()
        {
            var ret = new Dictionary<string, Tuple<int, Func<dynamic[], CodeData, dynamic>>>();
            ret.Add("print", MakeTuple(1, Print));
            ret.Add("println", MakeTuple(1, PrintLine));
            ret.Add("int.parse", MakeTuple(1, IntParse));
            ret.Add("typename", MakeTuple(1, TypeName));
            return ret;
        }

        public static Dictionary<string,Tuple<int,Func<dynamic[],CodeData,dynamic>>> GetDefaultFunction()
        {
            var ret = Get5BasicOperator();
            ret = ret.Concat(Get6BitOperator()).ToDictionary(x => x.Key, x => x.Value);
            ret = ret.Concat(Get8LogicOperator()).ToDictionary(x => x.Key, x => x.Value);
            ret = ret.Concat(GetArrayTraits()).ToDictionary(x => x.Key, x => x.Value);
            ret = ret.Concat(GetUtilityFunction()).ToDictionary(x => x.Key, x => x.Value);
            return ret;
        }

        public static dynamic Import(dynamic[] arg,ScriptRunner runner)
        {
            if (arg[0].GetType() != typeof(string))
                throw new Exception("import error(invalid argument)");
            if (runner.ImportedFilename.Exists(str => str == arg[0]))
                return null;
            var inner = new ScriptRunner(arg[0], runner.SystemCommand, runner.Functions, runner.LiteralChecker);
            foreach (var v in inner.Functions)
            {
                if (!runner.Functions.ContainsKey(v.Key))
                    runner.Functions.Add(v.Key, v.Value);

            }
            runner.ImportedFilename.AddRange(inner.ImportedFilename);
            return null;
        }

        public static Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>> GetDefaultCommand()
        {
            var ret = new Dictionary<string, Func<dynamic[], ScriptRunner, dynamic>>();
            ret.Add("import", Import);
            return ret;
        }

        public static Expression IntegerCheck(string str)
        {
            int x;
            if (int.TryParse(str, out x))
                return new Literal(x);
            return null;
        }

        public static Expression StringCheck(string str)
        {
            if (str.First() == '"' && str.Last() == '"')
                return new Literal(str.Substring(1, str.Length - 2));
            return null;
        }

        public static Expression BooleanCheck(string str)
        {
            if (str == "true")
                return new Literal(true);
            if (str == "false")
                return new Literal(false);
            return null;
        }

        public static Expression DoubleCheck(string str)
        {
            double v;
            if (double.TryParse(str, out v))
                return new Literal(v);
            return null;
        }

        public static List<Func<string,Expression>> GetDefaultLiteral()
        {
            var ret = new List<Func<string, Expression>>();
            ret.Add(IntegerCheck);
            ret.Add(StringCheck);
            ret.Add(BooleanCheck);
            ret.Add(DoubleCheck);
            return ret;
        }
    }
}
