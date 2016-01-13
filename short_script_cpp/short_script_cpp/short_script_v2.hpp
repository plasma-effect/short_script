#pragma once

#include<plasma_adt/algebraic_data_type.hpp>

#include<iostream>
#include<regex>
#include<functional>
#include<stack>
#include<algorithm>
#include<utility>

#include<boost/optional.hpp>
#include<boost/variant.hpp>
#include<boost/any.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/format.hpp>


namespace short_script_v2
{
	struct script_runner;
	struct system_command;
	struct static_command;
	struct script_command;
	typedef boost::any value_type;
	typedef boost::variant<int, double, std::string, bool> const_value_type;
	typedef unsigned int count_t;

	namespace utility
	{
		template<class T>T& at(std::vector<T>& vec, std::size_t i)
		{
#ifdef _DEBUG
			return vec.at(i);
#else
			return vec[i];
#endif
		}
		template<class T>T const& at(std::vector<T> const& vec, std::size_t i)
		{
#ifdef _DEBUG
			return vec.at(i);
#else
			return vec[i];
#endif
		}

		template<class Key, class Value>using dictionary = std::vector<std::pair<Key, Value>>;
		template<class Key, class Value, class K, class V>bool dictionary_add(dictionary<Key, Value>& dic, K&& key, V&& value)
		{
			for (auto& p : dic)
			{
				if (p.first == Key(key))
				{
					p.second = std::forward<V>(value);
					return true;
				}
			}
			dic.push_back(std::make_pair<Key, Value>(Key(key), Value(value)));
			return false;
		}
		template<class Key, class Value, class K>boost::optional<Value> dictionary_search(dictionary<Key, Value>const& dic, K const& key)
		{
			Key k = key;
			for (auto const& p : dic)
			{
				if (p.first == k)
				{
					return p.second;
				}
			}
			return boost::none;
		}
		template<class Key, class Value, class K>boost::optional<std::reference_wrapper<Value>> dictionary_ref_search(dictionary<Key, Value>& dic, K const& key)
		{
			Key k = key;
			for (auto& p : dic)
			{
				if (p.first == k)
				{
					return std::ref(p.second);
				}
			}
			return boost::none;
		}

		template<class T>struct type_holder {};

		inline boost::format make_format(boost::format format)
		{
			return format;
		}
		template<class T, class... Ts>boost::format make_format(boost::format format, T const& arg, Ts const&... args)
		{
			return make_format(format%arg, args...);
		}

		template<class Exception, class... Ts>Exception make_exception(std::string message, std::string filename, count_t line, count_t column, Ts const&... args)
		{
			return Exception(
				make_format(
					boost::format(
						(boost::format(R"(short script error: %1% %2%-%3% )") % filename%line%column).str() + message)
					,args...).str());
		}

		
		inline const_value_type translate_to_const(value_type const& value)
		{
			if (value.type() == typeid(int))
				return boost::any_cast<int>(value);
			if (value.type() == typeid(double))
				return boost::any_cast<double>(value);
			if (value.type() == typeid(std::string))
				return boost::any_cast<std::string>(value);
			if (value.type() == typeid(bool))
				return boost::any_cast<bool>(value);
			throw std::domain_error("type error");
		}
		inline value_type translate_to_runtime(const_value_type const& value)
		{
			return boost::apply_visitor([](auto const& v) {return value_type(v);}, value);
		}

		template<class T>T get_and_pop(std::stack<T>& s)
		{
			auto ret = s.top();
			s.pop();
			return ret;
		}
	}
	inline value_type run(script_runner& runner, count_t line, utility::dictionary<std::string, value_type>& local);

	//define part
	namespace syntax_tree
	{
		using namespace plasma_adt;
		typedef std::string::const_iterator string_iterator;
		struct token_tree :data_type_base<token_tree,
			tuple<std::vector<token_tree>, count_t>,
			tuple<std::string, count_t>
		>
		{
			token_tree(data_type_base const& d) :data_type_base(d) {}
		};
		namespace
		{
			const auto TokenTree = token_tree::instance_function<0>();
			const auto TokenString = token_tree::instance_function<1>();
		}
		struct expression : data_type_base<expression,
			tuple<int, count_t>,//Integer
			tuple<double, count_t>,//Double
			tuple<std::string, count_t>,//String
			tuple<bool, count_t>,//Boolean
			tuple<std::reference_wrapper<script_command>, std::vector<expression>, count_t>,//ScriptCommand
			tuple<std::reference_wrapper<static_command>, std::vector<expression>, count_t>,//StaticCommand
			tuple<std::string, count_t>//Value
		>
		{
			expression(data_type_base const& d) :data_type_base(d) {}
		};
		namespace 
		{
			const auto Integer = expression::instance_function<0>();
			const auto Double = expression::instance_function<1>();
			const auto String = expression::instance_function<2>();
			const auto Boolean = expression::instance_function<3>();
			const auto ScriptCommand = expression::instance_function<4>();
			const auto StaticCommand = expression::instance_function<5>();
			const auto Value = expression::instance_function<6>();
		}
		struct syntax :data_type_base<syntax,
			tuple<std::string, expression>,//Let
			tuple<std::string, expression, expression, expression>,//For
			void,//Next
			tuple<expression>,//While
			void,//Loop
			tuple<expression>,//If
			tuple<expression>,//Elif
			void,//Else
			void,//Endif
			tuple<expression>,//Return
			void,//Null
			tuple<expression>>//Expr
		{
			syntax(data_type_base const&d) :data_type_base(d) {}
		};
		namespace
		{
			const auto Let = syntax::instance_function<0>();
			const auto For = syntax::instance_function<1>();
			const auto Next = syntax::instance_function<2>();
			const auto While = syntax::instance_function<3>();
			const auto Loop = syntax::instance_function<4>();
			const auto If = syntax::instance_function<5>();
			const auto Elif = syntax::instance_function<6>();
			const auto Else = syntax::instance_function<7>();
			const auto Endif = syntax::instance_function<8>();
			const auto Return = syntax::instance_function<9>();
			const auto Null = syntax::instance_function<10>();
			const auto Expr = syntax::instance_function<11>();
		}
	}

	struct script_runner
	{
		std::vector<syntax_tree::syntax> code;
		utility::dictionary<std::string, static_command> static_commands;
		utility::dictionary<std::string, script_command> script_commands;
		utility::dictionary<std::string, value_type> global_values;
		std::string filename;
	};

	struct system_command
	{
		std::vector<std::string> arguments;
		std::function<script_runner&(script_runner&, utility::dictionary<std::string, const_value_type>&&, count_t)> func;
		script_runner& operator()(script_runner& runner, std::vector<const_value_type> val, count_t line)
		{
			utility::dictionary<std::string, const_value_type> vals;
			for (std::size_t i = 0;i < arguments.size();++i)
			{
				utility::dictionary_add(vals, utility::at(arguments, i), utility::at(val, i));
			}
			return func(runner, std::move(vals), line);
		}
		std::size_t size()const
		{
			return arguments.size();
		}
	};
	
	struct static_command
	{
		std::vector<std::string> arguments;
		std::function<const_value_type(script_runner&, utility::dictionary<std::string, const_value_type>&&, count_t, count_t)> func;
		const_value_type operator()(script_runner& runner, std::vector<const_value_type> val, count_t line, count_t column)
		{
			utility::dictionary<std::string, const_value_type> vals;
			for (std::size_t i = 0;i < arguments.size();++i)
			{
				utility::dictionary_add(vals, utility::at(arguments, i), utility::at(val, i));
			}
			return func(runner, std::move(vals), line, column);
		}
		std::size_t size()const
		{
			return arguments.size();
		}
	};
	
	struct script_command
	{
		std::vector<std::string> arguments;
		std::function<value_type(script_runner&, utility::dictionary<std::string, value_type>&&, count_t, count_t)> func;
		value_type operator()(script_runner& runner, std::vector<value_type> val, count_t line, count_t column)
		{
			utility::dictionary<std::string, value_type> vals;
			for (std::size_t i = 0;i < arguments.size();++i)
			{
				utility::dictionary_add(vals, utility::at(arguments, i), utility::at(val, i));
			}
			return func(runner, std::move(vals), line, column);
		}
		std::size_t size()const
		{
			return arguments.size();
		}
	};

	namespace command_traits
	{
		template<class T>struct named_argument
		{
			std::string name;
		};
		template<class T>named_argument<T> make_named_argument(std::string name)
		{
			return named_argument<T>{name};
		}
		
		namespace detail
		{
			template<class T> T get_argument(
				script_runner& runner,
				utility::dictionary<std::string, value_type>const& vals,
				named_argument<T> named, count_t line, count_t column)
			{
				if (auto v = utility::dictionary_search(vals, named.name))
				{
					auto const& u = *v;
					return u.type() == typeid(T) ? boost::any_cast<T>(u) : throw utility::make_exception<std::invalid_argument>("function argument type error", runner.filename, line, column);
				}
				throw utility::make_exception<std::domain_error>(R"(internal script error(report to this app publisher))", runner.filename, line, column);
			}

			template<class T> T get_argument(
				script_runner& runner,
				utility::dictionary<std::string, const_value_type>const& vals,
				named_argument<T> named, count_t line, count_t column)
			{
				if (auto v = utility::dictionary_search(vals, named.name))
				{
					auto const& u = *v;
					return u.type() == typeid(T) ? boost::get<T>(u) : throw utility::make_exception<std::invalid_argument>("function argument type error", runner.filename, line, column);
				}
				throw utility::make_exception<std::domain_error>(R"(internal script error(report to this app publisher))", runner.filename, line, column);
			}

			template<class Return, class... Args>
			std::function<value_type(script_runner&, utility::dictionary<std::string, value_type>&&, count_t, count_t)>
				make_script_function(std::function<Return(script_runner&, count_t, count_t, Args...)> func, named_argument<Args>const&... named)
			{
				return [=](script_runner& runner, utility::dictionary<std::string, value_type>&& val, count_t line, count_t column)
				{
					return value_type(func(runner, line, column, get_argument(runner, val, named, line, column)...));
				};
			}

			template<class Return, class... Args>
			std::function<const_value_type(script_runner&, utility::dictionary<std::string, const_value_type>&&, count_t, count_t)>
				make_static_function(std::function<Return(script_runner&, count_t, count_t, Args...)> func, named_argument<Args>const&... named)
			{
				return [=](script_runner& runner, utility::dictionary<std::string, const_value_type>&& val, count_t line, count_t column)
				{
					return const_value_type(func(runner, line, column, get_argument(runner, val, named, line, column)...));
				};
			}
		}
		template<class Return,class Func, class... Args>static_command make_static_command(
			Func func, 
			named_argument<Args> const&... args)
		{
			return static_command{ 
				std::vector<std::string>{args.name...}, 
				detail::make_static_function(
					static_cast<std::function<Return(script_runner&, count_t, count_t, Args...)>>(func),args...) };
		}

		template<class Return, class Func, class... Args>script_command make_script_command(
			Func func,
			named_argument<Args> const&... args)
		{
			return script_command{
				std::vector<std::string>{args.name...},
				detail::make_script_function(
					static_cast<std::function<Return(script_runner&, count_t, count_t, Args...)>>(func),args...) };
		}


	}




	//function part
	namespace syntax_tree
	{
		namespace detail
		{
			struct make_token_tree_t
			{
				count_t line;
				std::string const& filename;
				string_iterator top;
				string_iterator now;
				string_iterator last;

				void push_back(std::vector<token_tree>& vec, token_tree t)
				{
					vec.push_back(t);
				}

				token_tree make_token_tree()
				{
					std::vector<token_tree> ret;
					string_iterator s = now;
					string_iterator e = now;
					count_t const start = std::distance(top, now);
					bool f = false;
					bool mode = false;

					for (; now != last;++now)
					{
						char c = *now;
						
						if (mode)
						{
							if (c == '\"')
							{
								++now;
								push_back(ret, TokenString(std::string(s, now), std::distance(top, s)));
								mode = false;
								f = false;
							}
						}
						else if (c == '(')
						{
							if (auto d = std::distance(s, e))
								push_back(ret, TokenString(std::string(s, e), d));
							++now;
							ret.push_back(make_token_tree());
							f = false;
							s = e = now;
						}
						else if (c == ')' || c == '#')
						{
							break;
						}
						else if (c == '\"')
						{
							if (auto d = std::distance(s, e))
								push_back(ret, TokenString(std::string(s, e), d));
							s = now;
							mode = true;
						}
						else if (c == ' ')
						{
							f = false;
						}
						else if (c == '\t')
						{
							f = false;
						}
						else if (f)
						{
							++e;
						}
						else
						{
							if (auto d = std::distance(s, e))
								push_back(ret, TokenString(std::string(s, e), d));
							f = true;
							s = now;
							e = std::next(s);
						}
					}
					if (mode)
						throw utility::make_exception<std::domain_error>(R"(missing terminating '"' charactor)", filename, line, std::distance(top, s));

					if (auto d = std::distance(s, e))
						push_back(ret, TokenString(std::string(s, e), d));
					return TokenTree(ret, start);
				}
			};
		}
		token_tree make_token_tree(std::string str, count_t line, std::string filename)
		{
			detail::make_token_tree_t runner{ line, filename, std::begin(str), std::begin(str), std::end(str) };
			return runner.make_token_tree();
		}

		namespace detail
		{
			using namespace plasma_adt::place_holder;
			namespace
			{
				const auto GetToken = plasma_adt::pattern_match::pattern_match<
					boost::optional<std::pair<std::string, count_t>>,
					token_tree>()
					| TokenString(0_, 1_) <= [](std::string const& str, count_t column) {return std::make_pair(str, column);}
					| TokenTree <= []() {return static_cast<boost::optional<std::pair<std::string, count_t>>>(boost::none);};

				const auto GetTopToken = plasma_adt::pattern_match::pattern_match<
					boost::optional<std::pair<std::string, count_t>>,
					token_tree>()
					| TokenTree(0_, 1_) <= [](std::vector<token_tree>const& t, count_t)
						{
							return t.size() == 0 ?
								static_cast<boost::optional<std::pair<std::string, count_t>>>(boost::none) :
								GetToken(utility::at(t, 0));
						}
					| TokenString <= []() {return static_cast<boost::optional<std::pair<std::string, count_t>>>(boost::none);};

				const auto GetVector = plasma_adt::pattern_match::pattern_match<
					boost::optional<std::vector<token_tree>>,
					token_tree>()
					| TokenTree(0_, 1_) <= [](std::vector<token_tree> const& v, count_t column)
						{
						return boost::make_optional(v);
						}
					| TokenString <= []() {return static_cast<boost::optional<std::vector<token_tree>>>(boost::none);};

				const auto GetColumn = plasma_adt::pattern_match::pattern_match<
					count_t,
					token_tree>()
					| TokenTree(0_, 1_) <= [](auto const&, count_t column) {return column;}
					| TokenString(0_, 1_) <= [](auto const&, count_t column) {return column;};

				const auto GetLiteral = plasma_adt::pattern_match::pattern_match<
					boost::optional<const_value_type>,
					expression>()
					| Integer(0_, 1_) <= [](auto v, count_t) {return boost::make_optional(const_value_type(v));}
					| Double(0_, 1_) <= [](auto v, count_t) {return boost::make_optional(const_value_type(v));}
					| String(0_, 1_) <= [](auto const& v, count_t) {return boost::make_optional(const_value_type(v));}
					| Boolean(0_, 1_) <= [](auto v, count_t) {return boost::make_optional(const_value_type(v));}
					| 0_ <= [](auto const&) {return static_cast<boost::optional<const_value_type>>(boost::none);};
			
					const auto print_ = plasma_adt::pattern_match::recursion_match<int, token_tree, int>()
						| TokenTree(0_, 1_) <= [](auto f, std::vector<token_tree>const& vec, count_t, int count)
					{
						for (int i = 0;i < count;++i)
							std::cout << " ";
						std::cout << "----" << std::endl;
						for (auto const& v : vec)
						{
							f(v, count + 2);
						}
						return 0;
					}
						| TokenString(0_, 1_) <= [](auto, std::string const& str, count_t, int count)
					{
						for (int i = 0;i < count;++i)
						{
							std::cout << " ";
						}
						std::cout << str << std::endl;
						return 0;
					};
			}
			namespace 
			{
				inline expression make_literal(const_value_type v, count_t column)
				{
					if (v.type() == typeid(int))
						return Integer(boost::get<int>(v), column);
					if (v.type() == typeid(double))
						return Double(boost::get<double>(v), column);
					if (v.type() == typeid(std::string))
						return String(boost::get<std::string>(v), column);

					return Boolean(boost::get<bool>(v), column);
				}

				inline boost::optional<expression> literal_check(token_tree const& ttree)
				{
					auto i = GetToken(ttree);
					if (!i)
						return boost::none;
					auto const& str = (*i).first;
					auto const column = (*i).second;
					if (std::regex_match(str, std::basic_regex<char>(R"(-?\d+)")))
						return Integer(boost::lexical_cast<int>(str), column);
					if (std::regex_match(str, std::basic_regex<char>(R"([+-]?((\d*.\d+)|(\d+.\d*))|(\d+[eE][-+]\d+))")))
						return Double(boost::lexical_cast<double>(str), column);
					if (std::regex_match(str, std::basic_regex<char>(R"(\".+\")")))
						return String(str.substr(1, str.size() - 2), column);
					if (str == "true")
						return Boolean(true, column);
					if (str == "false")
						return Boolean(false, column);
					return boost::none;
				}

				inline expression static_eval(
					script_runner& runner,
					std::reference_wrapper<static_command> com,
					std::vector<expression> expr, count_t line, count_t column)
				{
					std::vector<const_value_type> val;
					for (auto const& e : expr)
					{
						if (auto v = GetLiteral(e))
						{
							val.push_back(*v);
						}
						else
						{
							return StaticCommand(com, expr, column);
						}
					}
					return make_literal(com.get().operator()(runner, std::move(val), line, column), column);
				}

				expression translation_expression(
					token_tree const& ttree, 
					utility::dictionary<std::string, const_value_type>const& static_values,
					script_runner& runner,
					count_t line)
				{

					if (auto t = literal_check(ttree))return *t;
					if (auto v = GetToken(ttree))
					{
						if (auto u = utility::dictionary_search(static_values, v->first))
						{
							return make_literal(*u, v->second);
						}
						if (auto c = utility::dictionary_ref_search(runner.static_commands, v->first))
						{
							auto& r = (*c).get();
							if (r.size() == 0)
							{
								return make_literal(r(runner, std::vector<const_value_type>{}, line, GetColumn(ttree)), GetColumn(ttree));
							}
							throw utility::make_exception<std::domain_error>(R"("%1%" requires more equal %2% argument, but 0 were provided)", runner.filename, line, GetColumn(ttree), v->first, r.size());
						}
						if (auto c = utility::dictionary_ref_search(runner.script_commands, v->first))
						{
							auto& r = (*c).get();
							if (r.size() == 0)
							{
								return ScriptCommand(r, std::vector<expression>{}, GetColumn(ttree));
							}
							throw utility::make_exception<std::domain_error>(R"("%1%" requires more equal %2% argument, but 0 were provided)", runner.filename, line, GetColumn(ttree), v->first, r.size());
						}
						return Value((*v).first, (*v).second);
					}
					auto const v = *GetVector(ttree);
					if (v.size() == 0)
					{
						throw utility::make_exception<std::domain_error>(R"(expect expression)", runner.filename, line, GetColumn(ttree));
					}
					if (auto c = GetToken(utility::at(v, 0)))
					{
						auto p = *c;
						if (auto u = utility::dictionary_ref_search(runner.static_commands, p.first))
						{
							auto c = u->get().size();
							if (v.size() == 1)
							{
								return c == 0 ?
									make_literal(u->operator()(runner, std::vector<const_value_type>{}, line, GetColumn(ttree)), GetColumn(ttree)) :
									throw utility::make_exception<std::domain_error>(R"("%1%" requires more equal %2% argument, but 0 were provided)", runner.filename, line, GetColumn(ttree), p.first, c);
							}
							else if (v.size() > c + 1)
							{
								std::vector<expression> exprs;
								for (std::size_t i = 1;i < c + 1;++i)
								{
									exprs.push_back(translation_expression(utility::at(v, i), static_values, runner, line));
								}
								exprs.push_back(
									translation_expression(
										TokenTree(std::vector<token_tree>(std::next(v.begin(), c + 1), std::end(v)),
											GetColumn(utility::at(v, c + 1))),
										static_values, runner, line));
								return static_eval(runner, *u, exprs, line, GetColumn(utility::at(v, 0)));
							}
							else if (v.size() == c + 1)
							{
								std::vector<expression> exprs;
								for (std::size_t i = 1;i < c + 1;++i)
								{
									exprs.push_back(translation_expression(utility::at(v, i), static_values, runner, line));
								}
								return static_eval(runner, *u, exprs, line, GetColumn(utility::at(v, 0)));
							}
							throw utility::make_exception<std::domain_error>(R"("%1%" requires more equal %2% argument, but %3% were provided)", runner.filename, line, GetColumn(ttree), p.first, c, v.size() - 1);
						}
						else if (auto u = utility::dictionary_ref_search(runner.script_commands, p.first))
						{
							auto c = u->get().size();

							if (v.size() == 1)
							{
								return c == 0 ?
									ScriptCommand(*u, std::vector<expression>{}, GetColumn(ttree)):
									throw utility::make_exception<std::domain_error>(R"("%1%" requires more equal %2% argument, but 0 were provided)", runner.filename, line, GetColumn(ttree), p.first, c);
							}
							else if (v.size() > c + 1)
							{
								std::vector<expression> exprs;
								for (std::size_t i = 1;i < c + 1;++i)
								{
									exprs.push_back(translation_expression(utility::at(v, i), static_values, runner, line));
								}
								exprs.push_back(
									translation_expression(
										TokenTree(std::vector<token_tree>(std::next(v.begin(), c + 1), std::end(v)),
											GetColumn(utility::at(v, c + 1))),
										static_values, runner, line));
								return ScriptCommand(*u, exprs, GetColumn(ttree));
							}
							else if (v.size() == c + 1)
							{
								std::vector<expression> exprs;
								for (int i = 1;i == c + 1;++i)
								{
									exprs.push_back(translation_expression(utility::at(v, i), static_values, runner, line));
								}
								return ScriptCommand(*u, exprs, GetColumn(ttree));
							}
							throw utility::make_exception<std::domain_error>(R"("%1%" requires more equal %2% argument, but %3% were provided)", runner.filename, line, GetColumn(ttree), p.first, c, v.size() - 1);
						}
						else
						{
							throw utility::make_exception<std::domain_error>(R"("%1%" is not function)", runner.filename, line, GetColumn(ttree), p.first);
						}
					}
					else
					{
						throw utility::make_exception<std::domain_error>(R"(error function)", runner.filename, line, GetColumn(ttree));
					}
				}

				inline script_runner& system_eval(
					script_runner& runner,
					system_command com,
					std::vector<expression> expr, count_t line, count_t column)
				{
					std::vector<const_value_type> val;
					for (auto const& e : expr)
					{
						if (auto v = GetLiteral(e))
						{
							val.push_back(*v);
						}
						else
						{
							throw utility::make_exception<std::invalid_argument>(R"(system command argument must be constant value)", runner.filename, line, column);
						}
					}
					return com(runner, std::move(val), line);
				}

				syntax translation_syntax(
					token_tree const& ttree,
					utility::dictionary<std::string, system_command> const& sys_commands,
					utility::dictionary<std::string, const_value_type>& static_values,
					script_runner& runner,
					count_t line)
				{
					auto vec = *GetVector(ttree);
					if (vec.size() == 0)
						return Null();
					if (auto ci = GetToken(utility::at(vec, 0)))
					{
						if (auto com = utility::dictionary_search(sys_commands, ci->first))
						{
							auto s = com->size();
							if (vec.size() <= s)
							{
								throw utility::make_exception<std::domain_error>(R"("%1%" requires more equal %2% argument, but %3% were provided)", runner.filename, line, GetColumn(ttree), ci->first, s, vec.size() - 1);
							}
							else if (vec.size() == s + 1)
							{
								std::vector<expression> expr;
								for (std::size_t i = 1;i < s + 1;++i)
								{
									expr.push_back(translation_expression(utility::at(vec, i), static_values, runner, line));
								}
								system_eval(runner, *com, std::move(expr), line, GetColumn(ttree));
							}
							else
							{
								std::vector<expression> expr;
								for (std::size_t i = 1;i < s;++i)
								{
									expr.push_back(translation_expression(utility::at(vec, i), static_values, runner, line));
								}
								expr.push_back(
									translation_expression(
										TokenTree(std::vector<token_tree>(std::next(std::begin(vec), s), std::end(vec)),
										GetColumn(utility::at(vec, s))),
										static_values, runner, line));
								system_eval(runner, *com, std::move(expr), line, GetColumn(ttree));
							}
							return Null();
						}
						if (ci->first == "def")
						{
							if (vec.size() == 1)
								throw utility::make_exception<std::domain_error>(R"(expect function name)", runner.filename, line, GetColumn(ttree));
							auto name = GetToken(utility::at(vec, 1));
							if(!name)
								throw utility::make_exception<std::domain_error>(R"(invalid function name)", runner.filename, line, GetColumn(ttree));
							std::vector<std::string> arg;
							for (int i = 2;i < vec.size();++i)
							{
								auto v = GetToken(utility::at(vec, i));
								if (!v)
									throw utility::make_exception<std::domain_error>(R"(invalid function name)", runner.filename, line, GetColumn(utility::at(vec, i)));
								arg.push_back(v->first);
							}
							utility::dictionary_add(runner.script_commands,name->first,script_command
							{
								arg,
								[](script_runner& runner, utility::dictionary<std::string, value_type>&& vals, count_t line, count_t column)
								{
									return run(runner, line + 1, std::move(vals));
								}
							});
							return Null();
						}
						else if (ci->first == "const")
						{
							if(vec.size()==1)
								throw utility::make_exception<std::domain_error>(R"(expect constant value name)", runner.filename, line, GetColumn(ttree));
							if(vec.size()==2)
								throw utility::make_exception<std::domain_error>(R"(expect initialize value)", runner.filename, line, GetColumn(ttree));
							auto name = GetToken(utility::at(vec, 1));
							if(!name)
								throw utility::make_exception<std::domain_error>(R"(invalid constant value name)", runner.filename, line, GetColumn(ttree));
							if (auto v = GetLiteral(translation_expression(TokenTree(std::vector<token_tree>(std::next(std::begin(vec), 2), std::end(vec)), GetColumn(utility::at(vec, 2))), static_values, runner, line)))
							{
								utility::dictionary_add(static_values, name->first, *v);
								return Null();
							}
							throw utility::make_exception<std::domain_error>(R"(constant value must be initialized by constant value)", runner.filename, line, GetColumn(ttree));
						}
						else if (ci->first == "let")
						{
							if (vec.size() == 1)
								throw utility::make_exception<std::domain_error>(R"(expect value name)", runner.filename, line, GetColumn(ttree));
							if (vec.size() == 2)
								throw utility::make_exception<std::domain_error>(R"(expect assigned value)", runner.filename, line, GetColumn(ttree));
							auto name = GetToken(utility::at(vec, 1));
							return Let(name->first, translation_expression(
								TokenTree(std::vector<token_tree>(std::next(std::begin(vec), 2), std::end(vec)),
								GetColumn(utility::at(vec,2))), 
								static_values, runner, line));
						}
						else if (ci->first == "for")
						{
							if (vec.size() < 4)
								throw utility::make_exception<std::domain_error>(R"(invalid command)", runner.filename, line, GetColumn(ttree));
							auto name = GetToken(utility::at(vec, 1));
							if (!name)
								throw utility::make_exception<std::domain_error>(R"(invalid value name)", runner.filename, line, GetColumn(ttree));
							return For(name->first,
								translation_expression(utility::at(vec, 2), static_values, runner, line),
								translation_expression(utility::at(vec, 3), static_values, runner, line),
								vec.size() == 4 ? Integer(1, 0) :
								vec.size() == 5 ? translation_expression(utility::at(vec, 4), static_values, runner, line) :
								translation_expression(TokenTree(std::vector<token_tree>(std::next(std::begin(vec), 4), std::end(vec)), GetColumn(utility::at(vec, 4))), static_values, runner, line));

						}
						else if (ci->first == "next")
						{
							return Next();
						}
						else if (ci->first == "while")
						{
							if(vec.size()==1)
								throw utility::make_exception<std::domain_error>(R"(invalid command)", runner.filename, line, GetColumn(ttree));
							return While(
								vec.size() == 2 ? translation_expression(utility::at(vec, 1), static_values, runner, line) :
								translation_expression(TokenTree(std::vector<token_tree>(std::next(std::begin(vec), 1), std::end(vec)), GetColumn(utility::at(vec, 1))), static_values, runner, line));
						}
						else if (ci->first == "loop")
						{
							return Loop();
						}
						else if (ci->first == "if")
						{
							if(vec.size()==1)
								throw utility::make_exception<std::domain_error>(R"(invalid command)", runner.filename, line, GetColumn(ttree));
							return If(
								vec.size() == 2 ? 
								translation_expression(utility::at(vec, 1), static_values, runner, line) :
								translation_expression(
									TokenTree(std::vector<token_tree>(std::next(std::begin(vec), 1), std::end(vec)),
										GetColumn(utility::at(vec, 1))), static_values, runner, line));
						}
						else if (ci->first == "elif")
						{
							if (vec.size() == 1)
								throw utility::make_exception<std::domain_error>(R"(invalid command)", runner.filename, line, GetColumn(ttree));
							return Elif(
								vec.size() == 2 ?
								translation_expression(utility::at(vec, 1), static_values, runner, line) :
								translation_expression(
									TokenTree(std::vector<token_tree>(std::next(std::begin(vec), 1), std::end(vec)),
										GetColumn(utility::at(vec, 1))), static_values, runner, line));
						}
						else if (ci->first == "else")
						{
							return Else();
						}
						else if (ci->first == "endif")
						{
							return Endif();
						}
						else if (ci->first == "return")
						{	
							return Return(
								vec.size() == 1 ? Integer(0, 0) :
								vec.size() == 2 ?
								translation_expression(utility::at(vec, 1), static_values, runner, line) :
								translation_expression(
									TokenTree(std::vector<token_tree>(std::next(std::begin(vec), 1), std::end(vec)),
										GetColumn(utility::at(vec, 1))), static_values, runner, line));
						}
						else
						{
							return Expr(translation_expression(ttree, static_values, runner, line));
						}
					}
				}
			}
			
		}

	}
}