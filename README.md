#short scriptとは
言語仕様が小さいインタプリタ言語。いうてそんなに小さくない。  
組み込み関数を自由に定義してあなただけの最強のshort scriptが作れる。

#使うのに必要なもの
[plasma.ADT](https://github.com/plasma-effect/plasma_adt)とboostのパスを通しておく必要があります。  
あとはshort_script_core.hppをインクルードすれば使えます。

#とりあえず使ってみたい
```cpp
#include"short_script_core.hpp"
#include<iostream>
#include<fstream>

using namespace short_script_cpp;

int main(int argc,char const* argv[])
{
	std::fstream ss{ argv[1] };
	try
    {
		script_runner runner{ std::move(ss),get_default_command() };
		runner.main("main");
	}
	catch (std::exception& exp)
	{
		std::cout << exp.what() << std::endl;
	}
}
```
```
>filename.exe text.txt
```
text.txt内が実行される
#short script共通の言語仕様
基本的にLispと手続き型言語を足して2未満で割った感じの言語。
##let
```
let v 1
```
変数に値を代入する。第2引数以降を評価する。
##defとreturn
```
def func x
let v + x x
return v

def main
let x func 10
return
```
defで関数を定義する。returnで値を返す。ただしreturnで値を指定しない場合は0が返される。
##forとnext
```
def main
let v 0
for i 1 10
 let v + v i
next
let u 0
for i 2 10 2
 let u
next
return
```
第1引数の変数を第2引数から第3引数まで動かす。  
正確には第3引数になった瞬間に抜けるので第1引数の変数が第3引数になることはない。  
第4引数が指定されている場合その値ずつ、そうでない場合1ずつ増える。
##whileとloop
```
def main
let x 10
let v 0
while = x 0
let v + v x
let x - x 1
loop
returm
```
whileの中身がtrueの間loopまでをループし続ける。
##ifとelseとelifとendif
```
def main
let x 0
if = x 1
 let v "one"
elif = x 2
 let v "two"
else
 let v "other"
endif
return
```
ifの中身がtrueならその直後からelseもしくはelifまで、そうでない場合elseもしくはelifを探しelifならifからの手順を繰り返す。
endifでif文は終了する。
##変数の寿命について
letによる変数はその関数内でのみ使える。  
関数を超えた変数を使う場合はgrobalを使う。
```
def func
grobal x 1
return

def func2
grobal x + x 1
return

def main
func
func2
return
```
##コメント
```
def main
let x 1#これはコメントです
return
```
##関数の評価について
関数の評価は原則Lispと同様に行う。  
ただし関数の仮引数より実引数の方が多い場合、第(関数の仮引数の数)番より以後を評価した値をその引数とする。
```
let x + 1 + 2 + 3 4#+ 1 (+ 2 (+ 3 4))と同じ
```
##値の型について
short scriptは動的型付け言語であり全ての値はint型、double型、boolean型、string型のいずれかである。
##ランタイムエラーについて
script_runner型のコンストラクタでソースコードの読み込みを行い、mainメンバ関数で実行をするが、
関数の定義に関するエラーはコンストラクタから、それ以外のエラーはmainメンバ関数から例外(std::exceptionを継承)をエラーメッセージを内蔵して放出する。
上のようにtry~catchした方が良い。
#基本の組み込み関数
組み込み関数に関してはshort scriptを自分のアプリケーションに組み込みたい人が作ることもできる(後述)が、ある程度基本の関数も定義されている。
##operator+
足し算もしくは文字列の結合。 
```
let x + 1 2#3
let str + "hoge" "piyo"#"hogepiyo"
``` 
##operator- operator* operator/
引き算と掛け算と割り算。
```
let x - 2 1#1
let y * 3 4#12
let z / 8 4#2
```
##operator< operator> operator<= operator>= 
比較関数。同じ型でなければならない。
```
let x < 1 2#true
let x > 2 1#true
let x <= 1 2#true
let x >= 2 1#true

```
##operator= operator<>
equalとnot_equal。
```
let x = 1 1#true
let y <> 1 2#true
```
##print println
出力関数。printは改行を行わずprintlnは改行を行う。
```
print "1"
println "+1"
```
#組み込み関数の定義の仕方について
組み込み関数を定義する場合はmake_commandフリー関数を使う。make_command関数の引数は(Func&&, Ts const&...)である。
ただしFunc型はstd::function< value_type(dictionary< std::string, value_type>&,std::size_t)>型に暗黙変換できなければならず、
Ts型はそれぞれstd::string型に変換できなければならない。
##dictionary型について
std::mapみたいな型であるがフリー関数を使ってアクセスをする。実際はstd::vectorのusingエイリアスだが気にしてはいけない。
###dictionary_add
```cpp
dictionary<std::string, int> dic;
dictionary_add(dic, std::string("key"), 10);
```
要素の追加を行う。返り値はbool型である。trueが返された場合その要素がすでにあり上書きされたことを表す。
###dictionary_search
```cpp
dictionary<std::string, int> dic;
dictionary_add(dic, std::string("key"), 10);
auto v = dictionary_search(dic, std::string("key"));
```
要素の検索を行う。返り値はboost::optional型である。見つかった場合、及びその場合のみ返り値は有効となる。
##Ts型の制約
Ts型の引数はそれぞれshort_scriptの関数の仮引数となる。Func型のoperator()にはそれらをkeyとしたdictionaryと呼びだされた場所の行数-1が渡される。
##ランタイムエラーについて
ランタイムエラーが発生した場合例外を投げると便利。
自作することもできるがdetail::make_exceptionフリー関数を使うとより簡単にshort scriptのエラーっぽいメッセージが作れる。
またその場合boost::formatの形式で書くことも可能だが第1変数は行数表示で用いられる。
```cpp
[](dictionary<std::string, value_type>& dic, std::size_t line)
{
    auto v = *dictionary_search(dic, std::string("key"));
    if(v.type() == typeid(std::string))
        throw detail::make_exception<std::domain_error>(R"("key" is string)", line);
    return v;
}
```
#その他雑多なこと
・script_runner型のコンストラクタの引数はストリーム型(ソースコード用)とdictionary< std::string, script_command>型(コマンド用)で
共に右辺値参照。  
・script_runner::main関数の引数がエントリーポイントとなり、
第2引数にargc、第3引数にargvを指定してやるとargvが順にエントリーポイントの引数に渡されるが指定されなければ0が渡される  
・配列やリスト構造はない。