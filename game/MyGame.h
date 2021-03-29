#pragma once

class CMyGame : public CGame
{
	//Loads of enum for the different status
	enum { LEFT, RIGHT } align;
	enum {JUMP, DASH, RUN} status;
	enum {DOUBLEJUMP_ACTIVE, DOUBLEJUMP_INACTIVE} jumpStatus;
	enum {DASH_INACTIVE, DASH_ACTIVE, DASH_OFF, DASH_OVER, DASH_READY} dashStatus;
	enum {SLIDE_INACTIVE, SLIDE_ACTIVE, LEFT_SLIDE, RIGHT_SLIDE, SLIDE_OFF } slideStatus;

	int size; // One Unit size (ex: tile and player size)
	int jumpN; // Jump Number for double jump

	CSprite backGround; // Background sprite

	CSprite tileBrush; // Tile brush for creating maps easyily

	// Showtiles is for the tileMap, textbool is for the tutorial text, editor mode is for map editor mode and godMode is for easier testing
	bool showTiles, textBool, editorMode, godMode;

	int mapX, mapY; //different map coordination
	int gFragmentNumber, skillOrb;

	CSprite* tileMap[100]; // Loading all the different tiles here for generating the map and save memory
	CSprite* tileArray[32][18]; // the map is fixed size 32*18 tiles

	// Struct for the different map attributes
	struct mapComponents
	{
		int lvlNum;
		string mapName;
		char* backGround;
		string text;
		bool newGame;
	};

	mapComponents mapArray[20][20];

	CSprite player;	 // Player sprite
	CVector respawnPos; // Respawn position

	CSpriteList particleList; // Particle List

public:
	CMyGame(void);
	~CMyGame(void);

	virtual void OnUpdate();
	virtual void OnDraw(CGraphics* g);

	virtual void OnInitialize();

	virtual void OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode);
	virtual void OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode);
	virtual void OnLButtonDown(Uint16 x,Uint16 y);
	virtual void OnLButtonUp(Uint16 x,Uint16 y);
	virtual void OnRButtonDown(Uint16 x,Uint16 y);
	virtual void OnRButtonUp(Uint16 x,Uint16 y);

	void NewGame();
	void Animation(char* name, int frame);
	void CharacterController();
	void TileBrush();
	void LoadTileGenerator();
	void MapGenerator(string map);
	void SaveMap(string map);
	void JumpToMap();
	void PickUpFragment(CColor color, CVector position);
	void PickUpSkillOrb(CColor color, CVector position);
	void DustParticle();
	void SnowParticle();
	void PlayerParticle();
	void DashParticle(char* sprite);
	void PlayerDeathParticle();
	void ParticleSystem();
	void DeletePlatform(CSprite& platform);
	float Distance(float targetX, float targetY, float sourceX, float sourceY);
	float LookAt(float targetX, float targetY, float sourceX, float sourceY);
};
