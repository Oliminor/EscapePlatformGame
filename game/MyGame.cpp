#include "stdafx.h"
#include "UI.h"
#include "MyGame.h"
#include <math.h>

UI ui;

CMyGame::CMyGame(void){}

CMyGame::~CMyGame(void){}

void CMyGame::OnUpdate()
{
	Uint32 t = GetTime();
	delta = GetDeltaTime();
	ui.myGame = this;

	CharacterController();

	ui.OnUpdate(t, delta);
	player.Update(t);
	player.SetSize(size, size);
	tileBrush.Update(t);
	backGround.Update(t);

	for (CSprite* arrows : arrowList)
	{
		arrows->Update(t);
		arrows->Accelerate(0, -10);
		arrows->SetRotation(arrows->GetDirection());
		ArrowParticle(arrows->GetPosition());

		if (arrows->GetX() < 0 || arrows->GetX() > GetWidth() || arrows->GetY() < 0 || arrows->GetY() > GetHeight()) arrows->Delete();
	}

	for (CSprite* projectiles : turretProjectileList)
	{
		projectiles->Update(t);
		TurretProjectileParticle(projectiles->GetPosition());
		if (projectiles->HitTest(&player))
		{
			PlayerDeathParticle();
			projectiles->Delete();
			TurretProjectileDieParticle(projectiles->GetPosition());
		}
		if (projectiles->GetX() < 0 || projectiles->GetX() > GetWidth() || projectiles->GetY() < 0 || projectiles->GetY() > GetHeight()) projectiles->Delete();
	}

	TileBrush();
	ParticleSystem();
	BowSystem();

	CVector n = Normalise(player.GetVelocity());
	bool slideBool = false;
	bool jumpBool = false;
	bool nextMap = false;

	static float freq = 0;
	static int mapTemp = 0;

	if (mapNumber != mapTemp) freq = 0;
	mapTemp = mapNumber;

	freq += 0.03;

	player.SetXVelocity(0);
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 18; j++)
		{
			tileArray[i][j]->Update(t);

			// Platform and player reaction
			if ((string)tileArray[i][j]->GetProperty("tag") == "platform" ||
				(string)tileArray[i][j]->GetProperty("tag") == "movingPlatform" ||
				(string)tileArray[i][j]->GetProperty("tag") == "showPlatform" ||
				(string)tileArray[i][j]->GetProperty("tag") == "turret")
			{

				for (CSprite* arrows : arrowList)
				{
					if (tileArray[i][j]->HitTest(arrows->GetX(), arrows->GetY()))
					{
						arrows->Delete();
						ArrowDieParticle(arrows->GetPosition());
					}
				}

				// If player touch the left side you can climb on it (if the skill is unlocked) also reset the jumps
				if (tileArray[i][j]->HitTest(player.GetLeft() + 6, player.GetY())  && tileArray[i][j]->GetX() < player.GetX() - n.m_x)
				{
					player.SetPosition(tileArray[i][j]->GetX() + tileArray[i][j]->GetHeight() / 2 + player.GetHeight() / 2 - 8, player.GetY());
					player.SetXVelocity(0);
					if (IsKeyDown(SDLK_a))
					{
						if (slideStatus != SLIDE_INACTIVE)
						{
							player.SetYVelocity(tileArray[i][j]->GetYVelocity());
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
					player.SetPosition(tileArray[i][j]->GetX() - tileArray[i][j]->GetHeight() / 2 - player.GetHeight() / 2 + 5, player.GetY());
					player.SetXVelocity(0);
					if (IsKeyDown(SDLK_d))
					{
						if (slideStatus != SLIDE_INACTIVE)
						{
							player.SetYVelocity(tileArray[i][j]->GetYVelocity());
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
					player.SetXVelocity(tileArray[i][j]->GetXVelocity());
					if ((string)tileArray[i][j]->GetProperty("tag") == "movingPlatform") player.SetYVelocity(tileArray[i][j]->GetYVelocity());
					jumpN = 2;
					status = RUN;
					jumpBool = true;
				}

				// When the player hit the tiles with the top position, the velocity set 0 and start falling again
				if (tileArray[i][j]->HitTest(player.GetX(), player.GetTop()) && tileArray[i][j]->GetY() > player.GetY() - n.m_y)
				{
					player.SetPosition(player.GetX(), tileArray[i][j]->GetY() - tileArray[i][j]->GetHeight() / 2 - player.GetHeight() / 1.7);
					player.SetYVelocity(0);
				}

				BouncyEnemy(*tileArray[i][j]);
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

			PickUpFragment(*tileArray[i][j]);

			PickUpBow(*tileArray[i][j]);

			CheckPoint(*tileArray[i][j]);

			PickUpSkillOrb(*tileArray[i][j]);

			ExtraJump(*tileArray[i][j]);

			HidePlatform(*tileArray[i][j]);

			SpikeTile(*tileArray[i][j]);

			DeleteTile(*tileArray[i][j]);

			SpiritTarget(*tileArray[i][j]);

			MovingPlatform(*tileArray[i][j]);

			TurretPlatform(*tileArray[i][j]);

			RemoveEnemy(*tileArray[i][j]);

			CircleEnemy(*tileArray[i][j], freq);

			if ((string)tileArray[i][j]->GetProperty("tag") == "nextMap" && tileArray[i][j]->HitTest(&player, 0))
			{
				nextMap = true;
			}
		}
	}
	if (!slideBool && slideStatus != SLIDE_INACTIVE) slideStatus = SLIDE_OFF;
	if (!jumpBool) status = JUMP;

	JumpToMap(nextMap);

	arrowList.delete_if(deleted);
	turretProjectileList.delete_if(deleted);

	if (ui.UIMode == UI::PAUSE_MENU)
	{
		sfx.Volume(0);
		sfx2.Volume(0);
		sfx3.Volume(0);
	}
}

void CMyGame::OnDraw(CGraphics* g)
{
	backGround.Draw(g);

	// Map drawing
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 18; j++)
		{
			if (!editorMode)
			{
				if (tileArray[i][j]->GetState() > 0
					&& tileArray[i][j]->GetState() != 26
					&& tileArray[i][j]->GetState() != 41
					&& tileArray[i][j]->GetState() != 44)
				{
					tileArray[i][j]->Draw(g);
				}
			}
			if (editorMode) tileArray[i][j]->Draw(g);
		}
	}

	// Editor tiles drawing
	if (showTiles) for (int i = 0; i < 200; i++) tileMap[i]->Draw(g);

	// Particle drwaing
	for (CSprite* projectiles : turretProjectileList) projectiles->Draw(g);
	for (CSprite* particles : particleList) particles->Draw(g);

	tileBrush.Draw(g);

	player.Draw(g);
	if(bowStatus == BOW_ON) bow.Draw(g);

	for (CSprite* arrows : arrowList) arrows->Draw(g);

	ui.OnDraw(g);
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
	backGround.LoadImageW("night.bmp", CColor::Black());
	backGround.LoadImageW("castle.bmp", CColor::Black());
	backGround.SetSize(GetWidth(), GetHeight());
	backGround.SetPosition(GetWidth() / 2, GetHeight() / 2);

	// Setting the different map background, txt to load the map and tutorial text (might be change on the big project)
	mapArray[0][0] = { CVector(GetWidth() / 2, 100), 0, "menuMap.txt", "prisonBG.bmp", "menuMusic.ogg" };
	mapArray[5][5] = {CVector(200, 100), 1, "MapData01.txt", "prisonBG.bmp", "prison.ogg" };
	mapArray[5][6] = { CVector(250, 200), 2, "MapData02.txt", "prisonBG.bmp", "prison.ogg" };
	mapArray[6][6] = { CVector(200, 600), 3, "MapData03.txt", "prisonBG2.bmp" , "prison.ogg" };
	mapArray[6][5] = { CVector(400, 200), 4, "MapData04.txt", "prisonBG.bmp" , "prison.ogg" };
	mapArray[6][4] = { CVector(100, 200), 5, "MapData05.txt", "prisonBG.bmp", "prison.ogg" };
	mapArray[7][6] = { CVector(400, 600), 6, "MapData06.txt", "prisonBG.bmp", "prison.ogg" };
	mapArray[7][7] = { CVector(100, 200), 7, "MapData07.txt", "prisonBG.bmp" , "prison.ogg" };
	mapArray[8][7] = { CVector(100, 200), 8, "MapData08.txt", "prisonBG2.bmp", "prison.ogg" };
	mapArray[8][8] = { CVector(100, 200), 9, "MapData09.txt", "prisonBG.bmp" , "prison.ogg" };
	mapArray[9][8] = { CVector(100, 100), 10, "MapData10.txt", "prisonBG3.bmp" , "prison.ogg" };
	mapArray[10][8] = { CVector(100, 200), 11, "MapData11.txt", "prisonBG4.bmp", "prison.ogg" };
	mapArray[10][9] = { CVector(GetWidth() / 2, 100), 12, "MapData12.txt", "moon.bmp", "outside.ogg" };
	mapArray[9][9] = { CVector(GetWidth() / 2, 100), 13, "MapData13.txt", "night.bmp", "outside.ogg" };
	mapArray[8][9] = { CVector(100, 100), 14, "MapData14.txt", "castle.bmp" , "castle.ogg" };
	mapArray[7][9] = { CVector(GetWidth()-100, 100), 15, "MapData15.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[7][10] = { CVector(250, 100), 16, "MapData16.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[8][10] = { CVector(100, 100), 17, "MapData17.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[9][10] = { CVector(100, 100), 18, "MapData18.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[9][11] = { CVector(400, 100), 19, "MapData19.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[9][12] = { CVector(700, 200), 20, "MapData20.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[9][13] = { CVector(700, 200), 21, "MapData21.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[9][14] = { CVector(600, 100), 22, "MapData22.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[10][14] = { CVector(600, 400), 23, "MapData23.txt", "night.bmp"  , "outside.ogg" };
	mapArray[7][8] = { CVector(100, 100), 24, "MapData24.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[6][8] = { CVector(100, 100), 25, "MapData25.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[5][8] = { CVector(GetWidth() - 50, 680), 26, "MapData26.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[4][8] = { CVector(GetWidth() - 100, 700), 27, "MapData27.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[4][7] = { CVector(GetWidth() - 100, 700), 28, "MapData28.txt", "castle.bmp"  , "castle.ogg" };
	mapArray[3][7] = { CVector(GetWidth() - 100, 100), 29, "MapData29.txt", "night.bmp", "outside.ogg" };

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

	bow.LoadImageW("spiritBow.bmp", CColor::Black());
	bow.SetImage("spiritBow.bmp");
	bow.SetPosition(player.GetPosition());
	bow.SetPivotRel(0, 0.5);

	//New game function
	NewGame();

	mapX = 0;
	mapY = 0;
	MapGenerator(mapArray[0][0].mapName);
	mapNumber = 0;
}


void CMyGame::OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode)
{
	ui.OnKeyDown(sym);

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

	// F3 editor mode on/off

	if (sym == SDLK_F3)
	{
		if (editorMode)
		{
			editorMode = false;
			showTiles = false;
		}
		else
		{
			editorMode = true;
			showTiles = true;
		}
	}

	// Tilemap on / off in editor mode
	if (sym == SDLK_e && editorMode)
	{
		if (showTiles) showTiles = false;
		else showTiles = true;
	}

	// Space jump
	if (sym == SDLK_SPACE && jumpN > 0 && (slideStatus == SLIDE_OFF || slideStatus == SLIDE_INACTIVE))
	{
		sfx.Play("jump.wav", 0);
		sfx.Volume(0.6);
		if (jumpStatus == DOUBLEJUMP_ACTIVE) jumpN--;
		else jumpN -= 2;
		player.SetYVelocity(450);
		status = JUMP;
	}

	//For Edit the maps

	if (editorMode)
	{ 
		if (sym == SDLK_LEFT)
		{
			if (mapArray[mapX - 1][mapY].lvlNum != 0)
			{
				mapX--;
				MapGenerator(mapArray[mapX][mapY].mapName);
				player.SetPos(mapArray[mapX][mapY].defaultPos);
			}
		}
		if (sym == SDLK_RIGHT)
		{
			if (mapArray[mapX + 1][mapY].lvlNum != 0)
			{
				mapX++;
				MapGenerator(mapArray[mapX][mapY].mapName);
				player.SetPos(mapArray[mapX][mapY].defaultPos);
			}
		}
		if (sym == SDLK_DOWN)
		{
			if (mapArray[mapX][mapY - 1].lvlNum != 0)
			{
				mapY--;
				MapGenerator(mapArray[mapX][mapY].mapName);
				player.SetPos(mapArray[mapX][mapY].defaultPos);
			}
		}
		if (sym == SDLK_UP)
		{
			if (mapArray[mapX][mapY + 1].lvlNum != 0)
			{
				mapY++;
				MapGenerator(mapArray[mapX][mapY].mapName);
				player.SetPos(mapArray[mapX][mapY].defaultPos);
			}
		}
	}
}

void CMyGame::OnLButtonDown(Uint16 x,Uint16 y)
{
	ui.OnLButtonDown(x, y);

	if (bowStatus == BOW_ON && arrowList.size() < 3)
	{
		sfx2.Play("arrow.wav", 0);
		sfx2.Volume(0.6);

		CSprite* arrow = new CSprite(bow.GetX(), bow.GetY(), "arrow.bmp", CColor::Black(), GetTime());
		arrowList.push_back(arrow);
		arrow->SetRotation(bow.GetRotation() + 90);
		arrow->SetDirection(arrow->GetRotation());
		arrow->SetSpeed(1000);
		arrow->SetSize(size / 1.5, size / 1.5);		
	}
}

// New Game function to reset the game and start over
void CMyGame::NewGame()
{
	mapNumber = 1;
	ui.myGame = this;
	ui.OnInitialize();

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
	editorMode = false;
	godMode = false;

	bowStatus = BOW_INACTIVE;
	dashStatus = DASH_INACTIVE;
	slideStatus = SLIDE_INACTIVE;
	jumpStatus = DOUBLEJUMP_INACTIVE;
	align = RIGHT;

	LoadTileGenerator();
	MapGenerator(mapArray[mapX][mapY].mapName);
	player.SetPosition(mapArray[mapX][mapY].defaultPos);
}

// Bow system
void CMyGame::BowSystem()
{
	if (bowStatus != BOW_INACTIVE)
	{
		if (IsRButtonDown())
		{
			bowStatus = BOW_ON;
			bow.Update(GetTime());

			bow.SetSize(size, size);
			bow.SetPosition(player.GetPosition());
			float lookAt = LookAt(GetMouseCoords().GetX(), GetMouseCoords().GetY(), bow.GetX(), bow.GetY());
			bow.SetRotation(lookAt);
		}
		else bowStatus = BOW_OFF;
	}
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
		dashStatus = DASH_OFF;
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
			player.SetYVelocity(vector.GetY() * 150 + player.GetYVelocity());
			player.SetXVelocity(vector.GetX() * 320);
			player.Accelerate(0, 0);
		}
		else
		{
			player.Accelerate(0, -20);
			if (player.GetYVelocity() < -1000) player.SetYVelocity(-1000);
			player.SetXVelocity((vector.GetX() * 320) + player.GetXVelocity());
		}

	}

	// Tons of SetAnimation, for me this solutin is kind of compact, usually more messy

	if (vector.GetX() == -1 && (slideStatus == SLIDE_OFF || slideStatus == SLIDE_INACTIVE)) Animation("RunLeft", 12);
	if (vector.GetX() == 1 && (slideStatus == SLIDE_OFF || slideStatus == SLIDE_INACTIVE)) Animation("RunRight", 12);

	if ((player.GetXVelocity() < 120 && player.GetXVelocity() > -120) && status != JUMP)
	{
		if (align == LEFT) Animation("IdleLeft", 4);
		else Animation("IdleRight", 4);
	}

	if (slideStatus == RIGHT_SLIDE && !IsKeyDown(SDLK_w) && !IsKeyDown(SDLK_s)) Animation("ClimbIdleRight", 4);
	if (slideStatus == LEFT_SLIDE && !IsKeyDown(SDLK_w) && !IsKeyDown(SDLK_s)) Animation("ClimbIdleLeft", 4);
	if (slideStatus == RIGHT_SLIDE && IsKeyDown(SDLK_w)) Animation("ClimbUpRight", 12);
	if (slideStatus == LEFT_SLIDE && IsKeyDown(SDLK_w)) Animation("ClimbUpLeft", 12);
	if (slideStatus == RIGHT_SLIDE && IsKeyDown(SDLK_s)) Animation("ClimbDownRight", 12);
	if (slideStatus == LEFT_SLIDE && IsKeyDown(SDLK_s)) Animation("ClimbDownLeft", 12);

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
	for (int i = 0; i < 200; i++)
	{
		tileMap[i]->Update(GetTime());

		CVector v = CVector(0,0);
		if (IsKeyDown(SDLK_i)) v.SetY(1);
		if (IsKeyDown(SDLK_k)) v.SetY(-1);
		if (IsKeyDown(SDLK_l)) v.SetX(1);
		if (IsKeyDown(SDLK_j)) v.SetX(-1);

		tileMap[i]->SetVelocity(v.GetX() * 200, v.GetY() * 200);

		if (tileMap[i]->HitTest(GetMouseCoords()) && IsRButtonDown() && editorMode && showTiles)
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
	if (IsLButtonDown() && tileBrush.GetState() != tileArray[x][y]->GetState() && editorMode)
	{
		CSprite* tile = tileBrush.Clone();
		tileArray[x][y] = tile;
		cout << tile->GetState() << endl;
	}
}

// TileMap generator
void CMyGame::LoadTileGenerator()
{
	for (int i = 0; i < 200; i++)
	{
		tileMap[i] = new CSprite;
	}

	ifstream data;
	string ID = "";
	char* tag = "";
	// Reading the tile tag-s from file
	data.open("tileTag.txt");

	int counter = 1;

	int column = 4;
	int row = 27;

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			data >> ID;
			tag = const_cast<char*>(ID.c_str());

			tileMap[counter] = new CSprite;
			tileMap[counter]->LoadImageW("tileMap.bmp", "tile", CSprite::Sheet(column, row).Tile(j, i), CColor::Black());
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

			if (counter == 45)
			{
				tileMap[counter]->LoadAnimation("extraJump.bmp", "extraJump", CSprite::Sheet(8, 1).Row(0).From(0).To(7), CColor::Black());
				tileMap[counter]->SetAnimation("extraJump", 8);
			}

			if (counter == 46)
			{
				tileMap[counter]->LoadImageW("tileMap.bmp", "showTile", CSprite::Sheet(column, row).Tile(j, i), CColor::Black());
				tileMap[counter]->LoadImageW("tileMap.bmp", "hideTile", CSprite::Sheet(column, row).Tile(j + 1, i), CColor::Black());
				tileMap[counter]->SetHealth(2);
			}

			if (counter == 47)
			{
				tileMap[counter]->LoadImageW("tileMap.bmp", "hideTile", CSprite::Sheet(column, row).Tile(j, i), CColor::Black());
				tileMap[counter]->LoadImageW("tileMap.bmp", "showTile", CSprite::Sheet(column, row).Tile(j - 1, i), CColor::Black());
				tileMap[counter]->SetHealth(2);
			}

			if (counter == 97)
			{
				tileMap[counter]->LoadAnimation("spiritGate.bmp", "spiritGate", CSprite::Sheet(9, 1).Row(0).From(0).To(8), CColor::Black());
				tileMap[counter]->SetAnimation("spiritGate", 20);
			}

			if (counter == 98)
			{
				tileMap[counter]->LoadAnimation("spiritTarget.bmp", "spiritTarget", CSprite::Sheet(8, 1).Row(0).From(0).To(7), CColor::Black());
				tileMap[counter]->SetAnimation("spiritTarget", 8);
				tileMap[counter]->SetOmega(250);
			}

			if (counter == 99) 	tileMap[counter]->SetOmega(400);
			if (counter == 100) tileMap[counter]->SetOmega(400);

			if (counter == 102) tileMap[counter]->SetRotation(90);
			if (counter == 103) tileMap[counter]->SetRotation(180);
			if (counter == 104) tileMap[counter]->SetRotation(270);

			counter++;
		}
	}
	data.close();
}

// Generating map from files
void CMyGame::MapGenerator(string map)
{
	turretProjectileList.delete_all();

	ifstream data;
	int ID = 0;
	data.open(map);
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 18; j++)
		{
			data >> ID;
			delete tileArray[i][j];
			tileArray[i][j] = new CSprite();
			if (ID > 0)
			{
				tileArray[i][j] = tileMap[ID]->Clone();
				tileArray[i][j]->SetPosition(i * size + 20, j * size + 20);
				tileArray[i][j]->SetState(ID);

				if (ID == 42) tileArray[i][j]->SetYVelocity(100);
				if (ID == 43) tileArray[i][j]->SetXVelocity(100);
				if (ID == 99) tileArray[i][j]->SetYVelocity(200);
				if (ID == 100)tileArray[i][j]->SetXVelocity(200);

				if (ID == 105)
				{
					tileArray[i][j]->SetDirection(rand() % 360);
					tileArray[i][j]->SetSpeed(350);
					cout << tileArray[i][j]->GetDirection() << endl;
				}
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

	mapNumber = mapArray[mapX][mapY].lvlNum;
	respawnPos = mapArray[mapX][mapY].defaultPos;

	static bool firstBool = false;
	if (firstBool)
		ui.PlayDialogue(0);
	firstBool = true;
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
void CMyGame::JumpToMap(bool _bool)
{
	if (player.GetX() < 0 && _bool)
	{
		mapX--;
		player.SetX(GetWidth());
		MapGenerator(mapArray[mapX][mapY].mapName);
	}
	if (player.GetX() > GetWidth() && _bool)
	{
		mapX++;
		player.SetX(0);
		MapGenerator(mapArray[mapX][mapY].mapName);
	}
	if (player.GetY() < 0 && _bool)
	{
		mapY--;
		player.SetY(GetHeight());
		MapGenerator(mapArray[mapX][mapY].mapName);
	}
	if (player.GetY() > GetHeight() && _bool)
	{
		mapY++;
		player.SetY(0);
		MapGenerator(mapArray[mapX][mapY].mapName);
	}
	if (!_bool && (player.GetX() < 0 || player.GetX() > GetWidth() || player.GetY() < 0 || player.GetY() > GetHeight()))
	{
		player.SetPosition(respawnPos);
	}
}

// Particle effect for the green fragment item
void CMyGame::FragmentParticle(CColor color, CVector position)
{
	sfx3.Play("fragment.wav", 0);
	sfx3.Volume(0.6);

	for (int i = 0; i < 20; i++)
	{
		CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
		particleList.push_back(particle);
		particle->GetGraphics()->SetPixel(0, 0, color);
		particle->SetSize(12, 12);
		particle->SetDirection(rand() % 360);
		particle->SetSpeed(100 + rand() % 150);
		particle->SetState(1);
	}
}

// Particle effect for the skill orb item
void CMyGame::SkillOrbParticle(CColor color, CVector position)
{
	sfx3.Play("skillorb.wav", 0);
	sfx3.Volume(0.6);

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

// Particle effect for picking up the bow
void CMyGame::PickUpBowParticle(CVector position)
{
	for (int i = 0; i < 80; i++)
	{
		CSprite* particle = new CSprite;
		particleList.push_back(particle);
		particle->LoadImageW("particle.bmp", CColor::Black());
		particle->SetImage("particle.bmp");
		particle->GetGraphics()->SetPixel(0, 0, CColor(149, 210, 223));
		particle->SetSize(8, 8);
		particle->SetState(3);
		particle->SetPos(position.GetX() + rand() % 700 - 350, position.GetY() + rand() % 700 - 350);
	}
}

// Particle effect for arrow flying 
void CMyGame::ArrowParticle(CVector position)
{
	CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
	particleList.push_back(particle);
	particle->GetGraphics()->SetPixel(0, 0, CColor(149, 210, 223));
	particle->SetVelocity(rand() % 50, rand() % 50);
	particle->SetSize(7, 7);
	particle->SetPosition(position.GetX(), position.GetY());
	particle->SetState(11);
}

// Particle effect for arrow if deleted
void CMyGame::ArrowDieParticle(CVector position)
{
	sfx2.Play("arrowexplode.wav", 0);
	sfx2.Volume(0.6);

	for (int i = 0; i < 10; i++)
	{
		CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
		particleList.push_back(particle);
		particle->GetGraphics()->SetPixel(0, 0, CColor(149, 210, 223));
		particle->SetSize(12, 12);
		particle->SetDirection(rand() % 360);
		particle->SetSpeed(100 + rand() % 150);
		particle->SetState(12);
	}
}

// Particle ffect for Turret Projectile
void CMyGame::TurretProjectileParticle(CVector position)
{
	CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
	particleList.push_back(particle);
	particle->GetGraphics()->SetPixel(0, 0, CColor(150 + rand() % 100, 50, 8));
	particle->SetVelocity(rand() % 75, rand() % 75);
	particle->SetSize(10, 10);
	particle->SetPosition(position.GetX(), position.GetY());
	particle->SetState(11);
}

// Particle effect for turret Projectile if deleted
void CMyGame::TurretProjectileDieParticle(CVector position)
{
	sfx3.Play("fire.wav", 0);
	sfx3.Volume(0.8);

	for (int i = 0; i < 10; i++)
	{
		CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
		particleList.push_back(particle);
		particle->GetGraphics()->SetPixel(0, 0, CColor(150 + rand() % 100, 50, 8));
		particle->SetSize(12, 12);
		particle->SetDirection(rand() % 360);
		particle->SetSpeed(100 + rand() % 150);
		particle->SetState(13);
	}
}

// Particle effect for enemy
void CMyGame::EnemyDeathParticle(CSprite& fragment) 
{
	sfx3.Play("death.wav", 0);
	sfx3.Volume(0.5);

	for (int i = 0; i < 20; i++)
	{
		CVector position = CVector(fragment.GetX() + rand() % (int)fragment.GetWidth() - fragment.GetWidth() / 2, fragment.GetY() + rand() % (int)fragment.GetHeight() - fragment.GetHeight() / 2);
		CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
		particleList.push_back(particle);
		particle->GetGraphics()->SetPixel(0, 0, CColor(36, 113, 31));
		particle->SetSize(12, 12);
		particle->SetDirection(rand() % 360);
		particle->SetVelocity((float)(100 - rand() % 200) / 100, (float)(100 - rand() % 200) / 100);
		particle->SetVelocity(particle->GetVelocity() * 200);
		particle->SetState(8);
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
	if (!godMode)
	{
		ui.deathCounter++;
		if (ui.deathCounter == 10) 	ui.PlayDialogue(5);

		sfx.Play("death.wav", 0);
		sfx.Volume(0.5);

		for (int i = 0; i < 20; i++)
		{
			CVector position = CVector(player.GetX() + rand() % (int)player.GetWidth() - player.GetWidth() / 2, player.GetY() + rand() % (int)player.GetHeight() - player.GetHeight() / 2);
			CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
			particleList.push_back(particle);
			particle->GetGraphics()->SetPixel(0, 0, CColor(93, 13, 13));
			particle->SetSize(10, 10);
			particle->SetDirection(rand() % 360);
			particle->SetVelocity((float)(100 - rand() % 200) / 100, (float)(100 - rand() % 200) / 100);
			particle->SetVelocity(particle->GetVelocity() * 200);
			particle->SetState(8);
		}
		player.SetPosition(respawnPos);
	}
}

// Extra jump crystal Particle effect
void CMyGame::ExtraJumpParticle(int mode, CVector position)
{
	switch (mode)
	{
	case 1:
	{
		sfx2.Play("jumporb.wav", 0);
		sfx2.Volume(0.8);

		for (int i = 0; i < 20; i++)
		{
			CSprite* particle = new CSprite;
			particleList.push_back(particle);
			particle->LoadImageW("particle.bmp", CColor::Black());
			particle->SetImage("particle.bmp");
			particle->GetGraphics()->SetPixel(0, 0, CColor(178, 32, 32));
			particle->SetSize(8, 8);
			particle->SetState(9);
			particle->SetPos(player.GetX() + rand() % 100 - 50, player.GetY() + rand() % 100 - 50);
		}
	}
		break;
	case 2:
	{
		sfx2.Play("orbre.wav", 0);
		sfx2.Volume(0.8);
		for (int i = 0; i < 15; i++)
		{
			CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
			particleList.push_back(particle);
			particle->GetGraphics()->SetPixel(0, 0, CColor(178, 32, 32));
			particle->SetSize(12, 12);
			particle->SetDirection(rand() % 360);
			particle->SetSpeed(100 + rand() % 150);
			particle->SetState(10);
		}
	}
	break;
	}
}

// Spirit Particle effect
void CMyGame::SpiritParticle(int mode, CVector position)
{
	switch (mode)
	{
	case 1:
	{
		for (int i = 0; i < 20; i++)
		{
			CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
			particleList.push_back(particle);
			particle->GetGraphics()->SetPixel(0, 0, CColor(93, 150, 223));
			particle->SetSize(12, 12);
			particle->SetDirection(rand() % 360);
			particle->SetSpeed(100 + rand() % 150);
			particle->SetState(12);
		}
	}
	break;
	case 2:
	{
		for (int i = 0; i < 15; i++)
		{
			CSprite* particle = new CSprite(position.GetX(), position.GetY(), "particle.bmp", CColor::Black(), GetTime());
			particleList.push_back(particle);
			particle->GetGraphics()->SetPixel(0, 0, CColor(93, 150, 223));
			particle->SetSize(10, 10);
			particle->SetDirection(rand() % 360);
			particle->SetSpeed(300);
			particle->SetState(13);
		}
		break;
	}
	}
}

// And the Particle System Update for every Particle effect
void CMyGame::ParticleSystem()
{
	if (mapArray[mapX][mapY].lvlNum > 11 && mapArray[mapX][mapY].lvlNum < 14 || mapArray[mapX][mapY].lvlNum == 23) SnowParticle();
	else DustParticle();

	PlayerParticle();

	for (CSprite* particles : particleList)
	{
		particles->Update(GetTime());

		// Pick up Fragment Particle
		if (particles->GetState() == 1)
		{
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
			particles->SetSpeed(particles->GetSpeed() + 50);
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
			particles->SetYVelocity(particles->GetYVelocity() - 10);
			//particles->SetSpeed(400);

			particles->SetSize(particles->GetWidth() - 0.2, particles->GetHeight() - 0.2);
			if (particles->GetWidth() <= 0) particles->Delete();
		}

		// Extra Jump Particle
		if (particles->GetState() == 9)
		{
			float rotateTo = LookAt(player.GetX(), player.GetY(), particles->GetX(), particles->GetY());
			particles->SetSpeed(particles->GetSpeed() + 100);
			particles->SetDirection(rotateTo + 90);

			if (particles->HitTest(&player)) particles->Delete();
		}

		// Spawn Extra Jump Crystal Particle
		if (particles->GetState() == 10)
		{
			particles->SetSize(particles->GetWidth() - 0.3, particles->GetHeight() - 0.3);
			particles->SetDirection(particles->GetDirection() + 10);

			if (particles->GetWidth() <= 0) particles->Delete();
		}

		// Arrow flying Particle
		if (particles->GetState() == 11)
		{
			particles->SetSize(particles->GetWidth() - 0.3, particles->GetHeight() - 0.3);
			if (particles->GetWidth() <= 0) particles->Delete();
		}

		// Spirit target destroy Particle
		if (particles->GetState() == 12)
		{
			particles->SetSize(particles->GetWidth() - 0.5, particles->GetHeight() - 0.5);
			particles->SetDirection(particles->GetDirection() + 20);

			if (particles->GetWidth() <= 0) particles->Delete();
		}

		// Spirit gate destroy
		if (particles->GetState() == 13)
		{
			particles->SetSize(particles->GetWidth() - 0.5, particles->GetHeight() - 0.5);
			if (particles->GetWidth() <= 0) particles->Delete();
		}
	}
	particleList.delete_if(deleted);
}

// Delete sand platform if you dash on it, and check every neighbor sand platform and call itself again until destroyed all of them on the same row (risky I know)
void CMyGame::DeletePlatform(CSprite& platform)
{
	sfx2.Play("sand.wav", 0);
	sfx2.Volume(0.6);

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

// Fragment is needed to open the door (spoiler), the last room background is change, depend how much fragment you have
void CMyGame::PickUpFragment(CSprite& fragment)
{
	if ((string)fragment.GetProperty("tag") == "fragment" && fragment.HitTest(&player))
	{
		FragmentParticle(CColor(36, 113, 31), fragment.GetPosition());

		fragment.SetState(-fragment.GetState());
		fragment.SetProperty("tag", "NULL");

		gFragmentNumber++;

		switch (gFragmentNumber)
		{
		case 1:
			ui.PlayDialogue(1);
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
			ui.exitBool = true;
			if (IsKeyDown(SDLK_f))
			{
				mapX = 10;
				mapY = 9;
				MapGenerator(mapArray[mapX][mapY].mapName);
				ui.exitBool = false;
			}
		}
		else ui.exitBool = false;
	}
}

// Pick up spirit bow
void CMyGame::PickUpBow(CSprite& fragment)
{
	if ((string)fragment.GetProperty("tag") == "bow" && fragment.HitTest(&player))
	{
		ui.PlayDialogue(6);
		sfx2.Play("bowpick.wav", 0);
		sfx2.Volume(0.6);

		bowStatus = BOW_ACTIVE;
		fragment.SetState(-fragment.GetState());
		fragment.SetProperty("tag", "NULL");
		PickUpBowParticle(fragment.GetPosition());

		SaveMap(mapArray[mapX][mapY].mapName);
		mapArray[mapX][mapY].newGame = false;
	}
}

// Skill Orb is give you different skills such us (climb, dash, double jump (spoiler again))
void CMyGame::PickUpSkillOrb(CSprite& fragment)
{
	if ((string)fragment.GetProperty("tag") == "skillOrb" && fragment.HitTest(&player))
	{
		SkillOrbParticle(CColor(122, 8, 8), fragment.GetPosition());

		fragment.SetState(-fragment.GetState());
		fragment.SetProperty("tag", "NULL");

		skillOrb++;
		switch (skillOrb)
		{
		case 1:
			ui.PlayDialogue(2);
			slideStatus = SLIDE_ACTIVE;
			break;
		case 2:
			ui.PlayDialogue(3);
			jumpStatus = DOUBLEJUMP_ACTIVE;
			break;
		case 3:
			ui.PlayDialogue(4);
			dashStatus = DASH_ACTIVE;
			break;
		}
		SaveMap(mapArray[mapX][mapY].mapName);
		mapArray[mapX][mapY].newGame = false;
	}
	else if ((string)fragment.GetProperty("tag") == "skillOrb")
	{
		float rotateTo = LookAt(player.GetX(), player.GetY(), fragment.GetX(), fragment.GetY());
		fragment.SetRotation(rotateTo + 90);
	}
}

// Extra Jump crystal, gives +1 extra jump
void CMyGame::ExtraJump(CSprite& fragment)
{
	if ((string)fragment.GetProperty("tag") == "extraJump" && fragment.HitTest(&player) && fragment.GetState() > 0)
	{
		jumpN++;
		fragment.SetState(-fragment.GetState());
		ExtraJumpParticle(1, fragment.GetPosition());
	}

	if (status == RUN && (string)fragment.GetProperty("tag") == "extraJump" && fragment.GetState() < 0)
	{
		fragment.SetState(fragment.GetState() * -1);
		ExtraJumpParticle(2, fragment.GetPosition());
	}
}

// Platform switch between hide and show (only react to the player if show)
void CMyGame::HidePlatform(CSprite& fragment)
{
	if ((string)fragment.GetProperty("tag") == "showPlatform")
	{
		fragment.SetHealth(fragment.GetHealth() - (float)GetDeltaTime() / 1000);
		if (fragment.GetHealth() < 0)
		{
			fragment.SetProperty("tag", "hidePlatform");
			fragment.SetImage("hideTile");
			fragment.SetHealth(2);
			fragment.SetSize(size, size);
		}
	}

	if ((string)fragment.GetProperty("tag") == "hidePlatform")
	{
		fragment.SetHealth(fragment.GetHealth() - (float)GetDeltaTime() / 1000);
		if (fragment.GetHealth() < 0)
		{
			fragment.SetProperty("tag", "showPlatform");
			fragment.SetImage("showTile");
			fragment.SetHealth(2);
			fragment.SetSize(size, size);
		}
	}
}

// Set the checkpoint position
void CMyGame::CheckPoint(CSprite& fragment)
{
	if ((string)fragment.GetProperty("tag") == "checkpoint")
	{
		float distance = Distance(fragment.GetX(), fragment.GetY(), player.GetX(), player.GetY());
		if (distance < 100)
		{
			respawnPos = fragment.GetPosition();
		}
	}
}

// if the palyer touch the spike, it dies and reset the position to the checkpoint position
void CMyGame::SpikeTile(CSprite& fragment)
{
	if (((string)fragment.GetProperty("tag") == "spike" ||
		(string)fragment.GetProperty("tag") == "saw" ||
		(string)fragment.GetProperty("tag") == "bouncyEnemy" ||
		(string)fragment.GetProperty("tag") == "circleEnemy")
		&& fragment.HitTest(&player, 1))
	{
			PlayerDeathParticle();
	}
}

// In editor mode if Q is pressed, the tile on the mouse position deleted
void CMyGame::DeleteTile(CSprite& fragment)
{
	if (IsKeyDown(SDLK_q) && fragment.HitTest(GetMouseCoords()) && editorMode)
	{
		fragment.SetState(0);
		fragment.Delete();
		fragment.SetProperty("tag", "NULL");
	}
}

// Spirit Target
void CMyGame::SpiritTarget(CSprite& fragment)
{
	bool openGate = false;
	if ((string)fragment.GetProperty("tag") == "spiritTarget")
	{
		for (CSprite* arrows : arrowList)
		{
			if (arrows->HitTest(&fragment))
			{
				fragment.SetState(-fragment.GetState());
				fragment.SetProperty("tag", "NULL");
				fragment.Delete();
				openGate = true;
				SpiritParticle(1, fragment.GetPosition());
				SaveMap(mapArray[mapX][mapY].mapName);
				mapArray[mapX][mapY].newGame = false;
			}
		}
	}

	if ((string)fragment.GetProperty("tag") == "spiritGate")
	{
		if (fragment.HitTest(&player))
		{
			if (!godMode)
			{
				PlayerDeathParticle();
			}
		}
	}

	if (openGate)
	{
		for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 18; j++)
			{
				if ((string)tileArray[i][j]->GetProperty("tag") == "spiritGate")
				{
					tileArray[i][j]->SetState(-tileArray[i][j]->GetState());
					tileArray[i][j]->SetProperty("tag", "NULL");
					tileArray[i][j]->Delete();
					SpiritParticle(2, tileArray[i][j]->GetPosition());
					SaveMap(mapArray[mapX][mapY].mapName);
					mapArray[mapX][mapY].newGame = false;
				}
			}
		}
	}
}

// Moving Platform / object
void CMyGame::MovingPlatform(CSprite& fragment)
{
	// Moving Platform change velocity
	int x = fragment.GetX() / size;
	int y = fragment.GetY() / size;
	if ((string)fragment.GetProperty("tag") == "movingPlatform" ||
		(string)fragment.GetProperty("tag") == "saw")
	{
		if (fragment.GetHealth() == 0 && (fragment.GetX() == tileArray[x][y]->GetX() || fragment.GetY() == tileArray[x][y]->GetY()) && (string)tileArray[x][y]->GetProperty("tag") == "platformSwitch")
		{
			fragment.SetVelocity(fragment.GetVelocity() * -1);
			fragment.SetOmega(fragment.GetOmega() * -1);
		}
	}

	if ((string)fragment.GetProperty("tag") == "saw")
	{
		fragment.SetSize(size * 2, size * 2);
		if (fragment.HitTest(tileArray[x][y]) && (string)tileArray[x][y]->GetProperty("tag") == "platformSwitch") fragment.SetHealth(1);
		else fragment.SetHealth(0);
	}
	if ((string)fragment.GetProperty("tag") == "movingPlatform")
	{
		fragment.SetSize(size * 1.5, size * 1.5);
		if (fragment.HitTest(tileArray[x][y]) && (string)tileArray[x][y]->GetProperty("tag") == "platformSwitch") fragment.SetHealth(1);
		else fragment.SetHealth(0);
	}
}

// Turret system
void CMyGame::TurretPlatform(CSprite& fragment)
{
	for (CSprite* projectiles : turretProjectileList)
	{
		if (projectiles->HitTest(&fragment) && (string)fragment.GetProperty("tag") != "turret" &&
			(string)fragment.GetProperty("tag") != "bouncyEnemy" &&
			(string)fragment.GetProperty("tag") != "circleEnemy")
		{
			projectiles->Delete();
			TurretProjectileDieParticle(projectiles->GetPosition());
		}
	}

	if ((string)fragment.GetProperty("tag") == "turret")
	{
		if (fragment.GetHealth() < 0)
		{
			sfx3.Play("fire.wav", 0);
			sfx3.Volume(0.8);

			fragment.SetHealth(1.5);
			CSprite* projectile = new CSprite(fragment.GetX(), fragment.GetY(), "fireball.bmp", CColor::Black(), GetTime());
			turretProjectileList.push_back(projectile);
			projectile->SetSize(24, 24);
			projectile->SetDirection(fragment.GetRotation());
			projectile->SetRotation(fragment.GetRotation());
			projectile->SetSpeed(300);
		}
		else
		{
			fragment.SetHealth(fragment.GetHealth() - delta / 1000);
		}
	}

	if ((string)fragment.GetProperty("tag") == "bouncyEnemy" ||
		(string)fragment.GetProperty("tag") == "circleEnemy")
	{
		if (fragment.GetHealth() < 0)
		{
			sfx3.Play("fire.wav", 0);
			sfx3.Volume(0.8);

			fragment.SetHealth(1);
			CSprite* projectile = new CSprite(fragment.GetX(), fragment.GetY(), "fireball.bmp", CColor::Black(), GetTime());
			turretProjectileList.push_back(projectile);
			projectile->SetSize(24, 24);
			float lookAt = LookAt(player.GetX(), player.GetY(), fragment.GetX(), fragment.GetY());
			projectile->SetDirection(lookAt + 90);
			projectile->SetRotation(lookAt + 90);
			projectile->SetSpeed(300);
		}
		else
		{
			fragment.SetHealth(fragment.GetHealth() - delta / 1000);
		}
	}
}

// Bouncy enemy physics 
void CMyGame::BouncyEnemy(CSprite& fragment)
{
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 18; j++)
		{
			if ((string)tileArray[i][j]->GetProperty("tag") == "bouncyEnemy")
			{
				tileArray[i][j]->SetSize(size * 1.5, size * 1.5);
				tileArray[i][j]->Accelerate(0,-0.1);
				if (tileArray[i][j]->GetXVelocity() > 0) tileArray[i][j]->SetOmega(350);
				else tileArray[i][j]->SetOmega(-350);

				CVector vector = tileArray[i][j]->GetVelocity();
				CVector vectorN = Normalise(tileArray[i][j]->GetVelocity());
				float xN = sin(DEG2RAD(fragment.GetRotation()));
				float yN = cos(DEG2RAD(fragment.GetRotation()));
				CVector n = CVector(0, 0);

				CVector position = fragment.GetPosition() - tileArray[i][j]->GetPosition();
				CVector nP = Normalize(position);
				if (tileArray[i][j]->HitTest(&fragment))
				{
					n = CVector(xN, yN);
					if (Dot(vectorN, n) < 0 && tileArray[i][j]->GetY() - vectorN.m_y > fragment.GetY() &&
						tileArray[i][j]->GetX() - vectorN.m_x < fragment.GetX() + fragment.GetWidth() / 2 &&
						tileArray[i][j]->GetX() - vectorN.m_x > fragment.GetX() - fragment.GetWidth() / 2)
					{
						vector = Reflect(vector, n);
						tileArray[i][j]->SetPosition(tileArray[i][j]->GetPosition() - nP * 2);
					}
					n = CVector(xN, yN);
					if (Dot(vectorN, n) > 0 && tileArray[i][j]->GetY() - vectorN.m_y < fragment.GetY() &&
						tileArray[i][j]->GetX() - vectorN.m_x < fragment.GetX() + fragment.GetWidth() / 2 &&
						tileArray[i][j]->GetX() - vectorN.m_x > fragment.GetX() - fragment.GetWidth() / 2)
					{
						vector = Reflect(vector, n);
						tileArray[i][j]->SetPosition(tileArray[i][j]->GetPosition() - nP * 2);
					}
					n = CVector(yN, xN);
					if (Dot(vectorN, n) < 0 && tileArray[i][j]->GetX() - vectorN.m_x > fragment.GetX() &&
						tileArray[i][j]->GetY() - vectorN.m_y < fragment.GetY() + fragment.GetHeight() / 2 &&
						tileArray[i][j]->GetY() - vectorN.m_y > fragment.GetY() - fragment.GetHeight() / 2)
					{
						vector = Reflect(vector, n);
						tileArray[i][j]->SetPosition(tileArray[i][j]->GetPosition() - nP * 2);
					}
					n = CVector(yN, xN);
					if (Dot(vectorN, n) > 0 && tileArray[i][j]->GetX() - vectorN.m_x < fragment.GetX() &&
						tileArray[i][j]->GetY() - vectorN.m_y < fragment.GetY() + fragment.GetHeight() / 2 &&
						tileArray[i][j]->GetY() - vectorN.m_y > fragment.GetY() - fragment.GetHeight() / 2)
					{
						vector = Reflect(vector, n);
						tileArray[i][j]->SetPosition(tileArray[i][j]->GetPosition() - nP * 2);
					}
					tileArray[i][j]->SetVelocity(vector);
				}
			}
		}
	}
}

// Circle enemy math
void CMyGame::CircleEnemy(CSprite& fragment, float freq)
{
	if ((string)fragment.GetProperty("tag") == "circleEnemy")
	{
		fragment.SetSize(size * 1.5, size * 1.5);
		fragment.SetXVelocity(sin(freq) * 300);
		fragment.SetYVelocity(cos(freq) * 300);
	}
}

// Remove enemy if die
void CMyGame::RemoveEnemy(CSprite& fragment)
{
	if ((string)fragment.GetProperty("tag") == "bouncyEnemy" ||
		(string)fragment.GetProperty("tag") == "circleEnemy")
	{
		for (CSprite* arrows : arrowList)
		{
			if (arrows->HitTest(&fragment))
			{
				fragment.Delete();
				EnemyDeathParticle(fragment);
				arrows->Delete();
				fragment.SetProperty("tag", "NULL");
			}
		}
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