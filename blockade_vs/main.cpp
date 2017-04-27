#pragma once
#include "VoxelRenderBase.h"
#include "GameBase.h"

using namespace Eigen;
using namespace std;
using namespace SoftRendering;


int main(int argc, char* argv[]) {// Get current flag
//int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

// Turn on leak-checking bit.
//tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

// Turn off CRT block checking bit.
//tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

// check integrity of heap before every alloc /delloc
//tmpFlag &= _CRTDBG_CHECK_ALWAYS_DF;
// Set flag to the new value.
//_CrtSetDbgFlag(tmpFlag);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
	GameBase theGame;

	theGame.InitApp();
	theGame.GameLoop();


	return 0;

}
