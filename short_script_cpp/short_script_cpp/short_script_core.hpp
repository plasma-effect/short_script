#pragma once

//Copyright(c) 2015-2016, plasma-effect
//All rights reserved.
//
// Distributed under The BSD 2-Clause License.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met :
//
//1. Redistributions of source code must retain the above copyright notice,
//this list of conditions and the following disclaimer.
//2. Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and / or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
//ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//The views and conclusions contained in the software and documentation are those
//of the authors and should not be interpreted as representing official policies,
//either expressed or implied, of the FreeBSD Project.

#include<plasma_adt/algebraic_data_type.hpp>
#include<string>
#include<iostream>
#include<regex>
#include<boost/lexical_cast.hpp>
#include<boost/format.hpp>
namespace short_script_cpp
{
	namespace detail
	{
		struct scope_exit_t
		{
			std::function<void(void)> func;
			scope_exit_t(std::function<void(void)>&& f) :func(std::move(f)){}
			scope_exit_t(std::function<void(void)>const& f) :func(f) {}
			scope_exit_t(scope_exit_t const&) = delete;
			scope_exit_t(scope_exit_t&&) = default;
			scope_exit_t& operator=(scope_exit_t const&) = delete;
			scope_exit_t& operator=(scope_exit_t&&) = default;
			~scope_exit_t()
			{
				func();
			}
		};
		template<class Func>scope_exit_t scope_exit(Func&& func)
		{
			return scope_exit_t(std::forward<Func>(func));
		}
	}
	using namespace plasma_adt;
	using namespace plasma_adt::pattern_match;
	using namespace plasma_adt::place_holder;
	struct string_tree :data_type_base<string_tree, std::string, std::vector<string_tree>>
	{
		string_tree(data_type_base const&v) :data_type_base(v) {}
	};
	const auto atomic = string_tree::instance_function<0>();
	const auto tree = string_tree::instance_function<1>();
	const auto get_atomic = pattern_match::pattern_match<boost::optional<std::string>,string_tree>()
		| atomic(0_) <= [](std::string const& v) {return boost::make_optional(v);}
		| tree <= [] {return static_cast<boost::optional<std::string>>(boost::none);};

	const auto get_tree=pattern_match::pattern_match<boost::optional<std::vector<string_tree>>,string_tree>()
		| tree(0_) <= [](std::vector<string_tree>const& vec) {return boost::make_optional(vec);}
		| atomic <= [] {return static_cast<boost::optional<std::vector<string_tree>>>(boost::none);};

	const auto stree_size = pattern_match::pattern_match<std::size_t,string_tree>()
		| tree(0_) <= [](std::vector<string_tree>const& vec) {return vec.size();}
		| atomic <= [] {return 1u;};

	typedef boost::variant<int, double, std::string, bool> value_type;
	template<class Key, class Value>using dictionary = std::vector<std::pair<Key, Value>>;

	typedef std::string::const_iterator string_iterator;
	std::pair<string_iterator, string_tree> make_tree(string_iterator first,string_iterator last)
	{
		std::vector<string_tree> ret;
		bool f = false;
		string_iterator s = first;
		string_iterator e = first;
		for (;first != last;++first)
		{
			auto c = *first;
			if (c == '(')
			{
				if (std::distance(s, e) != 0)ret.push_back(atomic(std::string(s, e)));
				auto r = make_tree(std::next(first), last);
				ret.push_back(r.second);
				f = false;
				first = r.first;
				s = std::next(first);
				e = std::next(first);
			}
			else if (c == ')')
			{
				if (std::distance(s, e) != 0)ret.push_back(atomic(std::string(s, e)));
				return std::make_pair(first, tree(ret));
			}
			else if (c == ' ')
			{
				f = false;
			}
			else if (c == '\t')
			{
				f = false;
			}
			else if(c=='\"')
			{
				if (std::distance(s, e) != 0)ret.push_back(atomic(std::string(s, e)));
				s = first;
				++first;
				for (;first != last;++first)
				{
					if (*first == '\"')
					{
						break;
					}
				}
				ret.push_back(atomic(std::string(s, first == last ? first : first + 1)));
				s = first;
				e = first;
				f = false;
			}
			else if (c == '#')
			{
				break;
			}
			else if(f)
			{
				++e;
			}
			else
			{
				if (std::distance(s, e) != 0)ret.push_back(atomic(std::string(s, e)));
				s = first;
				e = std::next(first);
				f = true;
			}
		}
		if (std::distance(s, e) != 0)ret.push_back(atomic(std::string(s, e)));
		return std::make_pair(last, tree(ret));
	}

	template<class Key, class Value>boost::optional<Value> dictionary_search(dictionary<Key,Value> const& dic, Key const& key)
	{
		for (auto const& v : dic)
		{
			if (v.first == key)
				return v.second;
		}
		return boost::none;
	}
	template<class Key, class Value>bool dictionary_add(dictionary<Key,Value>& dic, Key const& key, Value const& value)
	{
		for (auto& v : dic)
		{
			if (v.first == key)
			{
				v.second = value;
				return true;
			}
		}
		dic.push_back(std::make_pair(key, value));
		return false;
	}
	template<class Key, class Value>boost::optional<Value> dictionary_remove(dictionary<Key, Value>& dic, Key const& key)
	{
		boost::optional<Value> ret{};
		std::remove_if(std::begin(dic),std::end(dic),[&](std::pair<Key,Value>const& v)
		{
			if (v.first == key)
			{
				ret = v.second;
				return true;
			}
			return false;
		});
		return ret;
	}

	namespace detail
	{
		template<class T>boost::format make_format(boost::format format, T const& v)
		{
			return format%v;
		}

		template<class T, class... Ts>boost::format make_format(boost::format format, T const& arg, Ts const&... args)
		{
			return make_format(format%arg, args...);
		}

		template<class Exception, class... Ts>Exception make_exception(std::string message, std::size_t line, Ts const&... args)
		{
			return Exception(make_format(boost::format(R"(short script error: line %1% )" + message), line + 1, args...).str());
		}

		template<class T, class Exception>T optional_value(boost::optional<T>const& v, Exception exp)
		{
			return v ? *v : throw exp;
		}

		template<class T>T& at(std::vector<T>& vec,std::size_t i)
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

	}

	const auto short_print = recursion_match<int, string_tree, int>()
		| atomic(0_) <= [](auto, std::string const& str, int x)
	{
		for (int i = 1;i < x;++i)
		{
			std::cout << "  ";
		}
		auto v = literal_check(str);
		if (!v)
		{
			std::cout << "other:" << str << std::endl;
		}
		else
		{
			auto const& u = *v;
			if (u.which() == 0)
			{
				std::cout << "int:";
			}
			else if (u.which() == 1)
			{
				std::cout << "double:";
			}
			else
			{
				std::cout << "string:";
			}
			boost::apply_visitor([](auto const& x) {std::cout << x << std::endl;}, u);
		}
		return 0;
	}
		| tree(0_) <= [](auto recur, std::vector<string_tree> const& vec, int x)
	{
		for (auto const& v : vec)
		{
			recur(v, x + 1);
		}
		return 0;
	};

	boost::optional<value_type> literal_check(std::string const& str)
	{
		if (std::regex_match(str, std::basic_regex<char>(R"(-?\d+)")))
			return boost::lexical_cast<int>(str);
		if (std::regex_match(str, std::basic_regex<char>(R"([+-]?((\d*.\d+)|(\d+.\d*))|(\d+[eE][-+]\d+))")))
			return boost::lexical_cast<double>(str);
		if (std::regex_match(str, std::basic_regex<char>(R"(\".+\")")))
			return str.substr(1, str.size() - 2);
		if (str == "true")
			return true;
		if (str == "false")
			return false;
		return boost::none;
	}

	struct script_command
	{
		std::vector<std::string> arguments;
		std::function<value_type(dictionary<std::string, value_type>&,std::size_t)> func;
		value_type operator()(
			std::vector<value_type> params,
			std::size_t line)
		{
			dictionary<std::string, value_type> val{};
			for (std::size_t i = 0;i < arguments.size();++i)
			{
				dictionary_add(val, detail::at(arguments, i), detail::at(params, i));
			}
			return func(val, line);
		}

		std::size_t size()const
		{
			return arguments.size();
		}
	};

	namespace value_trait
	{
#define SHORT_SCRIPT_MAKE_VALUE_TRAIT(name, lhs, rhs, line)\
	boost::apply_visitor(\
	[&lhs=lhs,line=line](const auto& rhs)\
	{\
		return boost::apply_visitor(\
			[&rhs=rhs,line=line](const auto& lhs)\
			{\
				return name(lhs, rhs, line);\
			},lhs);\
	},rhs);

		namespace detail
		{
			inline value_type addi(int lhs, int rhs, std::size_t)
			{
				return lhs + rhs;
			}
			inline value_type addi(int lhs, double rhs, std::size_t)
			{
				return lhs + rhs;
			}
			inline value_type addi(double lhs, int rhs, std::size_t)
			{
				return lhs + rhs;
			}
			inline value_type addi(double lhs, double rhs, std::size_t)
			{
				return lhs + rhs;
			}

			inline value_type addi(std::string const& lhs, std::string const& rhs, std::size_t)
			{
				return lhs + rhs;
			}
			template<class T>inline value_type addi(T const& lhs, std::string const& rhs, std::size_t)
			{
				return boost::lexical_cast<std::string>(lhs) + rhs;
			}
			template<class T>inline value_type addi(std::string const& lhs, T const& rhs, std::size_t)
			{
				return lhs + boost::lexical_cast<std::string>(rhs);
			}

			inline value_type addi(bool lhs, int rhs, std::size_t line)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator+ type exception (boolean cant use in operator+))", line);
			}
			inline value_type addi(bool lhs, double rhs, std::size_t line)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator+ type exception (boolean cant use in operator+))", line);
			}
			inline value_type addi(bool lhs, bool rhs, std::size_t line)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator+ type exception (boolean cant use in operator+))", line);
			}
			inline value_type addi(int lhs, bool rhs, std::size_t line)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator+ type exception (boolean cant use in operator+))", line);
			}
			inline value_type addi(double lhs, bool rhs, std::size_t line)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator+ type exception (boolean cant use in operator+))", line);
			}
		}
		inline value_type add(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return boost::apply_visitor([&rhs = rhs, line = line](const auto& lhs)
			{
				return boost::apply_visitor([&lhs = lhs, line = line](const auto& rhs) {return detail::addi(lhs, rhs, line);},rhs);
			}, lhs);
		}

		namespace detail
		{
			template<class Lhs, class Rhs>value_type equal(Lhs const& lhs, Rhs const& rhs, std::size_t, std::enable_if_t<std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				return lhs == rhs;
			}
			template<class Lhs, class Rhs>value_type equal(Lhs const& lhs, Rhs const& rhs, std::size_t line, std::enable_if_t<!std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator= type exception (another types' equality cant be checked))", line);
			}
		}
		inline value_type equal(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return SHORT_SCRIPT_MAKE_VALUE_TRAIT(detail::equal, lhs, rhs, line);
		}

		namespace detail
		{
			template<class Lhs, class Rhs>value_type notequal(Lhs const& lhs, Rhs const& rhs, std::size_t, std::enable_if_t<std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				return lhs != rhs;
			}
			template<class Lhs, class Rhs>value_type notequal(Lhs const& lhs, Rhs const& rhs, std::size_t line, std::enable_if_t<!std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator<> type exception (another types' equality cant be checked))", line);
			}
		}
		inline value_type notequal(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return SHORT_SCRIPT_MAKE_VALUE_TRAIT(detail::notequal, lhs, rhs, line);
		}

		namespace detail
		{
			inline value_type sub(int lhs, int rhs, std::size_t)
			{
				return lhs - rhs;
			}
			inline value_type sub(double lhs, int rhs, std::size_t)
			{
				return lhs - rhs;
			}
			inline value_type sub(int lhs, double rhs, std::size_t)
			{
				return lhs - rhs;
			}
			inline value_type sub(double lhs,double rhs, std::size_t)
			{
				return lhs - rhs;
			}
			template<class Lhs, class Rhs>inline value_type sub(Lhs const&, Rhs const&,std::size_t line)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator- type exception (provided type cant use in operator-))", line);
			}
		}
		inline value_type sub(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return SHORT_SCRIPT_MAKE_VALUE_TRAIT(detail::sub, lhs, rhs, line);
		}

		namespace detail
		{
			inline value_type mul(int lhs, int rhs, std::size_t)
			{
				return lhs * rhs;
			}
			inline value_type mul(double lhs, int rhs, std::size_t)
			{
				return lhs * rhs;
			}
			inline value_type mul(int lhs, double rhs, std::size_t)
			{
				return lhs * rhs;
			}
			inline value_type mul(double lhs, double rhs, std::size_t)
			{
				return lhs * rhs;
			}
			template<class Lhs, class Rhs>inline value_type mul(Lhs const&, Rhs const&, std::size_t line)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator* type exception (provided type cant use in operator*))", line);
			}
		}
		inline value_type mul(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return SHORT_SCRIPT_MAKE_VALUE_TRAIT(detail::mul, lhs, rhs, line);
		}

		namespace detail
		{
			inline value_type div(int lhs, int rhs, std::size_t)
			{
				return lhs / rhs;
			}
			inline value_type div(double lhs, int rhs, std::size_t)
			{
				return lhs / rhs;
			}
			inline value_type div(int lhs, double rhs, std::size_t)
			{
				return lhs / rhs;
			}
			inline value_type div(double lhs, double rhs, std::size_t)
			{
				return lhs / rhs;
			}
			template<class Lhs, class Rhs>inline value_type div(Lhs const&, Rhs const&, std::size_t line)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator/ type exception (provided type cant use in operator/))", line);
			}
		}
		inline value_type div(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return SHORT_SCRIPT_MAKE_VALUE_TRAIT(detail::div, lhs, rhs, line);
		}

		namespace detail
		{
			template<class Lhs, class Rhs>value_type less_than(Lhs const& lhs, Rhs const& rhs, std::size_t, std::enable_if_t<std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				return lhs < rhs;
			}
			template<class Lhs, class Rhs>value_type less_than(Lhs const& lhs, Rhs const& rhs, std::size_t line, std::enable_if_t<!std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator< type exception (another types' size cant be compared))", line);
			}
		}
		inline value_type less_than(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return SHORT_SCRIPT_MAKE_VALUE_TRAIT(detail::less_than, lhs, rhs, line);
		}

		namespace detail
		{
			template<class Lhs, class Rhs>value_type less_equal(Lhs const& lhs, Rhs const& rhs, std::size_t, std::enable_if_t<std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				return lhs <= rhs;
			}
			template<class Lhs, class Rhs>value_type less_equal(Lhs const& lhs, Rhs const& rhs, std::size_t line, std::enable_if_t<!std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator<= type exception (another types' size cant be compared))", line);
			}
		}
		inline value_type less_equal(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return SHORT_SCRIPT_MAKE_VALUE_TRAIT(detail::less_equal, lhs, rhs, line);
		}

		namespace detail
		{
			template<class Lhs, class Rhs>value_type larger_than(Lhs const& lhs, Rhs const& rhs, std::size_t, std::enable_if_t<std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				return lhs > rhs;
			}
			template<class Lhs, class Rhs>value_type larger_than(Lhs const& lhs, Rhs const& rhs, std::size_t line, std::enable_if_t<!std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator> type exception (another types' size cant be compared))", line);
			}
		}
		inline value_type larger_than(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return SHORT_SCRIPT_MAKE_VALUE_TRAIT(detail::larger_than, lhs, rhs, line);
		}

		namespace detail
		{
			template<class Lhs, class Rhs>value_type larger_equal(Lhs const& lhs, Rhs const& rhs, std::size_t, std::enable_if_t<std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				return lhs >= rhs;
			}
			template<class Lhs, class Rhs>value_type larger_equal(Lhs const& lhs, Rhs const& rhs, std::size_t line, std::enable_if_t<!std::is_same<Lhs, Rhs>::value>* = nullptr)
			{
				throw short_script_cpp::detail::make_exception<std::domain_error>(R"(operator>= type exception (another types' size cant be compared))", line);
			}
		}
		inline value_type larger_equal(value_type const& lhs, value_type const& rhs, std::size_t line)
		{
			return SHORT_SCRIPT_MAKE_VALUE_TRAIT(detail::larger_equal, lhs, rhs, line);
		}

	}

	struct script_runner
	{
		std::vector<string_tree> code;
		dictionary<std::string, value_type> global;
		dictionary<std::string, script_command> commands;
		
		template<class Stream>
		script_runner(Stream&& ss, dictionary<std::string, script_command>&& default_command) :
			code{}, global{}, commands(std::move(default_command))
		{
			std::string str;
			while (std::getline(ss, str))
			{
				auto stree = make_tree(std::begin(str), std::end(str)).second;
				code.push_back(stree);
				auto vec = *get_tree(stree);
				if (vec.size() == 0)
					continue;
				auto com = get_atomic(detail::at(vec,0));
				if (!com || *com != "def")
					continue;
				auto name = detail::optional_value(
					get_atomic(detail::at(vec,1)),
					detail::make_exception<std::domain_error>("invalid function name", code.size() - 1));
				std::vector<std::string> args;
				for (std::size_t i = 2;i < vec.size();++i)
				{
					args.push_back(
						detail::optional_value(
							get_atomic(detail::at(vec, i)),
							detail::make_exception<std::domain_error>("invalid function argument", code.size() - 1)));
				}
				auto r = dictionary_add(commands,name, script_command{ std::move(args),
					[&runner = *this, line = code.size()](dictionary<std::string,value_type>& local,std::size_t)
				{
					return runner.run(local,line);
				} });
				if(r)
					throw detail::make_exception<std::domain_error>(R"("%2%" is defined)", code.size() - 1, name);
			}
		}

		value_type value_eval(string_tree const& stree, dictionary<std::string, value_type>const& local, std::size_t line)
		{
			auto veci = get_tree(stree);
			if (veci && (*veci).size() == 1)
				return value_eval(detail::at(*veci, 0), local, line);
			if (auto name = get_atomic(stree))
			{
				auto lit = literal_check(*name);
				auto loc = dictionary_search(local, *name);
				auto gro = dictionary_search(global, *name);
				auto fun = dictionary_search(commands, *name);
				return lit ? *lit : loc ? *loc : gro ? *gro : (fun && (*fun).size() == 0) ? fun->operator()(std::vector<value_type>{}, line) :
					throw detail::make_exception<std::domain_error>(R"(value name "%2%" was not found)", line, *name);
			}
			auto vec = *veci;
			auto comname = detail::optional_value(get_atomic(detail::at(vec,0)), detail::make_exception<std::domain_error>(R"(invalid command)", line));
			auto com = detail::optional_value(
				dictionary_search(commands, comname), 
				detail::make_exception<std::domain_error>(R"(function name "%2%" was not found)", line, comname));
			std::vector<value_type> values{};
			if (vec.size() <= com.size())
				throw detail::make_exception<std::invalid_argument>(
					R"(few parameters by "%2%" (requires %3% argument, but %4% were provided))",
					line, comname, com.size(), vec.size() - 1);
			if (vec.size() == com.size()+1)
			{
				std::for_each(
					std::next(std::begin(vec)), 
					std::end(vec), 
					[&](string_tree const& s) {values.push_back(value_eval(s, local, line));});
			}
			else
			{
				std::for_each(
					std::next(std::begin(vec)),
					std::next(std::begin(vec), com.size()),
					[&](string_tree const& s) {values.push_back(value_eval(s, local, line));});
				values.push_back(value_eval(tree(std::vector<string_tree>(std::next(std::begin(vec), com.size()), std::end(vec))), local, line));
			}
			return com(std::move(values), line);
		}

		value_type run(dictionary<std::string, value_type>& local, std::size_t line)
		{
			for (;line < code.size();++line)
			{
				
				auto const svec = *get_tree(detail::at(code,line));
				if (svec.size() == 0)continue;

				auto com = detail::optional_value(get_atomic(detail::at(svec,0)), detail::make_exception<std::domain_error>(R"(invalid command)", line));
				if (com == "let")
				{
					if (svec.size() < 3)throw detail::make_exception<std::domain_error>(R"(let command error)", line);
					dictionary_add(local,
						detail::optional_value(get_atomic(svec[1]), detail::make_exception<std::invalid_argument>(R"(invalid value name)", line)),
						value_eval(tree(std::vector<string_tree>(std::next(std::begin(svec), 2), std::end(svec))), local, line));
					continue;
				}
				else if (com == "global")
				{
					if (svec.size() < 3)throw detail::make_exception<std::domain_error>(R"(global command error)", line);
					dictionary_add(global,
						detail::optional_value(get_atomic(svec[1]), detail::make_exception<std::invalid_argument>(R"(invalid value name)", line)),
						value_eval(tree(std::vector<string_tree>(std::next(std::begin(svec), 2), std::end(svec))), local, line));
					continue;
				}
				else if (com == "for")
				{
					if (svec.size() < 4)throw detail::make_exception<std::invalid_argument>(
						R"("for" argument error (requires more equal 3 argument, but %2% were provided))", line, svec.size() - 1);
					std::string name = detail::optional_value(get_atomic(svec[1]), detail::make_exception<std::domain_error>(R"(invalid parameter name)", line));
					boost::optional<value_type> val = dictionary_search(local, name);
					value_type end = value_eval(svec[3], local, line);
					value_type step = svec.size() == 4 ? 1 : 
						value_eval(tree(std::vector<string_tree>(std::next(std::begin(svec), 4), std::end(svec))), local, line);
					for (value_type val = value_eval(svec[2], local, line);val != end;val = value_trait::add(val, step, line))
					{
						dictionary_add(local, name, val);
						run(local, line + 1);
					}
					if (val)
						dictionary_add(local, name, *val);
					else
						dictionary_remove(local, name);

					++line;
					for (;line < code.size();++line)
					{
						auto const svec = *get_tree(detail::at(code, line));
						if (svec.size() == 0)continue;

						auto com = detail::optional_value(get_atomic(detail::at(svec, 0)), detail::make_exception<std::domain_error>(R"(invalid command)", line));
						if (com == "next")
							break;
					}
				}
				else if (com == "while")
				{
					if (svec.size() == 1)
						throw detail::make_exception<std::invalid_argument>(
							R"("if" argument error (requires more equal 1 argument, but 0 were provided))", line);
					auto cond = value_eval(tree(std::vector<string_tree>(std::next(std::begin(svec)), std::end(svec))), local, line);
					while (true)
					{
						if (cond.type() != typeid(bool))
							throw detail::make_exception<std::domain_error>(R"("if" condition must be recieved "boolean")", line);
						if (!boost::get<bool>(cond))
							break;
						run(local, line + 1);
						cond = value_eval(tree(std::vector<string_tree>(std::next(std::begin(svec)), std::end(svec))), local, line);
					}
					++line;
					for (;line < code.size();++line)
					{
						auto const svec = *get_tree(detail::at(code, line));
						if (svec.size() == 0)continue;

						auto com = detail::optional_value(get_atomic(detail::at(svec, 0)), detail::make_exception<std::domain_error>(R"(invalid command)", line));
						if (com == "loop")
							break;
					}
				}
				else if (com == "if")
				{
					auto sveci = svec;
					if (sveci.size() == 1)
						throw detail::make_exception<std::invalid_argument>(
							R"("if" argument error (requires more equal 1 argument, but 0 were provided))", line);
					auto cond = value_eval(tree(std::vector<string_tree>(std::next(std::begin(sveci)), std::end(sveci))), local, line);
					if (cond.type() != typeid(bool))
						throw detail::make_exception<std::domain_error>(R"("if" condition must be recieved "boolean")", line);
					while (!boost::get<bool>(cond))
					{
						bool break_flag{};
						while (true)
						{
							++line;
							auto const svec = *get_tree(detail::at(code, line));
							if (sveci.size() == 0)continue;
							
							auto com = detail::optional_value(get_atomic(detail::at(svec,0)), detail::make_exception<std::domain_error>(R"(invalid command)", line));
							if (com == "else")
							{
								break_flag = true;
								break;
							}
							else if (com == "endif")
							{
								break_flag = true;
								break;
							}
							else if (com == "elif")
							{
								break;
							}
						}
						if (break_flag)
							break;

						sveci = *get_tree(detail::at(code, line));
						cond = value_eval(tree(std::vector<string_tree>(std::next(std::begin(sveci)), std::end(sveci))), local, line);
						if (cond.type() != typeid(bool))
							throw detail::make_exception<std::domain_error>(R"("if" condition must be recieved "boolean")", line);
					}
					
				}
				else if (com == "else")
				{
					while (true)
					{
						++line;
						auto const svec = *get_tree(detail::at(code,line));
						if (svec.size() == 0)continue;

						auto com = detail::optional_value(get_atomic(detail::at(svec,0)), detail::make_exception<std::domain_error>(R"(invalid command)", line));
						if (com == "endif")
						{
							break;
						}
					}
				}
				else if (com == "elif")
				{
					while (true)
					{
						++line;
						auto const svec = *get_tree(detail::at(code,line));
						if (svec.size() == 0)continue;

						auto com = detail::optional_value(get_atomic(detail::at(svec,0)), detail::make_exception<std::domain_error>(R"(invalid command)", line));
						if (com == "endif")
						{
							break;
						}
					}
				}
				else if (com == "endif")
				{
					continue;
				}
				else if (com == "loop")
				{
					return 0;
				}
				else if (com == "next")
				{
					return 0;
				}
				else if (com == "def")
				{
					continue;
				}
				else if (com == "end")
				{
					break;
				}
				else if (com == "return")
				{
					return svec.size() == 1 ? 0 : value_eval(tree(std::vector<string_tree>(std::next(std::begin(svec)), std::end(svec))), local, line);
				}
				else
				{
					value_eval(code[line], local, line);
				}
			}
			return 0;
		}

		value_type main(std::string entrypoint, int argc = 0, char const* argv[] = nullptr)
		{
			auto m = dictionary_search(commands, entrypoint);
			if (!m)throw std::domain_error(R"(short script error: entry point was not found)");
			auto& com = *m;
			std::vector<value_type> vec;
			for (int i = 0;i < argc;++i)
			{
				vec.push_back(std::string(argv[i]));
			}
			for (std::size_t i = argc;i < com.size();++i)
			{
				vec.push_back(0);
			}
			return com(std::move(vec), 0);
		}
	};

	inline dictionary<std::string, script_command>get_default_command()
	{
		dictionary<std::string, script_command> ret;
		std::cout << std::boolalpha;
		dictionary_add(ret, std::string("print"),script_command{ std::vector<std::string>{"v"},
			[](dictionary<std::string,value_type>& val,std::size_t)
			{
				return boost::apply_visitor([](auto const& u) {std::cout << u;return value_type(u);},*dictionary_search(val,std::string("v")));
			} 
		});
		dictionary_add(ret, std::string("println"),script_command{ std::vector<std::string>{"v"},
			[](dictionary<std::string,value_type>& val,std::size_t)
		{
			return boost::apply_visitor([](auto const& u) {std::cout << u << std::endl; return value_type(u);},*dictionary_search(val,std::string("v")));
		}
		});
		dictionary_add(ret, std::string("+"),script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
			{
				return value_trait::add(
					*dictionary_search(local,std::string("lhs")),
					*dictionary_search(local,std::string("rhs")),
					line);
			}
		});
		dictionary_add(ret, std::string("="), script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
		{
			return value_trait::equal(
				*dictionary_search(local,std::string("lhs")),
				*dictionary_search(local,std::string("rhs")),
				line);
		}
		});
		dictionary_add(ret, std::string("-"), script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
		{
			return value_trait::sub(
				*dictionary_search(local,std::string("lhs")),
				*dictionary_search(local,std::string("rhs")),
				line);
		}
		});
		dictionary_add(ret, std::string("<>"), script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
		{
			return value_trait::notequal(
				*dictionary_search(local,std::string("lhs")),
				*dictionary_search(local,std::string("rhs")),
				line);
		}
		});
		dictionary_add(ret, std::string("*"), script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
		{
			return value_trait::mul(
				*dictionary_search(local,std::string("lhs")),
				*dictionary_search(local,std::string("rhs")),
				line);
		}
		});
		dictionary_add(ret, std::string("/"), script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
		{
			return value_trait::div(
				*dictionary_search(local,std::string("lhs")),
				*dictionary_search(local,std::string("rhs")),
				line);
		}
		});
		dictionary_add(ret, std::string("<"), script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
		{
			return value_trait::less_than(
				*dictionary_search(local,std::string("lhs")),
				*dictionary_search(local,std::string("rhs")),
				line);
		}
		});
		dictionary_add(ret, std::string("<="), script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
		{
			return value_trait::less_equal(
				*dictionary_search(local,std::string("lhs")),
				*dictionary_search(local,std::string("rhs")),
				line);
		}
		});
		dictionary_add(ret, std::string(">"), script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
		{
			return value_trait::larger_than(
				*dictionary_search(local,std::string("lhs")),
				*dictionary_search(local,std::string("rhs")),
				line);
		}
		});
		dictionary_add(ret, std::string(">="), script_command{
			std::vector<std::string>{"lhs","rhs"},
			[](dictionary<std::string,value_type>& local,std::size_t line)
		{
			return value_trait::larger_equal(
				*dictionary_search(local,std::string("lhs")),
				*dictionary_search(local,std::string("rhs")),
				line);
		}
		});
		
		return ret;
	}

	template<class Func, class... Ts>script_command make_command(Func&& func, Ts const&... argument_names)
	{
		return script_command
		{
			std::vector<std::string>{std::string(argument_names...)},
			std::forward<Func>(func)
		};
	}
}