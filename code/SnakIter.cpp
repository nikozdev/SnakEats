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
//typedef
typedef enum eGridCell
{
	eGridCell_None,
	eGridCell_Head,
	eGridCell_Body,
	eGridCell_Food,
	eGridCell_Wall,
	eGridCell_Last,
} eGridCell;
typedef std::vector<eGridCell> tCellGrid;
typedef struct tBodyCell
{
	SDL_Point vPos;
	SDL_Point vDir;
} tBodyCell;
typedef std::vector<tBodyCell> tCellBody;
typedef struct tCore
{
	SDL_Point vGridSize;
	tCellGrid vCellGrid;
	struct
	{
		tCellBody vBody;
	} vPlayer;
	struct
	{
		SDL_Window *vHand;
		SDL_Point		vSize;
	} vWindow;
	struct
	{
		SDL_Renderer *vHand;
		SDL_Point			vSize;
		SDL_Color			vTint;
	} vDrawer;
	struct
	{
		unsigned vSecMul;
		unsigned vSecDiv;
	} vTicker;
	bool vInitFlag;
	bool vWorkFlag;
} tCore;
//actions
//-//system
auto fMake(const tConf &vConf) -> tCore
{
	return tCore{
    .vGridSize = vConf.vGridSize,
    .vCellGrid = tCellGrid((vConf.vGridSize.y * vConf.vGridSize.x), eGridCell_None),
    .vPlayer = {
      .vBody = { tBodyCell{ .vPos = { 0, 0 }, .vDir = { 0, 1 } } },
    },
    .vWindow = {
      .vHand = 0,
      .vSize = vConf.vWindow.vSize,
    },
    .vDrawer = {
      .vHand = 0,
      .vSize = vConf.vWindow.vSize,
      .vTint = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xff },
    },
    .vTicker = {
      .vSecMul = vConf.vTicker.vSecMul,
      .vSecDiv = vConf.vTicker.vSecDiv,
    },
		.vInitFlag = 0,
		.vWorkFlag = 0,
	};
}
bool fInit(tCore &vCore)
{
	if(vCore.vInitFlag)
	{
		return 0;
	}
	else
	{
		//framework
		vCore.vInitFlag = SDL_Init(SDL_INIT_EVERYTHING) == EXIT_SUCCESS;
		if(vCore.vInitFlag == 0)
		{
			fmt::println(stderr, "Error initializing SDL: {}", SDL_GetError());
			return 0;
		}
		//window
		vCore.vWindow.vHand = SDL_CreateWindow(
			dSnakIter_ProjName,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			vCore.vWindow.vSize.x,
			vCore.vWindow.vSize.y,
			0
		);
		if(vCore.vWindow.vHand == 0)
		{
			fmt::println(stderr, "Error creating SDL window: {}", SDL_GetError());
			return fQuit(vCore);
		}
		//drawer
		vCore.vDrawer.vHand = SDL_CreateRenderer(vCore.vWindow.vHand, -1, 0);
		if(vCore.vDrawer.vHand == 0)
		{
			fmt::println(stderr, "Error creating SDL renderer: {}", SDL_GetError());
			return fQuit(vCore);
		}
		return 1;
	}
}//fInit
bool fQuit(tCore &vCore)
{
	if(vCore.vInitFlag)
	{
		if(vCore.vDrawer.vHand != 0)
		{
			SDL_DestroyRenderer(vCore.vDrawer.vHand);
			vCore.vDrawer.vHand = 0;
		}
		if(vCore.vWindow.vHand != 0)
		{
			SDL_DestroyWindow(vCore.vWindow.vHand);
			vCore.vWindow.vHand = 0;
		}
		SDL_Quit();
		vCore.vInitFlag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}//fQuit
inline static bool fHitK(tCore &vCore, int vKey, int vMod, bool vState)
{
	switch(vKey)
	{
	case 'w':
	{
		vCore.vPlayer.vBody[0].vDir.x = +0;
		vCore.vPlayer.vBody[0].vDir.y = +1;
	}
	break;
	case 'a':
	{
		vCore.vPlayer.vBody[0].vDir.x = -1;
		vCore.vPlayer.vBody[0].vDir.y = +0;
	}
	break;
	case 's':
	{
		vCore.vPlayer.vBody[0].vDir.x = +0;
		vCore.vPlayer.vBody[0].vDir.y = -1;
	}
	break;
	case 'd':
	{
		vCore.vPlayer.vBody[0].vDir.x = +1;
		vCore.vPlayer.vBody[0].vDir.y = +0;
	}
	break;
	}
	return 0;
}//fHitK
inline static bool fProc(tCore &vCore)
{
	SDL_GetWindowSize(
		vCore.vWindow.vHand, &vCore.vWindow.vSize.x, &vCore.vWindow.vSize.y
	);
	auto vEvent = SDL_Event{};
	while(SDL_PollEvent(&vEvent))
	{
		switch(vEvent.type)
		{
		case SDL_KEYUP:
			fHitK(vCore, vEvent.key.keysym.sym, vEvent.key.keysym.mod, 1);
			break;
		case SDL_KEYDOWN:
			fHitK(vCore, vEvent.key.keysym.sym, vEvent.key.keysym.mod, 0);
			break;
		case SDL_WINDOWEVENT: break;
		case SDL_QUIT: fStop(vCore); break;
		default: break;
		}
	}
	return 1;
}//fProc
inline static bool fDraw(tCore &vCore)
{
	SDL_GetRendererOutputSize(
		vCore.vDrawer.vHand, &vCore.vDrawer.vSize.x, &vCore.vDrawer.vSize.y
	);
	SDL_SetRenderDrawColor(
		vCore.vDrawer.vHand,
		vCore.vDrawer.vTint.r,
		vCore.vDrawer.vTint.g,
		vCore.vDrawer.vTint.b,
		vCore.vDrawer.vTint.a
	);
	SDL_RenderClear(vCore.vDrawer.vHand);
	SDL_Rect vRectList[1] = {
		{.x = 100, .y = 100, .w = 200, .h = 200},
	};
	SDL_SetRenderDrawColor(vCore.vDrawer.vHand, 0, 0, 0, 255);
	SDL_RenderDrawRects(vCore.vDrawer.vHand, vRectList, 1);
	SDL_RenderPresent(vCore.vDrawer.vHand);
	//final
	return 1;
}//fDraw
inline static bool fStep(tCore &vCore)
{
	fProc(vCore);
	fDraw(vCore);
	return 1;
}//fStep
bool fWork(tCore &vCore)
{
	vCore.vWorkFlag = 1;
	while(vCore.vWorkFlag)
	{
		fStep(vCore);
	}
	return 1;
}//fWork
bool fStop(tCore &vCore)
{
	if(vCore.vWorkFlag)
	{
		vCore.vWorkFlag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}//fStop
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
		auto vCore = nSnakIter::fMake({
			.vWindow = {.vSize = {.x = 0x200, .y = 0x200}},
			.vTicker = {.vSecMul = 1, .vSecDiv = 1},
		});
		nSnakIter::fInit(vCore);
		nSnakIter::fWork(vCore);
		nSnakIter::fQuit(vCore);
	}
	return EXIT_SUCCESS;
}//main
#endif//dSnakIter_Cpp
