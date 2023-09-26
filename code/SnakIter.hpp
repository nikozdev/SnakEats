#ifndef dSnakIter_Hpp
#define dSnakIter_Hpp
//headers
#include <cstdlib>
#include <functional>
#include <algorithm>
//-//datetime
#include <chrono>
//-//template library
#include <initializer_list>
#include <array>
#include <vector>
#include <unordered_map>
//-//strings
#include <string_view>
#include <sstream>
#include <cctype>
//-//input output
#include <iostream>
#include <fstream>
//-//system
#include <SDL2/SDL.h>
//content
namespace nSnakIter
{
//typedef
typedef struct tCore tCore;
typedef struct tConf
{
  SDL_Point vGridSize = { 0x20, 0x20 };
	struct
	{
		SDL_Point vSize = {.x = 0x200, .y = 0x200};
	} vWindow;
	struct
	{
		unsigned vSecMul = 1;
		unsigned vSecDiv = 1;
	} vTicker;
} tConf;
//actions
auto fMake(const tConf &) -> tCore;
bool fInit(tCore &);
bool fQuit(tCore &);
bool fWork(tCore &);
bool fStop(tCore &);
}//namespace nSnakIter
#endif//dSnakIter_Hpp
