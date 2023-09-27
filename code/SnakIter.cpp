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
typedef std::default_random_engine				 tRandCore;
typedef std::uniform_int_distribution<int> tRandSint;
typedef enum eGridTile
{
	eGridTile_None,
	eGridTile_Head,
	eGridTile_Body,
	eGridTile_Food,
	eGridTile_Wall,
	eGridTile_Last,
} eGridTile;
typedef std::vector<eGridTile> tTileGrid;
typedef std::vector<SDL_Point> tBodyList;
typedef std::vector<SDL_Rect>	 tRectList;
typedef struct tCore
{
	SDL_Point vGridSize;
	SDL_Point vTileSize;
	tTileGrid vTileGrid;
	SDL_Point vFoodTpos;
	struct
	{
		SDL_Point vMoveCurr;
		SDL_Point vMovePrev;
		tBodyList vBodyCurr;
		tBodyList vBodyPrev;
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
		tRectList			vRectList;
	} vDrawer;
	struct
	{
		unsigned vSecMul;
		unsigned vSecDiv;
		unsigned vMilWas;
		unsigned vMilNow;
	} vTicker;
	struct
	{
		tRandCore vCore;
		tRandSint vSint;
	} vRandom;
	bool vInitFlag;
	bool vWorkFlag;
} tCore;
//actions
//-//grid
auto fGetTile(tCore &vCore, SDL_Point vTpos) -> eGridTile &
{
	vTpos.x %= vCore.vGridSize.x;
	vTpos.y %= vCore.vGridSize.y;
	return vCore.vTileGrid[vTpos.y * vCore.vGridSize.x + vTpos.x];
}//fGetTile
//-//system
auto fMake(const tConf &vConf) -> tCore
{
	return tCore{
    .vGridSize = vConf.vGridSize,
    .vTileSize = {vConf.vWindow.vSize.x / vConf.vGridSize.x, vConf.vWindow.vSize.y / vConf.vGridSize.y },
    .vTileGrid = tTileGrid((vConf.vGridSize.y * vConf.vGridSize.x), eGridTile_None),
    .vFoodTpos = { 0, 0 },
    .vPlayer = {
      .vMoveCurr = { 0, 0 },
      .vMovePrev = { 0, 0 },
      .vBodyCurr = { { vConf.vGridSize.x / 2, vConf.vGridSize.y / 2 } },
      .vBodyPrev = { { vConf.vGridSize.x / 2, vConf.vGridSize.y / 2 } },
    },
    .vWindow = {
      .vHand = 0,
      .vSize = vConf.vWindow.vSize,
    },
    .vDrawer = {
      .vHand = 0,
      .vSize = vConf.vWindow.vSize,
      .vTint = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xff },
      .vRectList = tRectList(vConf.vGridSize.x * vConf.vGridSize.y),
    },
    .vTicker = {
      .vSecMul = vConf.vTicker.vSecMul,
      .vSecDiv = vConf.vTicker.vSecDiv,
      .vMilWas = 0,
      .vMilNow = 0,
    },
    .vRandom = {
      .vCore = tRandCore(),
      .vSint = tRandSint(1, vConf.vGridSize.x * vConf.vGridSize.y),
    },
		.vInitFlag = 0,
		.vWorkFlag = 0,
	};
}//fMake
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
		//player
		auto vHeadTpos = SDL_Point{vCore.vGridSize.x / 2, vCore.vGridSize.y / 2};
		fGetTile(vCore, vHeadTpos) = eGridTile_Head;
		auto vTailTpos						 = SDL_Point{vHeadTpos.x, vHeadTpos.y - 1};
		fGetTile(vCore, vTailTpos) = eGridTile_Body;
		vCore.vPlayer.vMoveCurr		 = {0, 0};
		vCore.vPlayer.vMovePrev		 = vCore.vPlayer.vMoveCurr;
		vCore.vPlayer.vBodyCurr		 = {
			 {vHeadTpos},
			 {vTailTpos},
		 };
		vCore.vPlayer.vBodyPrev = vCore.vPlayer.vBodyCurr;
		auto vXposOrigin				= 0;
		auto vYposOrigin				= vCore.vGridSize.y;
		for(auto vYpos = 0; vYpos < vCore.vGridSize.y; vYpos++)
		{
			auto vYkey = vYpos * vCore.vGridSize.x;
			for(auto vXpos = 0; vXpos < vCore.vGridSize.x; vXpos++)
			{
				vCore.vDrawer.vRectList[vYkey + vXpos] = {
					.x = (vXposOrigin + vXpos) * vCore.vTileSize.x,
					.y = (vYposOrigin - vYpos) * vCore.vTileSize.y,
					.w = +vCore.vTileSize.x,
					.h = -vCore.vTileSize.y,
				};
			}
		}
		//final
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
#if 0
    // i'll have it here as a reminder
    // that this kind of check
    // is a stupid silly nonsense
    // that would allow our snake
    // to move into itself
    // with a double turn
    // in a single timestep
	case SDLK_w:
	{
		auto &vMove = vCore.vPlayer.vMove;
		if(vMove.y == 0)
		{
			vMove.x = +0;
			vMove.y = +1;
		}
	}
	break;
	case SDLK_a:
	{
		auto &vMove = vCore.vPlayer.vMove;
		if(vMove.x == 0)
		{
			vMove.x = -1;
			vMove.y = -0;
		}
	}
	break;
	case SDLK_s:
	{
		auto &vMove = vCore.vPlayer.vMove;
		if(vMove.y == 0)
		{
			vMove.x = -0;
			vMove.y = -1;
		}
	}
	break;
	case SDLK_d:
	{
		auto &vMove = vCore.vPlayer.vMove;
		if(vMove.x == 0)
		{
			vMove.x = +1;
			vMove.y = +0;
		}
	}
	break;
#else
	case SDLK_w:
	{
		auto &vMoveCurr = vCore.vPlayer.vMoveCurr;
		auto &vHeadTpos = vCore.vPlayer.vBodyCurr.at(0);
		auto &vTailTpos = vCore.vPlayer.vBodyCurr.at(1);
		if((vHeadTpos.y + 1) != vTailTpos.y)
		{
			vMoveCurr.x = +0;
			vMoveCurr.y = +1;
		}
	}
	break;
	case SDLK_a:
	{
		auto &vMoveCurr = vCore.vPlayer.vMoveCurr;
		auto &vHeadTpos = vCore.vPlayer.vBodyCurr.at(0);
		auto &vTailTpos = vCore.vPlayer.vBodyCurr.at(1);
		if((vHeadTpos.x - 1) != vTailTpos.x)
		{
			vMoveCurr.x = -1;
			vMoveCurr.y = -0;
		}
	}
	break;
	case SDLK_s:
	{
		auto &vMoveCurr = vCore.vPlayer.vMoveCurr;
		auto &vHeadTpos = vCore.vPlayer.vBodyCurr.at(0);
		auto &vTailTpos = vCore.vPlayer.vBodyCurr.at(1);
		if((vHeadTpos.y - 1) != vTailTpos.y)
		{
			vMoveCurr.x = -0;
			vMoveCurr.y = -1;
		}
	}
	break;
	case SDLK_d:
	{
		auto &vMoveCurr = vCore.vPlayer.vMoveCurr;
		auto &vHeadTpos = vCore.vPlayer.vBodyCurr.at(0);
		auto &vTailTpos = vCore.vPlayer.vBodyCurr.at(1);
		if((vHeadTpos.x + 1) != vTailTpos.x)
		{
			vMoveCurr.x = +1;
			vMoveCurr.y = +0;
		}
	}
	break;
#endif
	default: break;
	}
	return 0;
}//fHitK
inline static bool fTick(tCore &vCore)
{
	//player
	auto &vBodyCurr = vCore.vPlayer.vBodyCurr;
	auto &vBodyPrev = vCore.vPlayer.vBodyPrev;
	vBodyPrev				= vBodyCurr;
	auto &vMoveCurr = vCore.vPlayer.vMoveCurr;
	auto &vMovePrev = vCore.vPlayer.vMovePrev;
	vMovePrev				= vMoveCurr;
	if(vMoveCurr.x || vMoveCurr.y)
	{
		auto &vCurrTpos = vBodyCurr.at(0);
		auto	vNextTpos = SDL_Point{
			 vCurrTpos.x + vMoveCurr.x,
			 vCurrTpos.y + vMoveCurr.y,
		 };
		auto &vFoodTpos = vCore.vFoodTpos;
		auto	vTileEnum = fGetTile(vCore, vFoodTpos);
		while(vTileEnum == eGridTile_Body || vTileEnum == eGridTile_Head)
		{
			vFoodTpos.x = vCore.vRandom.vSint(vCore.vRandom.vCore);
			vFoodTpos.x = vFoodTpos.x % vCore.vGridSize.x;
			vFoodTpos.y = vCore.vRandom.vSint(vCore.vRandom.vCore);
			vFoodTpos.y = vFoodTpos.y % vCore.vGridSize.y;
			vTileEnum		= fGetTile(vCore, vFoodTpos);
		}
		fGetTile(vCore, vFoodTpos) = eGridTile_Food;
		if(vFoodTpos.x == vNextTpos.x && vFoodTpos.x == vNextTpos.y)
		{
			vBodyCurr.push_back(vNextTpos);
			vBodyPrev.push_back(vNextTpos);
		}
		else
		{
			fGetTile(vCore, vCurrTpos) = eGridTile_None;
			vCurrTpos									 = vNextTpos;
			fGetTile(vCore, vCurrTpos) = eGridTile_Head;
			//body
			for(auto vIter = 1; vIter < vBodyCurr.size(); vIter++)
			{
				auto &vNextTpos						 = vBodyPrev.at(vIter - 1);
				auto &vCurrTpos						 = vBodyCurr.at(vIter);
				fGetTile(vCore, vCurrTpos) = eGridTile_None;
				vCurrTpos									 = vNextTpos;
				fGetTile(vCore, vCurrTpos) = eGridTile_Body;
			}
		}
	}
	//final
	return 1;
}
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
	vCore.vTicker.vMilWas = vCore.vTicker.vMilNow;
	vCore.vTicker.vMilNow = SDL_GetTicks();
	auto vTickerSecWas
		= vCore.vTicker.vMilWas
		/ (1'000 * vCore.vTicker.vSecMul / vCore.vTicker.vSecDiv);
	auto vTickerSecNow
		= vCore.vTicker.vMilNow
		/ (1'000 * vCore.vTicker.vSecMul / vCore.vTicker.vSecDiv);
	if(vTickerSecWas != vTickerSecNow)
	{
		fTick(vCore);
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
#if 1
	for(auto vYpos = 0; vYpos < vCore.vGridSize.y; vYpos++)
	{
		auto vYkey = vYpos * vCore.vGridSize.x;
		for(auto vXpos = 0; vXpos < vCore.vGridSize.x; vXpos++)
		{
			auto vTile = vCore.vTileGrid[vYkey + vXpos];
			auto vTint = SDL_Color{.r = 0x00, .g = 0x00, .b = 0x00, .a = 0xff};
			switch(vTile)
			{
			case eGridTile_Head: vTint.g = 0xff; break;
			case eGridTile_Body: vTint.g = 0x80; break;
			case eGridTile_Food: vTint.r = 0xff; break;
			case eGridTile_Wall: vTint.b = 0x80; break;
			default: break;
			}
			SDL_SetRenderDrawColor(
				vCore.vDrawer.vHand, vTint.r, vTint.g, vTint.b, vTint.a
			);
			auto vRectData = &vCore.vDrawer.vRectList[vYkey + vXpos];
			SDL_RenderFillRect(vCore.vDrawer.vHand, vRectData);
		}
	}
#else
	SDL_RenderFillRects(
		vCore.vDrawer.vHand,
		vCore.vDrawer.vRectList.data(),
		vCore.vDrawer.vRectList.size()
	);
#endif
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
			.vTicker = {.vSecMul = 1, .vSecDiv = 5},
		});
		nSnakIter::fInit(vCore);
		nSnakIter::fWork(vCore);
		nSnakIter::fQuit(vCore);
	}
	return EXIT_SUCCESS;
}//main
#endif//dSnakIter_Cpp
