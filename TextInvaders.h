#pragma once
#include <string>
#include <vector>

const char* PLAYER_SPRITE[] = { " /A\\ ", "|/V\\|" };

const char* PLAYER_EXPLOSION_SPRITE[] = { " |@/.", ".`//-", "`_~; ", "_~`\"." };

const char PLAYER_MISSILE_SPRITE = '!';

const char* SHIELD_SPRITE[] = {"/IIIII\\", "IIIIIII", "I/   \\I"};

const char* ALIEN30_SPRITE[] = { "/oO\\", "/\"\"\\", "/Oo\\", "<''>" };

const char* ALIEN20_SPRITE[] = { " >< ", "|\\/|", "|><|", "/  \\" };

const char* ALIEN10_SPRITE[] = { "/--\\", "/  \\", "/--\\", "<  >" };

const char* ALIEN_EXPLOSION[] = { "\\||/", "/|\\*" };

const char* ALIEN_BOMB_SPRITE = "\\|/-";

enum
{
	SHIELD_SPRITE_HEIGHT = 3,
	SHIELD_SPRITE_WIDTH = 7,
	NUM_ALIEN_ROWS = 5,
	NUM_ALIEN_COLS = 11,
	MAX_NUMBER_ALIEN_BOMBS = 3,
	MAX_NUMBER_LIVES = 3,
	PLAYER_SPRITE_WIDTH = 5,
	PLAYER_SPRITE_HEIGHT = 2,
	NOT_IN_PLAY = -1,
	PLAYER_MOVEMENT_AMOUNT = 1,
	PLAYER_MISSILE_SPEED = 1,
	FPS = 30,
	NUM_SHIELDS = 4,
	ALIEN_SPRITE_WIDTH = 4,
	ALIEN_SPRITE_HEIGHT = 2,
	ALIENS_X_PADDING = 1,
	ALIENS_Y_PADDING = 1,
	ALIENS_EXPLOSION_TIME = 4,
	ALIEN_BOMB_SPEED = 1
};

enum AlienState
{
	AS_ALIVE = 0,
	AS_DEAD,
	AS_EXPLODING
};

enum GameState
{
	GS_INTRO = 0,
	GS_HIGH_SCORES,
	GS_PLAY,
	GS_PLAYER_DEAD,
	GS_WAIT,
	GS_GAME_OVER
};

struct Position
{
	int x;
	int y;
};

struct Size
{
	int width;
	int height;
};

struct Player
{
	Position position;
	Position missile;
	Size spriteSize;
	int animation;
	int lives; // max 3
	int score;
};

struct Shield
{
	Position position;
	char* sprite[SHIELD_SPRITE_HEIGHT];
};

struct AlienBomb
{
	Position position;
	int animation;
};

struct AlienSwarm
{
	Position position;
	AlienState aliens[NUM_ALIEN_ROWS][NUM_ALIEN_COLS];
	AlienBomb bombs[MAX_NUMBER_ALIEN_BOMBS];
	Size spriteSize;
	int animation;
	int direction; // >0 - for going right, <0 - for going left
	int numberOfBombsInPlay;
	int movementTime; // capture how fast the aliens should be going
	int explosionTimer; // capture how long to explode for
	int numAliensLeft; // capture when to go to next level
	int line; // capture when the aliens win starts at current level and decreases to zero
};

struct AlienUFO
{
	Position position;
	Size size;
	int points; // from 50-200
};

struct Score
{
	int score;
	std::string name;
};

struct HighScoreTable
{
	std::vector<Score> scores;
};

struct Game
{
	Size windowSize;
	GameState currentState;
	int level;
	int waitTimer;
};