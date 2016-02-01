#pragma once

#ifndef WANDBOX_TEST
#include<plasma_adt/generic_data_type.hpp>
#else
#include"generic_data_type.hpp"
#endif

#include<iostream>
#include<regex>
#include<functional>
#include<stack>
#include<algorithm>
#include<utility>
#include<typeinfo>
#include<typeindex>
#include<tuple>

#include<boost/optional.hpp>
#include<boost/variant.hpp>
#include<boost/any.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/format.hpp>
#include<boost/range/adaptors.hpp>


namespace short_script_v2
{
	using namespace generic_adt;
	using namespace place_holder;

	typedef unsigned int column_t;
	typedef unsigned int line_t;
	typedef unsigned int count_t;
	typedef std::string filename_t;
	template<class Type>using vector_iterator = typename std::vector<Type>::iterator;

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

		template<class Iterator>struct iterator_range_t
		{
			Iterator first;
			Iterator last;
			Iterator begin()const
			{
				return first;
			}
			Iterator end()const
			{
				return last;
			}
		};
		template<class Iterator>iterator_range_t<Iterator> iterator_range(Iterator first, Iterator last)
		{
			return iterator_range_t<Iterator>{first, last};
		}
	}

	namespace token_traits
	{
		struct token_tree_base :generic_data_type<token_tree_base,
			tuple<std::string, column_t>, 
			std::vector<token_tree_base>> {};
		const auto Token = token_tree_base::instance_function<0>();
		const auto Tree = token_tree_base::instance_function<1>();
		const auto make_token = Token(type_tag<std::string>{});
		const auto make_tree = Tree(type_tag<std::string>{});
		typedef std::shared_ptr<token_tree_base::value_type<std::string>> token_tree;
		namespace detail
		{
			token_tree make_token_tree_impl(
				std::string::iterator const& base,
				std::string::iterator& first,
				std::string::iterator const& last,
				std::string const& filename,
				line_t line,
				bool string_flag = false)
			{
				if (string_flag)
				{
					std::string::iterator fir = first;
					for (;first != last;++first)
					{
						if (*first == '\"')
							return make_token(std::string{ std::prev(fir),std::next(first) }, std::distance(base, fir) - 1);
					}
					throw utility::make_exception<std::domain_error>(R"(not found termination of string token)", filename, line, std::distance(base, fir));
				}
				else
				{
					bool token_flag = false;
					std::string::iterator fir = first;
					std::string::iterator las = first;
					std::vector<token_tree> ret;
					for (;first != last;++first)
					{
						char c = *first;
						if (c == '\"')
						{
							if (token_flag)
								ret.push_back(make_token(std::string{ fir,las }, std::distance(base, fir)));
							ret.push_back(make_token_tree_impl(
								base,
								++first,
								last,
								filename,
								line,
								true));
						}
						else if (c == '(')
						{
							if (token_flag)
								ret.push_back(make_token(std::string{ fir,las }, std::distance(base, fir)));
							fir = first;
							ret.push_back(make_token_tree_impl(base, ++first, last, filename, line));
							token_flag = false;
							
							if (first == last)
								throw utility::make_exception<std::domain_error>(R"(not found termination of "(")", filename, line, std::distance(base, fir));
						}
						else if (c == ')' || c == '#')
						{
							break;
						}
						else if (c == ' ' || c == '\t')
						{
							if (token_flag)
								ret.push_back(make_token(std::string{ fir,las }, std::distance(base, fir)));
							token_flag = false;
						}
						else if (!token_flag)
						{
							token_flag = true;
							fir = first;
							las = std::next(first);
						}
						else
						{
							++las;
						}
					}
					if (token_flag)
						ret.push_back(make_token(std::string{ fir,las }, std::distance(base, fir)));
					return make_tree(ret);
				}
			}
		}

		template<class Stream>std::vector<token_tree> make_token_tree(Stream&& stream, std::string filename)
		{
			line_t line = 1;
			std::string str;
			std::vector<token_tree> ret{};
			while (std::getline(stream, str))
			{
				auto first = str.begin();
				auto ite = str.begin();
				auto last = str.end();
				ret.push_back(detail::make_token_tree_impl(first, ite, last, filename, line));
			}
			return ret;
		}

		namespace detail 
		{
			const auto PrintTree = pattern_match::generic_recursion<int, token_tree_base, int>() 
				| Token(0_, 1_) <= [](auto, auto, std::string const& str, column_t c, int v)
			{
				for (int i = 0;i < v;++i)
					std::cout << "  ";
				std::cout << str << std::endl;
				return 0;
			} 
				| Tree(0_) <= [](auto recur, auto, std::vector<token_tree>const& vec, int v)
			{
				for (int i = 0;i < v;++i)
					std::cout << "  ";
				std::cout << "----" << std::endl;
				for (auto const& d : vec)
				{
					recur(d, v + 1);
				}
				return 0;
			};
		}
		void print_tree(token_tree const& t)
		{
			detail::PrintTree(t, -1);
		}

		const auto get_token = pattern_match::generic_match<boost::optional<std::pair<std::string, column_t>>,token_tree_base>()
			| Token(0_, 1_) <= [](auto, std::string const& str, column_t column) {return boost::make_optional(std::make_pair(str, column));}
			| Tree <= [](auto) {return static_cast<boost::optional<std::pair<std::string, column_t>>>(boost::none);};

		const auto get_tree = pattern_match::generic_match<boost::optional<std::reference_wrapper<std::vector<token_tree_base> const>>,token_tree_base>()
			| Tree(0_) <= [](auto, std::vector<token_tree_base>const& vec) {return boost::make_optional(std::cref(vec));}
			| Token <= [](auto) {return static_cast<boost::optional<std::reference_wrapper<std::vector<token_tree_base> const>>>(boost::none);};
	}

	template<class... Ts>struct short_script_runner;
	template<class ValueType>struct system_command;
	template<class... Ts>struct system_command<boost::variant<Ts...>>
	{
		typedef boost::variant<Ts...> value_type;
		typedef short_script_runner<Ts...> runner;
		std::function<runner&(std::vector<value_type>&&, runner&, line_t)> func;
		std::vector<std::type_index> types;
		std::string command_name;
		runner& call(std::vector<value_type>&& val, runner& target, line_t line, filename_t const& filename)const
		{
			if (types.size() != val.size())
				throw utility::make_exception<std::invalid_argument>(R"(system command "%1%" requires %2% arguments, but %3% were provided)", filename, line, 0, command_name, types.size(), val.size());
			for (int i = 0;i < val.size();++i)
				if (types[i] != val[i].type())
					throw utility::make_exception<std::invalid_argument>(R"(provided argument types were wrong)", filename, line, 0);
			return func(std::move(val), target, line);
		}
	};
	template<class ValueType>struct embedded_script_command;
	template<class... Ts>struct embedded_script_command<boost::variant<Ts...>>
	{
		typedef boost::variant<Ts...> value_type;
		std::vector<std::type_index> types;
		std::function<value_type(std::vector<value_type>, filename_t const&, column_t, line_t, short_script_runner<Ts...>&)> func;
		bool can_static_call_;
		bool type_check(std::vector<value_type>const& vec)const
		{
			if (vec.size() != types.size())
				return false;
			for (int i = 0;i < vec.size();++i)
			{
				if (vec[i].type() != types[i])
					return false;
			}
			return true;
		}
		value_type call(std::vector<value_type>&& value, filename_t const& filename, column_t column, line_t line, short_script_runner<Ts...>& runner)const
		{
			return func(std::move(value), filename, column, line, runner);
		}
		bool can_static_call()const
		{
			return can_static_call_;
		}
	};
	template<class ValueType>struct variadic_script_command;
	template<class... Ts>struct variadic_script_command<boost::variant<Ts...>>
	{
		typedef boost::variant<Ts...> value_type;
		std::type_index type;
		std::function<value_type(std::vector<value_type>, filename_t const&, column_t, line_t, short_script_runner<Ts...>&)> func;
		bool can_static_call_;
		bool type_check(std::vector<value_type>const& vec)const
		{
			for (auto const& v : vec)
			{
				if (v.type() != type)
					return false;
			}
			return true;
		}
		value_type call(std::vector<value_type>&& value, filename_t const& filename, column_t column, line_t line, short_script_runner<Ts...>& runner)const
		{
			return func(std::move(value), filename, column, line, runner);
		}
		bool can_static_call()const
		{
			return can_static_call_;
		}
	};
	template<class ValueType>struct user_defined_command;
	template<class... Ts>struct user_defined_command<boost::variant<Ts...>>
	{
		typedef boost::variant<Ts...> value_type;
		std::function<value_type(std::vector<value_type>, filename_t const&, column_t, line_t, short_script_runner<Ts...>&)> func;

		bool type_check(std::vector<value_type>const&)const
		{
			return true;
		}

		value_type call(std::vector<value_type>&& value, filename_t const& filename, column_t column, line_t line, short_script_runner<Ts...>& runner)const
		{
			return func(std::move(value), filename, column, line, runner);
		}

		bool can_static_call()const
		{
			return false;
		}
	};
	template<class ValueType>using script_command = boost::variant<
		embedded_script_command<ValueType>,
		variadic_script_command<ValueType>,
		user_defined_command<ValueType>>;
	template<class Generic>struct runner_type;
	template<class... Ts>struct runner_type<boost::variant<Ts...>>
	{
		typedef short_script_runner<Ts...> type;
	};
	template<class Generic>using runner_t = typename runner_type<Generic>::type;
	template<class Generic>bool type_check(
		script_command<Generic>const& v, 
		std::vector<value_type>const& val)
	{
		return boost::apply_visitor([&](auto const& u) {return u.type_check(val);}, v);
	}

	template<class Generic>Generic call(script_command<Generic>const& v, std::vector<Generic>&& value, filename_t const& filename, column_t column, line_t line, runner_t<Generic>& runner)
	{
		return boost::apply_visitor([&](auto const& u) {return u.type_check(std::move(value), filename, column, line, runner);}, v);
	}

	template<class Generic>bool can_static_call(script_command<Generic>const& v)
	{
		return boost::apply_visitor([](auto const& v) {return v.can_static_call();}, v);
	}


	namespace syntax_trait
	{
		struct expression :generic_data_type<expression,
			tuple<generic_tag, column_t, line_t>,//Literal
			tuple<std::string, column_t, line_t>,//Value
			tuple<std::vector<std::reference_wrapper<script_command<generic_tag>>>, 
				std::vector<expression>, column_t, line_t>> {};//Func
		const auto Literal = expression::instance_function<0>();
		const auto Value = expression::instance_function<1>();
		const auto Func = expression::instance_function<2>();
		namespace detail
		{
			struct eval_literal_t
			{
				template<class Recur, class Generic>Generic operator()(
					Recur recur,
					type_tag<Generic>,
					Generic const& v,
					column_t column,
					line_t line,
					std::string const& filename,
					utility::dictionary<std::string, Generic>const& global,
					utility::dictionary<std::string, Generic>const& local)const
				{
					return v;
				}
			};
			struct eval_value_t
			{
				template<class Recur, class Generic>Generic operator()(
					Recur recur,
					type_tag<Generic>,
					std::string const& name,
					column_t column,
					line_t line,
					std::string const& filename,
					utility::dictionary<std::string, Generic>const& global,
					utility::dictionary<std::string, Generic>const& local)const
				{
					if (auto v = utility::dictionary_search(local, name))
						return *v;
					if (auto v = utility::dictionary_search(global, name))
						return *v;
					return 0;
				}
			};
			struct eval_function_t
			{
				template<class Recur, class Generic>Generic operator()(
					Recur recur,
					type_tag<Generic>,
					std::vector<std::reference_wrapper<script_command<Generic>>>const& funcs,
					std::vector<expression> const& exprs, 
					column_t column,
					line_t line,
					std::string const& filename,
					utility::dictionary<std::string, Generic>const& global,
					utility::dictionary<std::string, Generic>const& local)const
				{
					for (auto const& func : boost::adaptors::reverse(funcs))
					{
						std::vector<Generic> val{};
						for (auto const& expr : exprs)
						{
							val.push_back(recur(expr, filename, global, local));
						}
						for (auto const& func : funcs)
						{
							auto& f = *func;
							if (boost::apply_visitor([&](auto& u) {return u.type_check(val);}, f))
								return boost::apply_visitor([&](auto& u) {return u.call(std::move(val), filename, line, column);});
						}
						throw utility::make_exception<std::domain_error>(R"(no match function error)", filename, line, column);
					}
				}
			};
		}
		const auto value_eval = pattern_match::generic_recursion<generic_tag,
			expression, std::string const&,
			utility::dictionary<std::string, generic_tag>const&,
			utility::dictionary<std::string, generic_tag>const&>()
			| Literal(0_, 1_, 2_) <= detail::eval_literal_t()
			| Value(0_, 1_, 2_) <= detail::eval_value_t()
			| Func(0_, 1_, 2_, 3_) <= detail::eval_function_t();
		template<class Generic>using expr_type = std::shared_ptr<expression::value_type<Generic>>;

		struct syntax : generic_data_type<syntax,
			tuple<std::string, std::array<expr_type<generic_tag>, 3>, std::vector<syntax>, line_t>,//For
			tuple<std::vector<std::pair<expr_type<generic_tag>, std::vector<syntax>>>, std::vector<syntax>, line_t>,//If
			tuple<expr_type<generic_tag>, std::vector<syntax>, line_t>,//While
			tuple<std::string, expr_type<generic_tag>>,//Global
			tuple<std::string, expr_type<generic_tag>>,//Let
			tuple<expr_type<generic_tag>>>//Expr
		{};
		const auto For = syntax::instance_function<0>();
		const auto If = syntax::instance_function<1>();
		const auto While = syntax::instance_function<2>();
		const auto Global = syntax::instance_function<3>();
		const auto Let = syntax::instance_function<4>();
		const auto Expr = syntax::instance_function<5>();
		template<class Generic>using syntax_type = std::shared_ptr<syntax::value_type<Generic>>;

		template<class Generic>std::pair<std::vector<syntax_type<Generic>>, boost::optional<std::reference_wrapper<script_command<Generic>>>> parsing_short_script(
			utility::dictionary<std::string, system_command<Generic>>const& system_command,
			runner_t<Generic>& runner,
			std::string entry,
			vector_iterator<token_traits::token_tree> base,
			vector_iterator<token_traits::token_tree> first,
			vector_iterator<token_traits::token_tree> last);
		
	}
	template<class... Ts>boost::variant<Ts...>run(
		short_script_runner<Ts...>& runner,
		vector_iterator<syntax_trait::syntax_type<boost::variant<Ts...>>> first,
		vector_iterator<syntax_trait::syntax_type<boost::variant<Ts...>>> last,
		utility::dictionary<std::string, boost::variant<Ts...>>& local);
	template<class... Ts>struct script_runner
	{
		typedef boost::variant<Ts...> value_type;
		std::vector<std::pair<std::string, script_command<value_type>>> commands;
		std::vector<syntax_trait::syntax_type<value_type>> code;
		utility::dictionary<std::string, value_type> global;
		std::string filename;
		boost::optional<std::reference_wrapper<script_command<value_type>>> entry_point;
		template<class Stream>script_runner(
			Stream&& stream,
			std::string filen,
			std::string entry,
			utility::dictionary<std::string, system_command<value_type>>const& system_commands, 
			std::vector<std::pair<std::string,script_command<value_type>>&&default_commands):
			commands(std::move(default_commands)),
			code{},
			global{},
			filename{std::move(filen)},
			entry_point(boost::none)
		{
			std::vector<token_traits::token_tree> token = token_traits::make_token_tree(stream, filename);
			auto first = token.begin();
			auto last = token.end();
			auto&& c = syntax_trait::parsing_short_script(system_commands, *this, first, first, last);
			entry_point = c.second;
			code = std::move(c.first);
		}
		value_type run(std::vector<value_type>&& val)
		{
			return call(entry_point, std::move(val), filename, 0, 0, *this);
		}
	};
	namespace syntax_trait
	{
		template<class Generic>std::pair<std::vector<syntax_type<Generic>>, boost::optional<std::reference_wrapper<script_command<Generic>>>> parsing_short_script(
			utility::dictionary<std::string, system_command<Generic>>const& system_command,
			runner_t<Generic>& runner,
			std::string entry,
			vector_iterator<token_traits::token_tree> base,
			vector_iterator<token_traits::token_tree> first,
			vector_iterator<token_traits::token_tree> last)
		{
			std::vector<syntax_type<Generic>> code;
			boost::optional<std::reference_wrapper<script_command<Generic>>> entry_point;
			for (;first != last;++first)
			{
				
			}
		}
	}
}