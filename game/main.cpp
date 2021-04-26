#include "stdafx.h"
#include "MyGame.h"

CMyGame game;
CGameApp app;

int main(int argc, char* argv[])
{
	app.OpenConsole();
	//app.OpenWindow(1280, 720, "Escape");
	// Sorry at moment the game only works as intended in 720p
	app.OpenFullScreen(1280, 720, 0);
	app.SetFPS(60);
	app.SetClearColor(CColor::Black());
	app.Run(&game);
	return(0);
}
