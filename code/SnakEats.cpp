#ifndef dSnakEats_Cpp
#define dSnakEats_Cpp
//headers
#include "SnakEats.hpp"
#include "fmt/core.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "fmt/chrono.h"
//content
namespace nSnakEats
{
//typedef
typedef std::default_random_engine				 tRandCore;
typedef std::uniform_int_distribution<int> tRandSint;
typedef struct tFoodTile
{
	SDL_Point vTpos;
} tFoodTile;
typedef std::vector<tFoodTile> tFoodList;
typedef struct tBodyTile
{
	SDL_Point vTpos;
	SDL_Point vMove;
} tBodyTile;
typedef std::vector<tBodyTile> tBodyList;
typedef std::vector<SDL_Rect>	 tRectList;
typedef struct tCore
{
	SDL_Point vGridSize;
	SDL_Point vTileSize;
	tFoodList vFoodList;
	struct
	{
		tBodyList vBody;
		bool			vMove;
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
auto fMake(const tConf &vConf) -> tCore
{
	return tCore{
    .vGridSize = vConf.vGridSize,
    .vTileSize = {vConf.vWindow.vSize.x / vConf.vGridSize.x, vConf.vWindow.vSize.y / vConf.vGridSize.y },
    .vFoodList = tFoodList(vConf.vFoodSize, tFoodTile{vConf.vGridSize.x / 2, vConf.vGridSize.y / 2}),
    .vPlayer = {
      .vBody = { { vConf.vGridSize.x / 2, vConf.vGridSize.y / 2 } },
      .vMove = 1,
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
			dSnakEats_ProjName,
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
		auto vXposOrigin = 0;
		auto vYposOrigin = vCore.vGridSize.y;
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
		//grid
		auto vGridHalfX = vCore.vGridSize.x / 2;
		auto vGridHalfY = vCore.vGridSize.y / 2;
		//player
		vCore.vPlayer.vBody = {
			{.vTpos = {vGridHalfX, vGridHalfY + 0}, .vMove = {0, 1}},
			{.vTpos = {vGridHalfX, vGridHalfY - 1}, .vMove = {0, 1}},
		};
		auto &vHeadTpos = vCore.vPlayer.vBody.at(0).vTpos;
		//food
		for(auto vIter = 0; vIter < vCore.vFoodList.size(); vIter++)
		{
			auto &vFoodIter = vCore.vFoodList.at(vIter);
			auto &vFoodTpos = vFoodIter.vTpos;
			while(vFoodTpos.x == vHeadTpos.x && vFoodTpos.y == vHeadTpos.y)
			{
				vFoodTpos.x = vCore.vRandom.vSint(vCore.vRandom.vCore);
				vFoodTpos.x = vFoodTpos.x % vCore.vGridSize.x;
				vFoodTpos.y = vCore.vRandom.vSint(vCore.vRandom.vCore);
				vFoodTpos.y = vFoodTpos.y % vCore.vGridSize.y;
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
	case SDLK_w:
	{
		auto &vHeadMove = vCore.vPlayer.vBody.at(0).vMove;
		auto &vTailMove = vCore.vPlayer.vBody.at(1).vMove;
		if(vCore.vPlayer.vMove && (not vHeadMove.y || vHeadMove.y != vTailMove.y))
		{
			vHeadMove.x					= +0;
			vHeadMove.y					= +1;
			vCore.vPlayer.vMove = 0;
		}
	}
	break;
	case SDLK_a:
	{
		auto &vHeadMove = vCore.vPlayer.vBody.at(0).vMove;
		auto &vTailMove = vCore.vPlayer.vBody.at(1).vMove;
		if(vCore.vPlayer.vMove && (not vHeadMove.x || vHeadMove.x != vTailMove.x))
		{
			vHeadMove.x					= -1;
			vHeadMove.y					= -0;
			vCore.vPlayer.vMove = 0;
		}
	}
	break;
	case SDLK_s:
	{
		auto &vHeadMove = vCore.vPlayer.vBody.at(0).vMove;
		auto &vTailMove = vCore.vPlayer.vBody.at(1).vMove;
		if(vCore.vPlayer.vMove && (not vHeadMove.y || vHeadMove.y != vTailMove.y))
		{
			vHeadMove.x					= -0;
			vHeadMove.y					= -1;
			vCore.vPlayer.vMove = 0;
		}
	}
	break;
	case SDLK_d:
	{
		auto &vHeadMove = vCore.vPlayer.vBody.at(0).vMove;
		auto &vTailMove = vCore.vPlayer.vBody.at(1).vMove;
		if(vCore.vPlayer.vMove && (not vHeadMove.x || vHeadMove.x != vTailMove.x))
		{
			vHeadMove.x					= +1;
			vHeadMove.y					= +0;
			vCore.vPlayer.vMove = 0;
		}
	}
	break;
	default: break;
	}
	return 0;
}//fHitK
inline static bool fTick(tCore &vCore)
{
	//body
	auto	vBody = tBodyList(vCore.vPlayer.vBody);
	auto &vMove = vBody.at(0).vMove;
	auto &vTpos = vBody.at(0).vTpos;
	vTpos.x			= (vTpos.x + vMove.x);
	if(vTpos.x < 0)
	{
		vTpos.x = vCore.vGridSize.x - 1;
	}
	else if(vTpos.x >= vCore.vGridSize.x)
	{
		vTpos.x = 0;
	}
	vTpos.y = (vTpos.y + vMove.y);
	if(vTpos.y < 0)
	{
		vTpos.y = vCore.vGridSize.y - 1;
	}
	else if(vTpos.y >= vCore.vGridSize.y)
	{
		vTpos.y = 0;
	}
	for(auto vIter = 1; vIter < vBody.size(); vIter++)
	{
		auto &vPrevTile = vCore.vPlayer.vBody.at(vIter - 1);
		auto &vPrevTpos = vPrevTile.vTpos;
		auto &vPrevMove = vPrevTile.vMove;
		auto &vThisTile = vBody.at(vIter + 0);
		auto &vThisTpos = vThisTile.vTpos;
		auto &vThisMove = vThisTile.vMove;
		if(vTpos.x == vThisTpos.x && vTpos.y == vThisTpos.y)
		{
			return fStop(vCore);
		}
		if(0
      || (vThisTpos.x == vPrevTpos.x && vThisMove.y != vPrevMove.y)
      || (vThisTpos.y == vPrevTpos.y && vThisMove.x != vPrevMove.x)
      )
		{
			vThisMove = vPrevMove;
		}
		vThisTpos.x = vPrevTpos.x;
		vThisTpos.y = vPrevTpos.y;
	}
	//food
	for(auto vIter = 0; vIter < vCore.vFoodList.size(); vIter++)
	{
		auto &vFoodIter = vCore.vFoodList.at(vIter);
		auto &vFoodTpos = vFoodIter.vTpos;
		while(vFoodTpos.x == vTpos.x && vFoodTpos.y == vTpos.y)
		{
			auto vBodyTail = vBody.back();
			vBodyTail.vTpos.x -= vBodyTail.vMove.x;
			vBodyTail.vTpos.y -= vBodyTail.vMove.y;
			vBody.push_back(vBodyTail);
			vFoodTpos.x = vCore.vRandom.vSint(vCore.vRandom.vCore);
			vFoodTpos.x = vFoodTpos.x % vCore.vGridSize.x;
			vFoodTpos.y = vCore.vRandom.vSint(vCore.vRandom.vCore);
			vFoodTpos.y = vFoodTpos.y % vCore.vGridSize.y;
		}
	}
	vCore.vPlayer.vBody = std::move(vBody);
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
		vCore.vPlayer.vMove = 1;
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
	//body
	const auto &vBodyList = vCore.vPlayer.vBody;
	for(auto vIter = 1; vIter < vCore.vPlayer.vBody.size(); vIter++)
	{
		const auto &vBodyTile = vBodyList.at(vIter);
		const auto &vBodyTpos = vBodyTile.vTpos;
    if (vIter % 2)
    {
      SDL_SetRenderDrawColor(vCore.vDrawer.vHand, 0x00, 0x80, 0x00, 0xff);
    }
    else
    {
      SDL_SetRenderDrawColor(vCore.vDrawer.vHand, 0x00, 0xa0, 0x00, 0xff);
    }
		const auto	vRectIter = vBodyTpos.y * vCore.vGridSize.x + vBodyTpos.x;
		const auto &vRectData = &vCore.vDrawer.vRectList[vRectIter];
		SDL_RenderFillRect(vCore.vDrawer.vHand, vRectData);
	}
	//head
	SDL_SetRenderDrawColor(vCore.vDrawer.vHand, 0x00, 0xff, 0x00, 0xff);
	const auto &vHeadTile = vBodyList.at(0);
	const auto &vHeadTpos = vHeadTile.vTpos;
	const auto	vRectIter = vHeadTpos.y * vCore.vGridSize.x + vHeadTpos.x;
	const auto &vRectData = &vCore.vDrawer.vRectList[vRectIter];
	SDL_RenderFillRect(vCore.vDrawer.vHand, vRectData);
	//food
	for(auto vIter = 0; vIter < vCore.vFoodList.size(); vIter++)
	{
		const auto &vFood = vCore.vFoodList.at(vIter);
		SDL_SetRenderDrawColor(vCore.vDrawer.vHand, 0xff, 0x00, 0x00, 0xff);
		const auto	vRectIter = vFood.vTpos.y * vCore.vGridSize.x + vFood.vTpos.x;
		const auto &vRectData = &vCore.vDrawer.vRectList[vRectIter];
		SDL_RenderFillRect(vCore.vDrawer.vHand, vRectData);
	}
	//final
	SDL_RenderPresent(vCore.vDrawer.vHand);
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
#if defined(dSnakEats_MakeTest)
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
#endif//ifd(dSnakEats_MakeTest)
}//namespace nSnakEats
//actions
int main(int vArgC, char **vArgV, char **vEnvi)
{
	using namespace nSnakEats;
#ifdef dSnakEats_MakeTest
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
#endif//ifd(dSnakEats_MakeTest)
	{
		auto vCore = nSnakEats::fMake({
      .vGridSize = {.x = 0x08, .y = 0x08},
			.vWindow = {.vSize = {.x = 0x200, .y = 0x200}},
			.vTicker = {.vSecMul = 1, .vSecDiv = 10},
		});
		nSnakEats::fInit(vCore);
		nSnakEats::fWork(vCore);
		nSnakEats::fQuit(vCore);
	}
	return EXIT_SUCCESS;
}//main
#endif//dSnakEats_Cpp
