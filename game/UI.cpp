#include "stdafx.h"
#include "UI.h"

UI::UI(void){}
UI::~UI(void){}

void UI::OnInitialize() 
{
	UIMode = GAME;

	static bool firstStart = true;

	if (firstStart)
	{
		UIMode = MAIN_MENU;
		firstStart = false;
		music.Play("menuMusic.ogg", -1);
		music.Volume(0.3);
	}

	width = myGame->GetWidth();
	height = myGame->GetHeight();

	deathCounter = 0;
	mapActive = false;

	LoadDialogue();

	LoadTileGenerator();
	MapGenerator("NewMap.txt");

	playerIcon.LoadImageW("playerIcon.bmp", CColor::Black());
	playerIcon.SetImage("playerIcon.bmp");

	skullIcon.LoadImageW("skull.bmp", CColor::Black());
	skullIcon.SetImage("skull.bmp");
	skullIcon.SetPosition(30, height - 40);
	skullIcon.SetSize(50, 50);

	// Main Menu
	newGame.LoadImageW("newGame.bmp", CColor::Black());
	newGame.LoadImageW("newGameS.bmp", CColor::Black());
	newGame.SetImage("newGame.bmp");
	newGame.SetPosition(width / 2, height / 2);

	exit.LoadImageW("exit.bmp", CColor::Black());
	exit.LoadImageW("exitS.bmp", CColor::Black());
	exit.SetImage("exit.bmp");
	exit.SetPosition(width / 2, height / 2 - 100);

	// Pause Menu
	resume.LoadImageW("resume.bmp", CColor::Black());
	resume.LoadImageW("resumeS.bmp", CColor::Black());
	resume.SetImage("resume.bmp");
	resume.SetPosition(width / 2, height / 2 + 100);

	controls.LoadImageW("controls.bmp", CColor::Black());
	controls.LoadImageW("controlsS.bmp", CColor::Black());
	controls.SetImage("controls.bmp");
	controls.SetPosition(width / 2, height / 2);

	tutorial.LoadImageW("tutorial01.bmp", CColor::Green());
	tutorial.LoadImageW("tutorial02.bmp", CColor::Green());
	tutorial.LoadImageW("tutorial03.bmp", CColor::Green());
	tutorial.LoadImageW("tutorial04.bmp", CColor::Green());
	tutorial.LoadImageW("tutorial05.bmp", CColor::Green());
	tutorial.LoadImageW("tutorial06.bmp", CColor::Green());
	tutorial.SetImage("tutorial01.bmp");
	tutorial.SetPosition(width / 2 + 200, height / 2);

	basic.LoadImageW("basic.bmp", CColor::Black());
	basic.LoadImageW("basicS.bmp", CColor::Black());
	basic.SetImage("basic.bmp");
	basic.SetPosition(width / 2 - 300, height / 2 + 230);

	climb.LoadImageW("climb.bmp", CColor::Black());
	climb.LoadImageW("climbS.bmp", CColor::Black());
	climb.SetImage("climb.bmp");
	climb.SetPosition(width / 2 - 300, height / 2 + 140);

	doubleJump.LoadImageW("jump.bmp", CColor::Black());
	doubleJump.LoadImageW("jumpS.bmp", CColor::Black());
	doubleJump.SetImage("jump.bmp");
	doubleJump.SetPosition(width / 2 - 300, height / 2 + 50);

	dash.LoadImageW("dash.bmp", CColor::Black());
	dash.LoadImageW("dashS.bmp", CColor::Black());
	dash.SetImage("dash.bmp");
	dash.SetPosition(width / 2 - 300, height / 2 - 40);

	jumpOrb.LoadImageW("jumpOrb.bmp", CColor::Black());
	jumpOrb.LoadImageW("jumpOrbS.bmp", CColor::Black());
	jumpOrb.SetImage("jumpOrb.bmp");
	jumpOrb.SetPosition(width / 2 - 300, height / 2 - 130);

	spiritBow.LoadImageW("bow.bmp", CColor::Black());
	spiritBow.LoadImageW("bowS.bmp", CColor::Black());
	spiritBow.SetImage("bow.bmp");
	spiritBow.SetPosition(width / 2 - 300, height / 2 - 220);

	back.LoadImageW("back.bmp", CColor::Black());
	back.LoadImageW("backS.bmp", CColor::Black());
	back.SetImage("back.bmp");
	back.SetPosition(width / 2, height / 2 - 280);

	tutorialBG.LoadImageW("tutorialBG.bmp", CColor::Black());
	tutorialBG.SetImage("tutorialBG.bmp");
	tutorialBG.SetPosition(width / 2, height / 2);
	tutorialBG.SetSize(width / 1.1, height / 1.1);

	pauseBG.LoadImageW("black.bmp", CColor::Green());
	pauseBG.SetImage("black.bmp");
	pauseBG.SetPosition(width / 2, height / 2);
}

void UI::OnUpdate(float time, float delta)
{
	playerIcon.Update(time);

	PlayAudio(myGame->mapArray[myGame->mapX][myGame->mapY].musicName);

	playerIcon.SetSize(30, 30);
	playerIcon.SetPosition(mapArray[myGame->mapX][myGame->mapY].mapTile->GetPosition());

	for (CSprite* dialogues : dialogueList)
	{
		dialogues->Update(time);
		dialogues->SetPosition(myGame->player.GetX(), myGame->player.GetY() + myGame->player.GetHeight());
		dialogues->SetHealth(dialogues->GetHealth() - delta / 1000);

		if (dialogues->GetHealth() < 0) dialogues->Delete();
	}
	dialogueList.delete_if(deleted);

	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < 20; j++)
		{
			mapArray[i][j].mapTile->Update(time);
		}
	}

	mapArray[myGame->mapX][myGame->mapY].isActive = true;
}

void UI::OnDraw(CGraphics* g)
{
	//*g << left << xy(200, height - 50) << font("ChavaRegular.ttf", 30) << color(CColor::White()) << myGame->mapNumber;
	if (myGame->mapNumber != 0)
	for (CSprite* dialogues : dialogueList) dialogues->Draw(g);

	if (myGame->mapNumber < 12)FragmentQuest(g);

	if (mapActive)
	{
		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				if (mapArray[i][j].isActive) mapArray[i][j].mapTile->Draw(g);
			}
		}
		playerIcon.Draw(g);
	}

	if (deathCounter >= 10)
	{
		skullIcon.Draw(g);
		*g << left << xy(60, height - 50) << font("ChavaRegular.ttf", 30) << color(CColor::White()) << deathCounter;
	}

	if (exitBool) *g << center << xy(width / 2, height - 200) << font("ChavaRegular.ttf", 30) << color(CColor::White()) << "Press [F] to exit";

	MainMenu(g);
	PauseMenu(g);

	if (myGame->mapNumber == 29)  *g << center << xy(width / 2, height - 320) << font("ChavaRegular.ttf", 30) << color(CColor::White()) << "THE PROTOTYPE 2.0 ENDS HERE. THANK YOU FOR PLAYING!";
}

void UI::OnKeyDown(SDLKey sym)
{
	if (sym == SDLK_m && UIMode == GAME)
	{
		if (mapActive) mapActive = false;
		else mapActive = true;
	}

	if (sym == SDLK_ESCAPE)
	{
		if (UIMode == GAME) UIMode = PAUSE_MENU;
		else if (UIMode == PAUSE_MENU && !tutorialBool) UIMode = GAME;

		if (tutorialBool) tutorialBool = false;
	}
}

void UI::OnLButtonDown(Uint16 x, Uint16 y)
{
	if (UIMode == MAIN_MENU)
	{
		if (newGame.HitTest(x, y))
		{
			myGame->NewGame();
			UIMode = GAME;
		}

		if (exit.HitTest(x, y)) myGame->StopGame();
	}

	if (UIMode == PAUSE_MENU)
	{
		if (resume.HitTest(x, y) && !tutorialBool) UIMode = GAME;
		if (controls.HitTest(x, y))
		{
			tutorialBool = true;
			tutorial.SetImage("tutorial01.bmp");
		}
		if (exit.HitTest(x, y) && !tutorialBool) myGame->StopGame();

		if (basic.HitTest(x, y)) tutorial.SetImage("tutorial01.bmp");
		if (climb.HitTest(x, y)) tutorial.SetImage("tutorial02.bmp");
		if (doubleJump.HitTest(x, y)) tutorial.SetImage("tutorial03.bmp");
		if (dash.HitTest(x, y)) tutorial.SetImage("tutorial04.bmp");
		if (jumpOrb.HitTest(x, y)) tutorial.SetImage("tutorial05.bmp");
		if (spiritBow.HitTest(x, y)) tutorial.SetImage("tutorial06.bmp");
		if (back.HitTest(x, y)) tutorialBool = false;
	}
}

void UI::MainMenu(CGraphics* g)
{
	if (UIMode == MAIN_MENU)
	{
		newGame.Draw(g);
		exit.Draw(g);

		if (newGame.HitTest(myGame->GetMouseCoords())) newGame.SetImage("newGameS.bmp");
		else newGame.SetImage("newGame.bmp");

		if (exit.HitTest(myGame->GetMouseCoords())) exit.SetImage("exitS.bmp");
		else exit.SetImage("exit.bmp");
	}
}

void UI::PauseMenu(CGraphics* g)
{
	tutorial.Update(GetTime());

	if (UIMode == PAUSE_MENU)
	{
		music.Volume(0.05);
		pauseBG.Draw(g);
		resume.Draw(g);
		controls.Draw(g);
		exit.Draw(g);

		if (resume.HitTest(myGame->GetMouseCoords())) resume.SetImage("resumeS.bmp");
		else resume.SetImage("resume.bmp");

		if (controls.HitTest(myGame->GetMouseCoords())) controls.SetImage("controlsS.bmp");
		else controls.SetImage("controls.bmp");

		if (exit.HitTest(myGame->GetMouseCoords())) exit.SetImage("exitS.bmp");
		else exit.SetImage("exit.bmp");

		if (tutorialBool)
		{
			tutorialBG.Draw(g);
			tutorial.Draw(g);
			basic.Draw(g);
			climb.Draw(g);
			doubleJump.Draw(g);
			dash.Draw(g);
			jumpOrb.Draw(g);
			spiritBow.Draw(g);
			back.Draw(g);

			if (basic.HitTest(myGame->GetMouseCoords())) basic.SetImage("basicS.bmp");
			else basic.SetImage("basic.bmp");

			if (climb.HitTest(myGame->GetMouseCoords())) climb.SetImage("climbS.bmp");
			else climb.SetImage("climb.bmp");

			if (doubleJump.HitTest(myGame->GetMouseCoords())) doubleJump.SetImage("jumpS.bmp");
			else doubleJump.SetImage("jump.bmp");

			if (dash.HitTest(myGame->GetMouseCoords())) dash.SetImage("dashS.bmp");
			else dash.SetImage("dash.bmp");

			if (jumpOrb.HitTest(myGame->GetMouseCoords())) jumpOrb.SetImage("jumpOrbS.bmp");
			else jumpOrb.SetImage("jumpOrb.bmp");

			if (spiritBow.HitTest(myGame->GetMouseCoords())) spiritBow.SetImage("bowS.bmp");
			else spiritBow.SetImage("bow.bmp");

			if (back.HitTest(myGame->GetMouseCoords())) back.SetImage("backS.bmp");
			else back.SetImage("back.bmp");
		}
	}
	else 	music.Volume(0.15);
}

void UI::LoadDialogue()
{
	for (size_t i = 0; i < 100; i++) dialogue[i] = " ";

	dialogue[1] = "A prison? Really?";
	dialogue[3] = "Looks like it's night time.";
	dialogue[10] = "I'm so close!";
	dialogue[11] = "Finally! The exit!";
	dialogue[12] = "Where am I?";
	dialogue[13] = "A Castle?";
	dialogue[14] = "Just as I thought";
	dialogue[17] = "Interesting";
	dialogue[19] = "This is a tall tower";
	dialogue[23] = "A bow? That's what I need!";
	dialogue[24] = "And what's in here?";
	dialogue[29] = "It is over?";
}

void UI::PlayDialogue(int _dialogueN)
{
	if (dialogueList.size() > 0 && _dialogueN == 0) dialogueList.delete_all();
		string tempDialogue;

	switch (_dialogueN)
	{
	case 0:
		tempDialogue = dialogue[myGame->mapNumber];
		break;
	case 1:
		tempDialogue = "I need this to escape!";
		break;
	case 2:
		tempDialogue = "Climb ability unlocked";
		break;
	case 3:
		tempDialogue = "Doube Jump unlocked";
		break;
	case 4:
		tempDialogue = "Dash Unlocked";
		break;
	case 5:
		tempDialogue = "10 times? Already?";
		break;
	case 6:
		tempDialogue = "Spirit bow acquired";
		break;
	}

	CSprite* dialogueText = new CSpriteText(myGame->player.GetX(), myGame->player.GetY() + myGame->player.GetHeight(), "ChavaRegular.ttf", 25, tempDialogue, CColor::White(), myGame->GetTime());
	dialogueList.push_back(dialogueText);
	dialogueText->SetHealth(3);
	dialogueText->SetSize(50, 50);

	dialogue[myGame->mapNumber] = " ";

}

void UI::FragmentQuest(CGraphics* g)
{
	if (myGame->gFragmentNumber > 0)
	{
		*g << right << xy(width - 30, height - 40) << font("ChavaRegular.ttf", 32) << color(CColor(215, 169, 55)) << "The key to escape!";
		if (myGame->gFragmentNumber < 4)
		{
			*g << right << xy(width - 30, height - 70) << font("ChavaRegular.ttf", 30) << color(CColor::Red()) << "Fragments: " << myGame->gFragmentNumber << "/4";
		}
		else
		{
			*g << right << xy(width - 30, height - 70) << font("ChavaRegular.ttf", 30) << color(CColor::DarkGreen()) << "Quest completed!";
			*g << right << xy(width - 30, height - 100) << font("ChavaRegular.ttf", 30) << color(CColor::White()) << "Find the prison door!";
		}
	}
}

void UI::LoadTileGenerator()
{
	for (int i = 0; i < 20; i++)
	{
		tileMap[i] = new CSprite;
	}

	int counter = 1;

	int column = 4;
	int row = 4;

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			tileMap[counter] = new CSprite;
			tileMap[counter]->LoadImageW("map.bmp", "tile", CSprite::Sheet(column, row).Tile(j, i), CColor::Black());
			tileMap[counter]->SetImage("tile");
			tileMap[counter]->SetSize(40, 40);
			tileMap[counter]->SetStatus(counter);
			counter++;
		}
	}
}

void UI::MapGenerator(string map)
{
	for (int j = 0; j < 20; j++)
	{
		for (int i = 0; i < 20; i++)
		{
			mapArray[i][j].isActive = false;
		}
	}

	ifstream data;
	int ID = 0;
	data.open(map);
	int reverseY = 19;
	for (int j = 0; j < 20; j++)
	{
		for (int i = 0; i < 20; i++)
		{
			data >> ID;
			delete mapArray[i][reverseY].mapTile;
			mapArray[i][reverseY].mapTile = new CSprite();
			if (ID > 0)
			{
				mapArray[i][reverseY].mapTile = tileMap[ID]->Clone();
				mapArray[i][reverseY].mapTile->SetPosition(300 + (i * 40 + 20), 820 - (j * 40 + 20));
				mapArray[i][reverseY].mapTile->SetState(ID);
			}
		}
		reverseY--;
	}
	data.close();
}

void UI::PlayAudio(char* name)
{
	static char* tempName;
	if (tempName != name)
	{
		music.Play(name, -1);
		music.Volume(0.15);
	}
	tempName = name;
}