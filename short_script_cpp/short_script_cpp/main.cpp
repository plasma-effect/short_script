#include"short_script_core.hpp"
#include<iostream>
#include<fstream>

using namespace short_script_cpp;

int main(int argc,char const* argv[])
{
#ifndef _DEBUG
	std::fstream ss{ argv[1] };
	try {
#else
	std::stringstream ss{
		R"(def fact n
if = n 0
 return 1
else
 return * n fact - n 1

def main
println fact 6
return)" };
#endif
	auto coms = get_default_command();
	dictionary_add(coms, std::string("test"),make_command([](dictionary<std::string, value_type>& local, std::size_t)
	{
		auto v = *dictionary_search(local, std::string("hage"));
		std::cout << v.type().name() << std::endl;
		return 0;
	}, "hage"));
	script_runner runner{ std::move(ss),std::move(coms) };
		runner.main("main");
#ifndef _DEBUG
	}
	catch (std::exception& exp)
	{
		std::cout << exp.what() << std::endl;
	}
#endif
}