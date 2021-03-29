#include "stdafx.h"
#include "MyGame.h"
#include <math.h>

CMyGame::CMyGame(void)
{
	//The game could be hard, press F1 for god mode (the spikes can't kill you after)
	//Release mode for the best game experience
}

CMyGame::~CMyGame(void)
{

}

void CMyGame::OnUpdate()
{
	CharacterController();

	Uint32 t = GetTime();
	player.Update(t);
	player.SetSize(size, size);
	tileBrush.Update(t);
	backGround.Update(t);

	if (editorMode) TileBrush();
	ParticleSystem();

	CVector n = Normalise(player.GetVelocity());
	bool slideBool = false;
	bool jumpBool = false;

	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 18; j++)
		{
			tileArray[i][j]->Update(t);

			// In editor mode if Q is pressed, de tile on the mouse position deleted
			if (IsKeyDown(SDLK_q) && tileArray[i][j]->HitTest(GetMouseCoords()) && editorMode)
			{
				tileArray[i][j]->SetState(0);
				tileArray[i][j]->Delete();
				tileArray[i][j]->SetProperty("tag", "NULL");
			}

			// Platform and player reaction
			if ((string)tileArray[i][j]->GetProperty("tag") == "platform")
			{
				// If player touch the left side you can climb on it (if the skill is unlocked) also reset the jumps
				if (tileArray[i][j]->HitTest(player.GetLeft() + 6, player.GetY())  && tileArray[i][j]->GetX() < player.GetX() - n.m_x)
				{
					player.SetPosition(tileArray[i][j]->GetX() + tileArray[i][j]->GetHeight() / 2 + player.GetHeight() / 2 - 6, player.GetY());
					player.SetXVelocity(0);
					if (IsKeyDown(SDLK_a))
					{
						if (slideStatus != SLIDE_INACTIVE)
						{
							jumpN = 2;
							slideStatus = LEFT_SLIDE;
						}
						if (dashStatus != DASH_INACTIVE) dashStatus = DASH_READY;
						slideBool = true;
					}
				}

				// same but right side
				if (tileArray[i][j]->HitTest(player.GetRight() - 6, player.GetY())  && tileArray[i][j]->GetX() > player.GetX() - n.m_x)
				{
					player.SetPosition(tileArray[i][j]->GetX() - tileArray[i][j]->GetHeight() / 2 - player.GetHeight() / 2 + 6, player.GetY());
					player.SetXVelocity(0);
					if (IsKeyDown(SDLK_d))
					{
						if (slideStatus != SLIDE_INACTIVE)
						{
							jumpN = 2;
							slideStatus = RIGHT_SLIDE;
						}
						if (dashStatus != DASH_INACTIVE) dashStatus = DASH_READY;
						slideBool = true;
					}
				}

				// When the player bottom position touch the ground, you can walk on it (set back the position every time)
				if (tileArray[i][j]->HitTest(player.GetX(), player.GetBottom()) && tileArray[i][j]->GetY() < player.GetY() - n.m_y && (!slideBool || slideStatus == SLIDE_INACTIVE))
				{
					if (dashStatus != DASH_INACTIVE) dashStatus = DASH_READY;
					player.SetYVelocity(0);
					player.SetPosition(player.GetX(), tileArray[i][j]->GetY() + tileArray[i][j]->GetHeight() / 2 + player.GetHeight() / 2);
					jumpN = 2;
					status = RUN;
					jumpBool = true;
				}

				// When the player hit the tiles with the top position, the velocity set 0 and start falling again
				if (tileArray[i][j]->HitTest(player.GetX(), player.GetTop()) && tileArray[i][j]->GetY() > player.GetY() - n.m_y)
				{
					player.SetPosition(player.GetX(), tileArray[i][j]->GetY() - tileArray[i][j]->GetHeight() / 2 - player.GetHeight() / 2);
					player.SetYVelocity(0);
				}
			}

			// Fragment is needed to open the door (spoiler), the last room background is change, depend how much fragment you have
			if ((string)tileArray[i][j]->GetProperty("tag") == "fragment" && tileArray[i][j]->HitTest(&player))
			{
				PickUpFragment(CColor(36, 113, 31), tileArray[i][j]->GetPosition());

				tileArray[i][j]->SetState(-tileArray[i][j]->GetState());
				tileArray[i][j]->SetProperty("tag", "NULL");

				gFragmentNumber++;

				switch (gFragmentNumber)
				{
				case 1:
					mapArray[10][8].backGround = "prisonBG5.bmp";
					break;
				case 2:
					mapArray[10][8].backGround = "prisonBG6.bmp";
					break;
				case 3:
					mapArray[10][8].backGround = "prisonBG7.bmp";
					break;
				case 4:
					mapArray[10][8].backGround = "prisonBG8.bmp";
					break;
				}
				SaveMap(mapArray[mapX][mapY].mapName);
				mapArray[mapX][mapY].newGame = false;
			}
			if (mapX == 10 && mapY == 8)
			{
				// If you have all the fragments, you can enter the last map (prototype last)
				float pDistance = Distance(GetWidth() / 2, GetHeight() / 4, player.GetX(), player.GetY());
				if (gFragmentNumber > 3 && pDistance < 150)
				{
					textBool = true;
					if (IsKeyDown(SDLK_f))
					{
						textBool = true;
						mapX = 10;
						mapY = 10;
						MapGenerator(mapArray[mapX][mapY].mapName);
					}
				}
				else textBool = false;
			}

			// Set the checkpoint position
			if ((string)tileArray[i][j]->GetProperty("tag") == "checkpoint")
			{
				float distance = Distance(tileArray[i][j]->GetX(), tileArray[i][j]->GetY(), player.GetX(), player.GetY());
				if (distance < 100)
				{
					respawnPos = tileArray[i][j]->GetPosition();
				}
			}

			// Skill Orb is give you different skills such us (climb, dash, double jump (spoiler again))
			if ((string)tileArray[i][j]->GetProperty("tag") == "skillOrb" && tileArray[i][j]->HitTest(&player))
			{
				PickUpSkillOrb(CColor(122, 8, 8), tileArray[i][j]->GetPosition());

				tileArray[i][j]->SetState(-tileArray[i][j]->GetState());
				tileArray[i][j]->SetProperty("tag", "NULL");

				skillOrb++;
				textBool = true;
				switch (skillOrb)
				{
				case 1:
					slideStatus = SLIDE_ACTIVE;
					break;
				case 2:
					jumpStatus = DOUBLEJUMP_ACTIVE;
					break;
				case 3:
					dashStatus = DASH_ACTIVE;
					break;
				}
				SaveMap(mapArray[mapX][mapY].mapName);
				mapArray[mapX][mapY].newGame = false;
			}
			else if ((string)tileArray[i][j]->GetProperty("tag") == "skillOrb")
			{
				float rotateTo = LookAt(player.GetX(), player.GetY(), tileArray[i][j]->GetX(), tileArray[i][j]->GetY());
				tileArray[i][j]->SetRotation(rotateTo + 90);
			}

			// Sand platform is the classic platform what you can destroy if you dash thorugh 
			if (tileArray[i][j]->HitTest(&player) && dashStatus == DASH_OFF && (string)tileArray[i][j]->GetProperty("tag") == "sand")
			{
				DeletePlatform(*tileArray[i][j]);
			}
			else if (tileArray[i][j]->HitTest(player.GetX(), player.GetBottom()) && tileArray[i][j]->GetY() < player.GetY() - n.m_y 
				&& (string)tileArray[i][j]->GetProperty("tag") == "sand")
			{
				if (dashStatus != DASH_INACTIVE) dashStatus = DASH_READY;
				player.SetYVelocity(0);
				player.SetPosition(player.GetX(), tileArray[i][j]->GetY() + tileArray[i][j]->GetHeight() / 2 + player.GetHeight() / 2);
				jumpN = 2;
				status = RUN;
				jumpBool = true;
			}

			// if the palyer touch the spike, it dies and reset the position to the checkpoint position
			if ((string)tileArray[i][j]->GetProperty("tag") == "spike" && tileArray[i][j]->HitTest(&player, 1))
			{
				if (!godMode)
				{
					PlayerDeathParticle();
					player.SetPosition(respawnPos);
				}
			}
		}
	}
	if (!slideBool && slideStatus != SLIDE_INACTIVE) slideStatus = SLIDE_OFF;
	if (!jumpBool) status = JUMP;

	JumpToMap();
}

void CMyGame::OnDraw(CGraphics* g)
{
	backGround.Draw(g);

	// Map drawing
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 18; j++)
		{
			if (tileArray[i][j]->GetState() > 0 && tileArray[i][j]->GetState() != 26) tileArray[i][j]->Draw(g);
			//if (tileArray[i][j]->GetState() > 0) tileArray[i][j]->Draw(g);
		}
	}

	// Editor tiles drawing
	if (showTiles)
		for (int i = 0; i < 100; i++)
		{
			tileMap[i]->Draw(g);
		}

	tileBrush.Draw(g);

	static int prevT = GetTime();

	// Particle drwaing
	for (CSprite* particles : particleList) particles->Draw(g);

	player.Draw(g);

	// FPS counter from you
	if (GetTime() > prevT)
		//*g << left << xy(GetWidth() - 170, GetHeight() - 50) << font("ChavaRegular.ttf", 30) << color(CColor::White()) << "fps = " << (int)(1000 / (GetTime() - prevT));
		prevT = GetTime();

	// Tutorial text drawing (I had to do like this if I wanted "stroke" effect
	if (textBool)
	{
		*g << center << xy(GetWidth() / 2 - 3, GetHeight() / 1.4) << font("ChavaRegular.ttf", 22) << color(CColor::Black()) << mapArray[mapX][mapY].text;
		*g << center << xy(GetWidth() / 2 + 3, GetHeight() / 1.4) << font("ChavaRegular.ttf", 22) << color(CColor::Black()) << mapArray[mapX][mapY].text;
		*g << center << xy(GetWidth() / 2, GetHeight() / 1.4 - 3) << font("ChavaRegular.ttf", 22) << color(CColor::Black()) << mapArray[mapX][mapY].text;
		*g << center << xy(GetWidth() / 2, GetHeight() / 1.4 + 3) << font("ChavaRegular.ttf", 22) << color(CColor::Black()) << mapArray[mapX][mapY].text;
		*g << center << xy(GetWidth() / 2, GetHeight() / 1.4) << font("ChavaRegular.ttf", 22) << color(CColor::White()) << mapArray[mapX][mapY].text;
	}
}

void CMyGame::OnInitialize()
{
	// Loading the backgrounds
	backGround.LoadImageW("prisonBG.bmp", CColor::Black());
	backGround.LoadImageW("prisonBG2.bmp", CColor::Black());
	backGround.LoadImageW("prisonBG3.bmp", CColor::Black());
	backGround.LoadImageW("prisonBG4.bmp", CColor::Black());
	backGround.LoadImageW("prisonBG5.bmp", CColor::Black());
	backGround.LoadImageW("prisonBG6.bmp", CColor::Black());
	backGround.LoadImageW("prisonBG7.bmp", CColor::Black());
	backGround.LoadImageW("prisonBG8.bmp", CColor::Black());
	backGround.LoadImageW("moon.bmp", CColor::Black());
	backGround.SetSize(GetWidth(), GetHeight());
	backGround.SetPosition(GetWidth() / 2, GetHeight() / 2);

	// Setting the different map background, txt to load the map and tutorial text (might be change on the big project)
	mapArray[5][5] = { 1, "MapData01.txt", "prisonBG.bmp", "Press [A] or [D] for movement and [SPACE] for jump (press [F]) to hide this)" };
	mapArray[5][6] = { 2, "MapData02.txt", "prisonBG.bmp" };
	mapArray[6][6] = { 3, "MapData03.txt", "prisonBG2.bmp" };
	mapArray[6][5] = { 4, "MapData04.txt", "prisonBG.bmp" };
	mapArray[6][4] = { 5, "MapData05.txt", "prisonBG.bmp", "Hold [A] or [D] on the wall to stick on it and [S] or [W] to climb" };
	mapArray[7][6] = { 6, "MapData06.txt", "prisonBG.bmp", "While you are on the air press [SPACE] again for double jump "};
	mapArray[7][7] = { 7, "MapData07.txt", "prisonBG.bmp" };
	mapArray[8][7] = { 8, "MapData08.txt", "prisonBG2.bmp" };
	mapArray[8][8] = { 9, "MapData09.txt", "prisonBG.bmp", "Press movement keys + [SHIFT] to Dash (Use it to destroy the sand platform)" };
	mapArray[9][8] = { 10, "MapData10.txt", "prisonBG3.bmp" };
	mapArray[10][8] = { 11, "MapData11.txt", "prisonBG4.bmp", "Press [F] to escape the prison"};
	mapArray[10][10] = { 12, "MapData12.txt", "moon.bmp", "Prototype end here! Thank your playing!"};

	// Player animations
	player.AddImage("player.bmp", "IdleLeft", 10, 6, 0, 5, 3, 5, CColor::Black());
	player.AddImage("player.bmp", "IdleRight", 10, 6, 4, 5, 7, 5, CColor::Black());
	player.AddImage("player.bmp", "RunLeft", 10, 6, 0, 4, 9, 4, CColor::Black());
	player.AddImage("player.bmp", "RunRight", 10, 6, 0, 3, 9, 3, CColor::Black());
	player.AddImage("player.bmp", "ClimbUpLeft", 10, 6, 0, 2, 3, 2, CColor::Black());
	player.AddImage("player.bmp", "ClimbUpRight", 10, 6, 4, 2, 7, 2, CColor::Black());
	player.AddImage("player.bmp", "ClimbDownLeft", 10, 6, 0, 1, 3, 1, CColor::Black());
	player.AddImage("player.bmp", "ClimbDownRight", 10, 6, 4, 1, 7, 1, CColor::Black());
	player.AddImage("player.bmp", "ClimbIdleLeft", 10, 6, 0, 0, 1, 0, CColor::Black());
	player.AddImage("player.bmp", "ClimbIdleRight", 10, 6, 2, 0, 3, 0, CColor::Black());
	player.AddImage("player.bmp", "JumpUpLeft", 10, 6, 4, 0, 4, 0, CColor::Black());
	player.AddImage("player.bmp", "JumpDownLeft", 10, 6, 5, 0, 5, 0, CColor::Black());
	player.AddImage("player.bmp", "JumpUpRight", 10, 6, 6, 0, 6, 0, CColor::Black());
	player.AddImage("player.bmp", "JumpDownRight", 10, 6, 7, 0, 7, 0, CColor::Black());
	player.SetAnimation("IdleRight", 4);
	player.SetSize(size, size);
	player.SetPosition(100, size);

	/*
	slideStatus = SLIDE_ACTIVE;
	jumpStatus = DOUBLEJUMP_ACTIVE;
	dashStatus = DASH_ACTIVE;
	*/

	//New game function
	NewGame();
}


void CMyGame::OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode)
{
	// Quit game (default)
	if (sym == SDLK_F4 && (mod & (KMOD_LALT | KMOD_RALT)))
		StopGame();

	// Ctrl + S saving map in editor mode
	if (sym == SDLK_s && (mod & (KMOD_LCTRL | KMOD_RCTRL)) && editorMode)
		SaveMap(mapArray[mapX][mapY].mapName);

	// God mode for testing / debugging purpose
	if (sym == SDLK_F1)
	{
		if (godMode) godMode = false;
		else godMode = true;
	}

	// F2 new game
	if (sym == SDLK_F2) NewGame();

	// F3 editor mode on /  off
	if (sym == SDLK_F3)
	{
		if (editorMode) editorMode = false;
		else editorMode = true;
	}

	// Tilemap on / off in editor mode
	if (sym == SDLK_e && editorMode)
	{
		if (showTiles) showTiles = false;
		else showTiles = true;
	}

	// Tutorial text diasppear of you press F
	if (sym == SDLK_f) textBool = false;

	// Space jump
	if (sym == SDLK_SPACE && jumpN > 0 && (slideStatus == SLIDE_OFF || slideStatus == SLIDE_INACTIVE))
	{
		if (jumpStatus == DOUBLEJUMP_ACTIVE) jumpN--;
		else jumpN -= 2;
		player.SetYVelocity(450);
		status = JUMP;
	}
}

void CMyGame::OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode)
{
}

void CMyGame::OnLButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnLButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonUp(Uint16 x,Uint16 y)
{
}

// New Game function to reset the game and start over
void CMyGame::NewGame()
{
	particleList.delete_all();

	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < 20; j++)
		{
			mapArray[i][j].newGame = true;
		}
	}

	mapArray[10][8].backGround = "prisonBG4.bmp";

	gFragmentNumber = 0;
	skillOrb = 0;

	size = 40;
	mapX = 5;
	mapY = 5;
	showTiles = false;
	textBool = false;
	editorMode = false;
	godMode = false;

	dashStatus = DASH_INACTIVE;
	slideStatus = SLIDE_INACTIVE;
	jumpStatus = DOUBLEJUMP_INACTIVE;
	align = RIGHT;

	LoadTileGenerator();
	MapGenerator(mapArray[mapX][mapY].mapName);
	player.SetPosition(100, size);
}

// I needed this function so I can set the animation in Update mode without the animation disappear
void CMyGame::Animation(char* name,  int frame)
{
	static char* tempName;
	if (tempName != name)
	player.SetAnimation(name, frame);
	tempName = name;
}

// Character controller
void CMyGame::CharacterController()
{
	CVector vector = CVector(0, 0);

	// Left and right movement (A and D)
	if (IsKeyDown(SDLK_a))
	{
		align = LEFT;
		vector.SetX(-1);
		if (slideStatus == RIGHT_SLIDE)
		{
			slideStatus = SLIDE_OFF;
			status = JUMP;
		}
	}
	if (IsKeyDown(SDLK_d))
	{
		align = RIGHT;
		vector.SetX(1);
		if (slideStatus == LEFT_SLIDE)
		{
			slideStatus = SLIDE_OFF;
			status = JUMP;
		}
	}
	// S and W for the dash movement
	if (IsKeyDown(SDLK_s))
	{
		vector.SetY(-1);
	}
	if (IsKeyDown(SDLK_w))
	{
		vector.SetY(1);
	}

	static float dashTimer = 0;
	static CVector dashV = CVector(0,0);

	// Shift for dash movement
	if (IsKeyDown(SDLK_LSHIFT) && dashTimer <= 0 && status == JUMP && slideStatus == SLIDE_OFF 
		&& dashStatus == DASH_READY && (vector.GetY() != 0 || vector.GetX() != 0))
	{
		status = DASH;
		dashStatus = DASH_OFF;
		dashTimer = 0.2;
		dashV = vector;
	}
	if (dashTimer > 0 && slideStatus == SLIDE_OFF)
	{
		player.SetDirection(Normalise(dashV));
		player.SetSpeed(1000);
		align == LEFT ? DashParticle("JumpDownLeft") : DashParticle("JumpDownRight");
	}
	else // Different movement if you climbing or not
	{
		if (dashStatus == DASH_OFF)
		{
			if(vector.GetY() == 1 || vector.GetY() == 0)
			player.SetVelocity(0, 0);
			dashStatus = DASH_OVER;
		}

		if (slideStatus == LEFT_SLIDE || slideStatus == RIGHT_SLIDE)
		{
			player.SetYVelocity(vector.GetY() * 150);
			player.SetXVelocity(vector.GetX() * 320);
			player.Accelerate(0, 0);
		}
		else
		{
			player.Accelerate(0, -20);
			player.SetXVelocity(vector.GetX() * 320);
		}

	}

	// Tons of SetAnimation, for me this solutin is kind of compact, usually more messy

	if (vector.GetX() == -1 && (slideStatus == SLIDE_OFF || slideStatus == SLIDE_INACTIVE)) Animation("RunLeft", 12);
	if (vector.GetX() == 1 && (slideStatus == SLIDE_OFF || slideStatus == SLIDE_INACTIVE)) Animation("RunRight", 12);

	if (player.GetXVelocity() == 0 && status != JUMP)
	{
		if (align == LEFT) Animation("IdleLeft", 4);
		else Animation("IdleRight", 4);
	}

	if (slideStatus == RIGHT_SLIDE && player.GetYVelocity() == 0) Animation("ClimbIdleRight", 4);
	if (slideStatus == LEFT_SLIDE && player.GetYVelocity() == 0) Animation("ClimbIdleLeft", 4);
	if (slideStatus == RIGHT_SLIDE && player.GetYVelocity() > 0) Animation("ClimbUpRight", 12);
	if (slideStatus == LEFT_SLIDE && player.GetYVelocity() > 0) Animation("ClimbUpLeft", 12);
	if (slideStatus == RIGHT_SLIDE && player.GetYVelocity() < 0) Animation("ClimbDownRight", 12);
	if (slideStatus == LEFT_SLIDE && player.GetYVelocity() < 0) Animation("ClimbDownLeft", 12);

	if (status == JUMP && (slideStatus == SLIDE_OFF || slideStatus == SLIDE_INACTIVE))
	{
		if (player.GetYVelocity() > 0) align == LEFT ? Animation("JumpUpLeft", 1) : Animation("JumpUpRight", 1);
		if (player.GetYVelocity() < 0) align == LEFT ? Animation("JumpDownLeft", 1) : Animation("JumpDownRight", 1);
	}

	dashTimer -= (float)GetDeltaTime() / 1000;
}

// Tile Brush if you editing map
void CMyGame::TileBrush()
{
	float tileBool = false;

	//If click select a tile from the tilemap with Right mouse button, the tile appear in your mouse position
	for (int i = 0; i < 100; i++)
	{
		tileMap[i]->Update(GetTime());

		if (tileMap[i]->HitTest(GetMouseCoords()) && IsRButtonDown())
		{
			if (tileMap[i]->GetState() != tileBrush.GetState())
			{
				CSprite* tile = tileMap[i]->Clone();
				tileMap[i]->SetSize(size, size);
				tileBrush = *tile;
			}
			tileBool = true;
		}
	}

	// if click the right button and no bool selected the tile on your mouse position disappear (deleted)
	if (IsRButtonDown() && !tileBool)
	{
		tileBrush.SetState(0);
		tileBrush.Delete();
	}

	// tile position is snapping
	tileBrush.SetPosition((int)GetMouseCoords().GetX() / size * size + 20, (int)GetMouseCoords().GetY() / size * size + 20);

	int x = tileBrush.GetX() / size;
	int y = tileBrush.GetY() / size;

	// if you have any brush in your mouse position and click with left mouse button, you can paint your map
	if (IsLButtonDown() && tileBrush.GetState() != tileArray[x][y]->GetState())
	{
		CSprite* tile = tileBrush.Clone();
		tileArray[x][y] = tile;
		cout << tile->GetState() << endl;
	}
}

// TileMap generator
void CMyGame::LoadTileGenerator()
{
	for (int i = 0; i < 100; i++)
	{
		tileMap[i] = new CSprite;
	}

	ifstream data;
	string ID = "";
	char* tag = "";
	// Reading the tile tag-s from file
	data.open("tileTag.txt");

	int counter = 1;

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			data >> ID;
			tag = const_cast<char*>(ID.c_str());

			tileMap[counter] = new CSprite;
			tileMap[counter]->LoadImageW("tileMap.bmp", "tile", CSprite::Sheet(4, 10).Tile(j, i), CColor::Black());
			tileMap[counter]->SetImage("tile");
			tileMap[counter]->SetSize(size, size);
			// Using tags to group certain tiles together
			tileMap[counter]->SetProperty("tag", tag);
			tileMap[counter]->SetPosition(100 + j * size, 100 + i * size);
			// Every tile has different state, so I can give different behavior to them if wanted
			tileMap[counter]->SetState(counter);

			// Animated tiles
			if (counter == 25)
			{
				tileMap[counter]->LoadAnimation("fragmentGreen.bmp", "fragG", CSprite::Sheet(12, 1).Row(0).From(0).To(11), CColor::Black());
				tileMap[counter]->SetAnimation("fragG", 12);
			}

			if (counter == 27)
			{
				tileMap[counter]->LoadAnimation("NewSkillOrb.bmp", "skillOrb", CSprite::Sheet(12, 1).Row(0).From(0).To(11), CColor::Black());
				tileMap[counter]->SetAnimation("skillOrb", 8);
			}

			counter++;
		}
	}
	data.close();
}

// Generating map from files
void CMyGame::MapGenerator(string map)
{
	ifstream data;
	int ID = 0;
	data.open(map);
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 18; j++)
		{
			data >> ID;
			delete tileArray[i][j];
			tileArray[i][j] = new CSprite;
			if (ID > 0)
			{
				tileArray[i][j] = tileMap[ID]->Clone();
				tileArray[i][j]->SetPosition(i * size + 20, j * size + 20);
				tileArray[i][j]->SetState(ID);
			}
			else if (ID < 0 && mapArray[mapX][mapY].newGame) // negative state if you pick an item, and when you reset the map (New Game) it generate them with the positive one
			{
				ID = abs(ID);

				tileArray[i][j] = tileMap[ID]->Clone();
				tileArray[i][j]->SetPosition(i * size + 20, j * size + 20);
				tileArray[i][j]->SetState(ID);
			}
		}
	}
	data.close();

	static char* tempName;

	//Setting the background if different
	if (mapArray[mapX][mapY].backGround != tempName)
	backGround.SetImage(mapArray[mapX][mapY].backGround);

	tempName = mapArray[mapX][mapY].backGround;
}

// Saving the map (every map has different txt file
void CMyGame::SaveMap(string map)
{
	ofstream saveFile(map);
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 18; j++)
		{
			if (j == 17)
			{
				saveFile << tileArray[i][j]->GetState() << " " << endl;
			}
			else saveFile << tileArray[i][j]->GetState() << " ";
		}
	}
	saveFile.close();
}

// Load the new map if you left the current map
void CMyGame::JumpToMap()
{
	if (player.GetX() < 0)
	{
		mapX--;
		player.SetX(GetWidth());
		MapGenerator(mapArray[mapX][mapY].mapName);
		textBool = false;
	}
	if (player.GetX() > GetWidth())
	{
		mapX++;
		player.SetX(0);
		MapGenerator(mapArray[mapX][mapY].mapName);
		textBool = false;
	}
	if (player.GetY() < 0)
	{
		mapY--;
		player.SetY(GetHeight());
		MapGenerator(mapArray[mapX][mapY].mapName);
		textBool = false;
	}
	if (player.GetY() > GetHeight())
	{
		mapY++;
		player.SetY(0);
		MapGenerator(mapArray[mapX][mapY].mapName);
		textBool = false;
	}
}

// Particle effect for the green fragment item
void CMyGame::PickUpFragment(CColor color, CVector position)
{
	for (int i = 0; i < 20; i++)
	{
		CSprite* particle = new CSprite;
		particleList.push_back(particle);
		particle->LoadImageW("particle.bmp", CColor::Black());
		particle->SetImage("particle.bmp");
		particle->GetGraphics()->SetPixel(0, 0, color);
		particle->SetSize(12, 12);
		particle->SetPos(position);
		particle->SetDirection(rand() % 360);
		particle->SetState(1);
	}
}

// Particle effect for the skill orb item
void CMyGame::PickUpSkillOrb(CColor color, CVector position)
{
	for (int i = 0; i < 40; i++)
	{
		CSprite* particle = new CSprite;
		particleList.push_back(particle);
		particle->LoadImageW("particle.bmp", CColor::Black());
		particle->SetImage("particle.bmp");
		particle->GetGraphics()->SetPixel(0, 0, color);
		particle->SetSize(8, 8);
		particle->SetState(3);
		particle->SetPos(position.GetX() + rand() % 400 - 200, position.GetY() + rand() % 400 - 200);
	}
}

// Particle effect for the Dust (background particle)
void CMyGame::DustParticle()
{
	if (rand() % 20 == 1)
	{
		CSprite* dust;
		particleList.push_back(dust = new CSprite);
		dust->LoadImageW("particle.bmp", CColor::Black());
		dust->SetImage("particle.bmp");
		dust->GetGraphics()->SetPixel(0, 0, CColor(135, 135, 135));
		dust->SetDirection(70 + rand() % 20);
		dust->SetSize(6, 6);
		dust->SetPosition(rand() % GetWidth(), rand() % GetHeight());
		dust->SetState(2);
	}
}

// Paricle effect for the Snow (Last prototype map)
void CMyGame::SnowParticle()
{
	if (rand() % 8 == 1)
	{
		int randomN = (3 + rand() % 4);
		CSprite* snow;
		particleList.push_back(snow = new CSprite);
		snow->LoadImageW("particle.bmp", CColor::Black());
		snow->SetImage("particle.bmp");
		snow->GetGraphics()->SetPixel(0, 0, CColor(240, 240, 240));
		snow->SetDirection(160 + rand() % 40);
		snow->SetSize(randomN, randomN);
		snow->SetPosition(rand() % GetWidth(), GetHeight());
		snow->SetState(5);
		snow->SetHealth(60);
	}
}

// Player Particle (the red line, which is always on)
void CMyGame::PlayerParticle()
{
	CSprite* playerP;
	particleList.push_back(playerP = new CSprite);
	playerP->LoadImageW("particle.bmp", CColor::Black());
	playerP->SetImage("particle.bmp");
	playerP->GetGraphics()->SetPixel(0, 0, CColor(143, 26, 26));
	playerP->SetSize(3, 3);
	playerP->SetPosition(player.GetX(), player.GetY() + size / 2.5);
	playerP->SetState(6);
}

// Dash Particle effect
void CMyGame::DashParticle(char* sprite)
{
	static int skipFrame = 0;

	if (skipFrame == 3) skipFrame = 0;

	if (skipFrame == 0)
	{
		CSprite* dashP;
		particleList.push_back(dashP = new CSprite);
		dashP->AddImage("player.bmp", "JumpDownLeft", 10, 6, 5, 0, 5, 0, CColor::Black());
		dashP->AddImage("player.bmp", "JumpDownRight", 10, 6, 7, 0, 7, 0, CColor::Black());
		dashP->SetAnimation(sprite);
		dashP->SetSize(3, 3);
		dashP->SetPosition(player.GetX(), player.GetY());
		dashP->SetHealth(1);
		dashP->SetState(7);
	}

	skipFrame++;
}

// Player Death Particle effect
void CMyGame::PlayerDeathParticle()
{
	for (int i = 0; i < 20; i++)
	{
		CSprite* particle = new CSprite;
		particleList.push_back(particle);
		particle->LoadImageW("particle.bmp", CColor::Black());
		particle->SetImage("particle.bmp");
		particle->GetGraphics()->SetPixel(0, 0, CColor(93, 13, 13));
		particle->SetSize(10, 10);
		particle->SetPosition(player.GetX() + rand() % (int)player.GetWidth() - player.GetWidth() / 2, player.GetY() + rand() % (int)player.GetHeight() - player.GetHeight() / 2);
		particle->SetDirection(rand() % 360);
		//particle->SetVelocity((float)(100 - rand() % 200) / 100, (float)(100 - rand() % 200) / 100);
		//particle->SetVelocity(particle->GetVelocity() * 200);
		particle->SetState(8);
	}
}

// And the Particle System Update for every Particle effect
void CMyGame::ParticleSystem()
{
	if (mapArray[mapX][mapY].lvlNum <= 11) 	DustParticle();
	else if (mapArray[mapX][mapY].lvlNum > 11) SnowParticle();

	PlayerParticle();

	for (CSprite* particles : particleList)
	{
		particles->Update(GetTime());

		// Pick up Fragment Particle
		if (particles->GetState() == 1)
		{
			particles->SetSpeed(250);
			particles->SetSize(particles -> GetWidth() - 0.7, particles ->GetHeight() - 0.7);
			particles->SetDirection(particles->GetDirection() + 10);

			if (particles->GetWidth() <= 0) particles->Delete();
		}

		// Dust Particle
		if (particles->GetState() == 2)
		{
			particles->SetSpeed(20);
			particles->SetSize(particles->GetWidth() - 0.2, particles->GetHeight() - 0.2);
			if (particles->GetWidth() <= 0) particles->Delete();
		}

		// Skill Orb Particle
		if (particles->GetState() == 3)
		{
			float rotateTo = LookAt(player.GetX(), player.GetY(), particles->GetX(), particles->GetY());
			particles->SetSpeed(particles->GetSpeed() + 30);
			particles->SetDirection(rotateTo + 90);

			if (particles->HitTest(&player)) particles->Delete();
		}

		// Falling Sand Particle
		if (particles->GetState() == 4)
		{
			particles->SetSpeed(400);

			particles->SetSize(particles->GetWidth() - 0.3, particles->GetHeight() - 0.3);
			if (particles->GetWidth() <= 0) particles->Delete();
		}

		// Snow Particle
		if (particles->GetState() == 5)
		{
			particles->SetSpeed(100);

			particles->SetHealth(particles->GetHealth() - 0.1);
			if (particles->GetHealth() <= 0) particles->Delete();
		}

		// Player Particle
		if (particles->GetState() == 6)
		{
			particles->SetSize(particles->GetWidth() - 0.1, particles->GetHeight() - 0.1);
			if (particles->GetWidth() <= 0) particles->Delete();
		}

		// Dash Particle
		if (particles->GetState() == 7)
		{
			particles->SetSize(size, size);

			particles->SetHealth(particles->GetHealth() - 0.1);
			if (particles->GetHealth() <= 0) particles->Delete();
		}

		//Player Death Particle
		if (particles->GetState() == 8)
		{
			//particles->SetYVelocity(particles->GetYVelocity() - 20);
			particles->SetSpeed(400);

			particles->SetSize(particles->GetWidth() - 0.6, particles->GetHeight() - 0.6);
			if (particles->GetWidth() <= 0) particles->Delete();
		}
	}
	particleList.delete_if(deleted);
}

// Delete sand platform if you dash on it, and check every neighbor sand platform and call itself again until destroyed all of them on the same row (risky I know)
void CMyGame::DeletePlatform(CSprite& platform)
{
	platform.SetState(-platform.GetState());
	platform.SetProperty("tag", "NULL");

	int x = (platform.GetX() - 20) / 40;
	int y = (platform.GetY() - 20) / 40;

	SaveMap(mapArray[mapX][mapY].mapName);
	mapArray[mapX][mapY].newGame = false;

	for (int i = 0; i < 20; i++)
	{
		CSprite* fallingRock;
		particleList.push_back(fallingRock = new CSprite);
		fallingRock->LoadImageW("particle.bmp", CColor::Black());
		fallingRock->SetImage("particle.bmp");
		fallingRock->GetGraphics()->SetPixel(0, 0, CColor(130, 96, 36));
		fallingRock->SetDirection(180);
		fallingRock->SetSize(6, 6);
		fallingRock->SetPosition(platform.GetX() + rand() % (int)platform.GetWidth() - platform.GetWidth() / 2, platform.GetY() + rand() % (int)platform.GetHeight() - platform.GetHeight());
		fallingRock->SetState(4);
	}
	for (int i = 0; i < 5; i++)
	{
		CSprite* fallingRock;
		particleList.push_back(fallingRock = new CSprite);
		fallingRock->LoadImageW("particle.bmp", CColor::Black());
		fallingRock->SetImage("particle.bmp");
		fallingRock->GetGraphics()->SetPixel(0, 0, CColor(130, 96, 36));
		fallingRock->SetDirection(60 - rand() % 120);
		fallingRock->SetSize(6, 6);
		fallingRock->SetPosition(player.GetX() + rand() % (int)platform.GetWidth() - platform.GetWidth() / 2, player.GetY() + rand() % (int)platform.GetHeight() - platform.GetHeight());
		fallingRock->SetState(4);
	}

	if (tileArray[x + 1][y]->GetState() == 28)
	{
		DeletePlatform(*tileArray[x + 1][y]);
	}

	if (tileArray[x - 1][y]->GetState() == 28)
	{
		DeletePlatform(*tileArray[x - 1][y]);
	}
}

// 2D Distance calculation
float CMyGame::Distance(float targetX, float targetY, float sourceX, float sourceY)
{
	float distance = sqrt(pow(targetX - sourceX, 2) + pow(targetY - sourceY, 2));

	return distance;
}

// LookAt function (naming after the Unity LookAt function)
float CMyGame::LookAt(float targetX, float targetY, float sourceX, float sourceY)
{
	float X = targetX - sourceX;
	float Y = targetY - sourceY;

	float angle = atan2(X, Y) * 180.0f / M_PI;

	return angle - 90;
}