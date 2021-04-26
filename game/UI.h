#pragma once
#include "MyGame.h"

class UI : public CGame
{
	int width, height;
	string dialogue[100];

	bool mapActive;

	CSpriteList dialogueList;

	struct mapComponents
	{
		CSprite* mapTile;
		CSprite* extraItem;
		bool isActive;
	};

	CSprite playerIcon;
	CSprite skullIcon;

	CSprite* tileMap[20];
	mapComponents mapArray[20][20];

	CSprite newGame, exit, controls, resume;
	CSprite tutorial, tutorialBG, basic, climb, doubleJump, dash, jumpOrb, spiritBow, back, pauseBG;
	bool tutorialBool = false;

public:
	enum { MAIN_MENU, GAME, PAUSE_MENU } UIMode;

	CMyGame* myGame;
	int deathCounter;
	CSoundPlayer music;
	bool exitBool = false;

public:
	UI(void);
	~UI(void);

	virtual void OnInitialize();
	virtual void OnUpdate(float time, float delta);
	virtual void OnDraw(CGraphics* g);
	virtual void OnKeyDown(SDLKey sym);
	virtual void OnLButtonDown(Uint16 x, Uint16 y);

	void MainMenu(CGraphics* g);
	void PauseMenu(CGraphics* g);
	void LoadDialogue();
	void PlayDialogue(int _dialogueN);
	void FragmentQuest(CGraphics* g);
	void LoadTileGenerator();
	void MapGenerator(string map);
	void PlayAudio(char* name);
};