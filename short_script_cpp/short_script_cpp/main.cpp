

#include<iostream>
#include<fstream>

#define TEST_V2
#ifndef TEST_V2
#include"short_script_core.hpp"
using namespace short_script_cpp;

int main(int argc,char const* argv[])
{
	std::fstream ss{ argv[1] };
	if (!ss)
	{
		std::cerr << argv[1] << " was not found." << std::endl;
	}
	else
	try 
	{
		auto coms = get_default_command();
		dictionary_add(coms, std::string("test"),make_command([](dictionary<std::string, value_type>& local, std::size_t)
		{
			auto v = *dictionary_search(local, std::string("hage"));
			std::cerr << v.type().name() << std::endl;
			return 0;
		}, "hage"));
		script_runner runner{ std::move(ss),std::move(coms) };
		runner.main("main");
	}
	catch (std::exception& exp)
	{
		std::cout << exp.what() << std::endl;
	}
}
#else
#include"short_script_v2.hpp"
#include<sstream>
int main()
{
	std::string str;
	std::getline(std::cin, str);
	std::stringstream ss{ str };
	try
	{
		auto t = short_script_v2::token_traits::make_token_tree(std::move(ss), "test.txt");
		short_script_v2::token_traits::print_tree(t[0]);
	}
	catch (std::exception& exp)
	{
		std::cerr << exp.what() << std::endl;
	}
}
#endif