#pragma once

class CMyGame : public CGame
{
	//Loads of enum for the different status
	enum { LEFT, RIGHT } align;
	enum {JUMP, DASH, RUN} status;
	enum {DOUBLEJUMP_ACTIVE, DOUBLEJUMP_INACTIVE} jumpStatus;
	enum {DASH_INACTIVE, DASH_ACTIVE, DASH_OFF, DASH_OVER, DASH_READY} dashStatus;
	enum {SLIDE_INACTIVE, SLIDE_ACTIVE, LEFT_SLIDE, RIGHT_SLIDE, SLIDE_OFF } slideStatus;
	enum { BOW_INACTIVE, BOW_ACTIVE , BOW_ON, BOW_OFF } bowStatus;

	int size; // One Unit size (ex: tile and player size)
	int jumpN; // Jump Number for double jump

	float delta;

	CSprite bow;
	CSpriteList arrowList;

	CSprite backGround; // Background sprite

	CSprite tileBrush; // Tile brush for creating maps easyily

	// Showtiles is for the tileMap, textbool is for the tutorial text, editor mode is for map editor mode and godMode is for easier testing
	bool showTiles, editorMode, godMode;

	CSprite* tileMap[200]; // Loading all the different tiles here for generating the map and save memory
	CSprite* tileArray[32][18]; // the map is fixed size 32*18 tiles

	// Struct for the different map attributes
	struct mapComponents
	{
		CVector defaultPos;
		int lvlNum = 0;
		string mapName;
		char* backGround;
		char* musicName;
		bool newGame;
	};

	CVector respawnPos; // Respawn position

	CSpriteList particleList; // Particle List
	CSpriteList turretProjectileList;

	CSoundPlayer sfx, sfx2, sfx3;
public:
	int mapX, mapY; //different map coordination
	mapComponents mapArray[20][20];

	int mapNumber;
	int gFragmentNumber;
	int skillOrb;

	CSprite player;

public:
	CMyGame(void);
	~CMyGame(void);

	virtual void OnUpdate();
	virtual void OnDraw(CGraphics* g);
	virtual void OnInitialize();
	virtual void OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode);
	virtual void OnLButtonDown(Uint16 x,Uint16 y);

	void NewGame();
	void BowSystem();
	void Animation(char* name, int frame);
	void CharacterController();
	void TileBrush();
	void LoadTileGenerator();
	void MapGenerator(string map);
	void SaveMap(string map);
	void JumpToMap(bool _bool);
	void FragmentParticle(CColor color, CVector position);
	void SkillOrbParticle(CColor color, CVector position);
	void PickUpBowParticle(CVector position);
	void ArrowParticle(CVector position);
	void ArrowDieParticle(CVector position);
	void TurretProjectileParticle(CVector position);
	void TurretProjectileDieParticle(CVector position);
	void EnemyDeathParticle(CSprite& fragment);
	void DustParticle();
	void SnowParticle();
	void PlayerParticle();
	void DashParticle(char* sprite);
	void PlayerDeathParticle();
	void ExtraJumpParticle(int mode, CVector position);
	void SpiritParticle(int mode, CVector position);
	void ParticleSystem();
	void DeletePlatform(CSprite& platform);
	void PickUpFragment(CSprite& fragment);
	void PickUpSkillOrb(CSprite& fragment);
	void PickUpBow(CSprite& fragment);
	void ExtraJump(CSprite& fragment);
	void HidePlatform(CSprite& fragment);
	void SpikeTile(CSprite& fragment);
	void SpiritTarget(CSprite& fragment);
	void CheckPoint(CSprite& fragment);
	void DeleteTile(CSprite& fragment);
	void MovingPlatform(CSprite& fragment);
	void TurretPlatform(CSprite& fragment);
	void BouncyEnemy(CSprite& fragment);
	void CircleEnemy(CSprite& fragment, float freq);
	void RemoveEnemy(CSprite& fragment);
	float Distance(float targetX, float targetY, float sourceX, float sourceY);
	float LookAt(float targetX, float targetY, float sourceX, float sourceY);
};
