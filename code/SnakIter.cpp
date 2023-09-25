#ifndef dSnakIter_Cpp
#define dSnakIter_Cpp
//headers
#include "SnakIter.hpp"
#include "fmt/core.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "fmt/chrono.h"
//content
namespace nSnakIter
{
//actions
//testing
#if defined(dSnakIter_MakeTest)
//-//typedef
using tTestKey = std::string_view;
using tTestOut = int;
using tTestFun = std::function<tTestOut(void)>;
using tTestTab = std::unordered_map<tTestKey, tTestFun>;
using tTestRef = tTestTab::iterator;
//-//consdef
static const tTestTab vTestTab = {
	{"Hello",
	 []()
	 {
		 fmt::println(stdout, "HelloWorld");
		 return EXIT_SUCCESS;
	 }},
};
#endif//ifd(dSnakIter_MakeTest)
}//namespace nSnakIter
//actions
int main(int vArgC, char **vArgV, char **vEnvi)
{
	using namespace nSnakIter;
#ifdef dSnakIter_MakeTest
	if(vArgC == 3 && std::string_view(vArgV[1]) == "test")
	{
		auto vTestKey = std::string_view(vArgV[2]);
		auto vTestRef = vTestTab.find(vTestKey);
		if(vTestRef == vTestTab.end())
		{
			fmt::println(stderr, "invalid test key: {}", vTestKey);
			return 0;
		}
		else
		{
			fmt::println(stdout, "${}?", vTestKey);
			auto vTestOut = vTestRef->second();
			fmt::println(stdout, "${}!", vTestKey);
			fmt::println(
				stderr,
				"TestCode is {}",
				(vTestOut == EXIT_SUCCESS) ? "EXIT_SUCCESS"
				: (vTestOut == EXIT_FAILURE)
					? "EXIT_FAILURE"
					: "UNDEFINED"
			);
			return vTestOut == EXIT_SUCCESS;
		}
		return 1;
	}
#endif//ifd(dSnakIter_MakeTest)
	{
		std::copy(
			vArgV, vArgV + vArgC, std::ostream_iterator<const char *>(std::cout, "\n")
		);
	}
}
#endif//dSnakIter_Cpp
