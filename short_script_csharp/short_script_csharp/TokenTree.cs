using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ShortScriptCsharp
{
    public interface TokenTree
    {
        string GetToken();
        TokenTree[] GetTree();
        CodeData GetData();
        int GetLast();
        string ToString(int v);
    }

    public class Token : TokenTree
    {
        string token;
        CodeData data;
        int last;
        public TokenTree[] GetTree()
        {
            return null;
        }

        public string GetToken()
        {
            return token;
        }

        public CodeData GetData()
        {
            return data;
        }

        public int GetLast()
        {
            return last;
        }
        
        public string ToString(int v)
        {
            string ret = "";
            for (int i = 0; i < v; ++i)
                ret += " ";
            return ret + token + "\n";
        }

        public Token(string token,CodeData data)
        {
            this.token = token;
            this.data = data;
            this.last = data.Column + token.Length;
        }

        public Token(string str,CodeData data,int count)
        {
            for (int i = 1; i < str.Length; ++i) 
            {
                if (str[i + count] == '"')
                {
                    this.token = str.Substring(count, i + 1);
                    this.data = data;
                    this.last = count + i;
                    return;
                }
            }
            throw new Exception(data.ExceptionMessage("termination of string literal was not found"));
        }
    }

    public class Tree : TokenTree
    {
        TokenTree[] tree;
        CodeData data;
        int last;

        public TokenTree[] GetTree()
        {
            return tree;
        }

        public string GetToken()
        {
            return null;
        }

        public CodeData GetData()
        {
            return data;
        }

        public int GetLast()
        {
            return last;
        }

        public string ToString(int v)
        {
            string ret = "";
            for (int i = 0; i < v; ++i)
                ret += " ";
            ret += "----\n";
            foreach(var t in tree)
            {
                ret += t.ToString(v + 1);
            }
            return ret;
        }

        public Tree(string str,CodeData data,int count=0)
        {
            this.data = data;
            int column = data.Column;
            int line = data.Line;
            string filename = data.Filename;

            List<TokenTree> list = new List<TokenTree>();
            bool f = false;
            int fir = count;
            int las = count;

            for (int i = count; i < str.Length; ++i)
            {
                if(str[i]=='"')
                {
                    list.Add(new Token(str, new CodeData(line, i, data.Filename), i));
                    i = list.Last().GetLast();
                }
                else if(str[i]=='(')
                {
                    if (f) list.Add(new Token(str.Substring(fir, las - fir), new CodeData(line, fir, data.Filename)));
                    list.Add(new Tree(str, new CodeData(line, i, data.Filename), i + 1));
                    i = list.Last().GetLast();
                    f = false;
                }
                else if(str[i]==')')
                {
                    break;
                }
                else if(str[i]=='#')
                {
                    break;
                }
                else if(str[i]==' ')
                {
                    if (f) list.Add(new Token(str.Substring(fir, las - fir), new CodeData(line, fir, data.Filename)));
                    f = false;
                }
                else if(str[i]=='\t')
                {
                    if (f) list.Add(new Token(str.Substring(fir, las - fir), new CodeData(line, fir, data.Filename)));
                    f = false;
                }
                else if(f)
                {
                    ++las;
                }
                else
                {
                    f = true;
                    fir = i;
                    las = i + 1;
                }
            }
            if (f) list.Add(new Token(str.Substring(fir, las - fir), new CodeData(line, fir, data.Filename)));
            this.tree = list.ToArray();
            this.last = tree.Count() > 0 ? tree.Last().GetLast() + 1 : 0;
        }
        public Tree(TokenTree[] tree, CodeData data)
        {
            this.tree = tree;
            this.data = data;
            this.last = tree.Last().GetLast();
        }
    }

}
