#include <iostream>
#include "TextInvaders.h"
#include "CursesUtils.h"
#include <string>
#include <ctime>
#include <cmath>
#include <cstring>
#include <cstdlib>

void InitGame(Game& game);

void InitPlayer(const Game& game, Player& player);
void InitShields(const Game& game, Shield shields[], int numberOfShields);
void InitAliens(const Game& game, AlienSwarm& aliens);

void DrawGame(const Game& game, const Player& player, Shield shields[], int numberOfShields, const AlienSwarm& aliens);
void DrawPlayer(const Player& player, const char* sprite[]);
void DrawShields(const Shield shields[], int numberOfShields);
void DrawAliens(const AlienSwarm& aliens);

void ResetPlayer(const Game& game, Player& player);
void ResetMissile(Player& player);
void ResetMovementTime(AlienSwarm& aliens);

void UpdateGame(Game& game, Player& player, Shield shields[], int numberOfShields, AlienSwarm& aliens);

int HandleInput(Game& game, Player& player);
void MovePlayer(const Game& game, Player& player, int dx);
void PlayerShoot(Player& player);

void UpdateMissile(Player& player);
bool UpdateAliens(const Game& game, AlienSwarm& aliens, Player& player, Shield shields[], int numberOfShields);
void FindEmptyRowsAndColumns(const AlienSwarm& aliens, int& emptyColsLeft, int& emptyColsRight, int& emptyRowsBottom);

int IsCollision(const Position& projectile, const Shield shields[], int numberOfShields, Position& shieldCollidePoint);
bool IsCollision(const Player& player, const AlienSwarm& aliens, Position& alienCollidePositionInArray);
bool IsCollision(const Position& projectile, const Position& spritePosition, const Size& spriteSize);

void ResolveShieldCollision(Shield shields[], int shieldIndex, const Position& shieldCollidePoint);
int ResolveAlienCollision(AlienSwarm& aliens, const Position& hitPositionInAliensArray);
void DestroyShields(const AlienSwarm& aliens, Shield shields[], int numberOfShields);
void CollideShieldsWithAlien(Shield shields[], int numberOfShields, int x, int y, const Size& spriteSize);

void CleanUpShields(Shield shields[], int numberOfShields);

bool ShouldShootBomb(const AlienSwarm& aliens);
void ShootBomb(AlienSwarm& aliens, int columnToShoot);
bool UpdateBombs(const Game& game, AlienSwarm& aliens, Player& player, Shield shields[], int numberOfShields);

int main()
{
	srand(time(NULL));

	Game game;
	Player player;
	Shield shields[NUM_SHIELDS];
	AlienSwarm aliens;

	InitializeCurses(true);

	InitGame(game);
	InitPlayer(game, player);
	InitShields(game, shields, NUM_SHIELDS);
	InitAliens(game, aliens);



	bool quit = false;
	int input;

	clock_t lastTime = clock();

	while (!quit)
	{
		input = HandleInput(game, player);
		if (input != 'q')
		{
			clock_t currentTime = clock();
			clock_t dt = currentTime - lastTime;

			if (dt > CLOCKS_PER_SEC / FPS)
			{
				lastTime = currentTime;

				UpdateGame(game, player, shields, NUM_SHIELDS, aliens);
				ClearScreen();
				DrawGame(game, player, shields, NUM_SHIELDS, aliens);
				RefreshScreen();
			}
		}
		else
		{
			quit = true;
		}
	}
	
	CleanUpShields(shields, NUM_SHIELDS);
	ShutdownCurses();

	return 0;
}

void InitGame(Game& game)
{
	game.windowSize.width = ScreenWidth();
	game.windowSize.height = ScreenHeight();
	game.level = 1;
	game.currentState = GS_PLAY; // TODO: change to GS_INTRO when we're done
}

void InitPlayer(const Game& game, Player& player)
{
	player.lives = MAX_NUMBER_LIVES;
	player.spriteSize.width = PLAYER_SPRITE_WIDTH;
	player.spriteSize.height = PLAYER_SPRITE_HEIGHT;
	ResetPlayer(game, player);
}

void ResetPlayer(const Game& game, Player& player)
{
	player.position.x = game.windowSize.width / 2 - player.spriteSize.width / 2;
	player.position.y = game.windowSize.height - player.spriteSize.height - 1;

	player.animation = 0;
	ResetMissile(player);
}

void ResetMissile(Player& player)
{
	player.missile.x = NOT_IN_PLAY;
	player.missile.y = NOT_IN_PLAY;
}

void ResetMovementTime(AlienSwarm& aliens)
{
	aliens.movementTime = aliens.line * 2 + (5 *
		(float(aliens.numAliensLeft) / float(NUM_ALIEN_COLS * NUM_ALIEN_ROWS)));
}

int HandleInput(Game& game, Player& player)
{
	int input = GetChar();
	switch (input)
	{
	case 'q':
		return input;
	case AK_LEFT:
		if (game.currentState == GS_PLAY)
		{
			MovePlayer(game, player, -PLAYER_MOVEMENT_AMOUNT);
		}
		break;
	case AK_RIGHT:
		if (game.currentState == GS_PLAY)
		{
			MovePlayer(game, player, PLAYER_MOVEMENT_AMOUNT);
		}
		break;
	case ' ':
		if (game.currentState == GS_PLAY)
		{
			PlayerShoot(player);
		}
		else if (game.currentState == GS_PLAYER_DEAD)
		{
			player.lives--;
			player.animation = 0;
			if (player.lives == 0)
			{
				game.currentState = GS_GAME_OVER;
			}
			else
			{
				game.currentState = GS_WAIT;
				game.waitTimer = 10;
			}
		}
	}
	
	return input;
}

void UpdateGame(Game& game, Player& player, Shield shields[], int numberOfShields, AlienSwarm& aliens)
{
	if (game.currentState == GS_PLAY)
	{
		UpdateMissile(player);
		Position shieldCollidePoint;

		int shieldIndex = IsCollision(player.missile, shields, numberOfShields, shieldCollidePoint);

		if (shieldIndex != NOT_IN_PLAY)
		{
			ResetMissile(player);
			ResolveShieldCollision(shields, shieldIndex, shieldCollidePoint);
		}

		Position playerAlienCollidePoint;
		if (IsCollision(player, aliens, playerAlienCollidePoint))
		{
			ResetMissile(player);
			player.score += ResolveAlienCollision(aliens, playerAlienCollidePoint);
		}

		UpdateAliens(game, aliens, player, shields, numberOfShields);
	}
	else if (game.currentState == GS_PLAYER_DEAD)
	{
		player.animation = (player.animation + 1) % 2;
	}
	else if (game.currentState == GS_WAIT)
	{
		game.waitTimer--;

		if (game.waitTimer == 0)
		{
			game.currentState = GS_PLAY;
		}
	}
}

void DrawGame(const Game& game, const Player& player, Shield shields[], int numberOfShields, const AlienSwarm& aliens)
{
	if (game.currentState == GS_PLAY || game.currentState == GS_PLAYER_DEAD || game.currentState == GS_WAIT)
	{
		if (game.currentState == GS_PLAY || game.currentState == GS_WAIT)
		{
			DrawPlayer(player, PLAYER_SPRITE);
		}
		else
		{
			DrawPlayer(player, PLAYER_EXPLOSION_SPRITE);
		}
		
		DrawShields(shields, numberOfShields);
		DrawAliens(aliens);
	}
	
}

void MovePlayer(const Game& game, Player& player, int dx)
{
	if (player.position.x + player.spriteSize.width + dx > game.windowSize.width)
	{
		player.position.x = game.windowSize.width - player.spriteSize.width; // rightmost position
	}
	else if (player.position.x + dx < 0)
	{
		player.position.x = 0;
	}
	else
	{
		player.position.x += dx;
	}
}

void PlayerShoot(Player& player)
{
	if (player.missile.x == NOT_IN_PLAY || player.missile.y == NOT_IN_PLAY)
	{
		player.missile.y = player.position.y - 1; //one row above the player
		player.missile.x = player.position.x + player.spriteSize.width / 2;
	}
}

void DrawPlayer(const Player& player, const char* sprite[])
{
	DrawSprite(player.position.x, player.position.y, PLAYER_SPRITE, player.spriteSize.height);

	if (player.missile.x != NOT_IN_PLAY)
	{
		DrawCharacter(player.missile.x, player.missile.y, PLAYER_MISSILE_SPRITE);
	}
}

void UpdateMissile(Player& player)
{
	if (player.missile.y != NOT_IN_PLAY)
	{
		player.missile.y -= PLAYER_MISSILE_SPEED;

		if (player.missile.y < 0)
		{
			ResetMissile(player);
		}
	}
}

bool UpdateAliens(const Game& game, AlienSwarm& aliens, Player& player, Shield shields[], int numberOfShields)
{
	if (UpdateBombs(game, aliens, player, shields, numberOfShields))
	{
		return true;
	}

	if (aliens.explosionTimer >= 0)
	{
		aliens.explosionTimer--; // even if explosion timer is zero, it'll go to NOT_IN_PLAY
	}

	for (int row = 0; row < NUM_ALIEN_ROWS; row++)
	{
		for (int col = 0; col < NUM_ALIEN_COLS; col++)
		{
			if (aliens.aliens[row][col] == AS_EXPLODING && aliens.explosionTimer == NOT_IN_PLAY)
			{
				aliens.aliens[row][col] = AS_DEAD;
			}
		}
	}

	aliens.movementTime--;

	bool moveHorizontal = 0 >= aliens.movementTime;
	int emptyColsLeft = 0;
	int emptyColsRight = 0;
	int emptyRowsBottom = 0;

	FindEmptyRowsAndColumns(aliens, emptyColsLeft, emptyColsRight, emptyRowsBottom);

	int numberOfColumns = NUM_ALIEN_COLS - emptyColsLeft - emptyColsRight;
	int leftAlienPosition = aliens.position.x + emptyColsLeft * (aliens.spriteSize.width + ALIENS_X_PADDING);
	int rightAlienPosition = leftAlienPosition + numberOfColumns * aliens.spriteSize.width + (numberOfColumns - 1) * ALIENS_X_PADDING;

	if (((rightAlienPosition >= game.windowSize.width && aliens.direction > 0) ||
		(leftAlienPosition <= 0 && aliens.direction < 0)) && 
		moveHorizontal && 
		aliens.line > 0)
	{
		//move down position
		moveHorizontal = false;
		aliens.position.y++;
		aliens.line--;
		aliens.direction = -aliens.direction;
		ResetMovementTime(aliens);
		DestroyShields(aliens, shields, numberOfShields);
	}

	if (moveHorizontal)
	{
		aliens.position.x += aliens.direction;
		ResetMovementTime(aliens);
		aliens.animation = aliens.animation == 0 ? 1 : 0;
		DestroyShields(aliens, shields, numberOfShields);
	}

	if (!moveHorizontal)
	{
		int activeColumns[NUM_ALIEN_COLS];

		int numActiveCols = 0;
		for (int c = emptyColsLeft; c < numberOfColumns; ++c)
		{
			for (int r = 0; r < NUM_ALIEN_ROWS; r++)
			{
				if (aliens.aliens[r][c] == AS_ALIVE)
				{
					activeColumns[numActiveCols] = c;
					numActiveCols++;
					break;
				}
			}
		}

		if (ShouldShootBomb(aliens))
		{
			if (numActiveCols > 0)
			{
				int numberOfShots = ((rand() % 3) + 1) - aliens.numberOfBombsInPlay;

				for (int i = 0; i < numberOfShots; i++)
				{
					int columnToShoot = rand() % numActiveCols;
					ShootBomb(aliens, columnToShoot);
				}
			}
		}
	}

	return false;
}

void FindEmptyRowsAndColumns(const AlienSwarm& aliens, int& emptyColsLeft, int& emptyColsRight, int& emptyRowsBottom)
{
	
	// check each column, starting at the left side
	bool found = false;
	for (int col = 0; col < NUM_ALIEN_COLS && !found; ++col)
	{
		for (int row = 0; row < NUM_ALIEN_ROWS && !found; ++row)
		{
			if ((aliens.aliens[row][col] == AS_DEAD))
			{
				if (row == NUM_ALIEN_ROWS - 1) // last row
				{
					emptyColsLeft++;
				}
				
			}
			else
			{
				found = true;
			}
		}
	}

	// check each column, starting at the right side
	found = false;

	for (int col = NUM_ALIEN_COLS - 1; col >= 0 && !found; col--)
	{
		for (int row = 0; row < NUM_ALIEN_ROWS && !found; row++)
		{
			if (aliens.aliens[row][col] == AS_DEAD)
			{
				if (row == NUM_ALIEN_ROWS - 1)
				{
					emptyColsRight++;
				}
			}
			else
			{
				found = true;
			}
		}
	}

	//check each row, starting at the bottom
	found = false;
	for (int row = NUM_ALIEN_ROWS - 1; row >= 0 && !found; row--)
	{
		for (int col = 0; col < NUM_ALIEN_COLS && !found; col++)
		{
			if (aliens.aliens[row][col] == AS_DEAD)
			{
				if (col == NUM_ALIEN_COLS - 1)
				{
					emptyRowsBottom++;
				}
			}
			else
			{
				found = true;
			}
		}
	}
}

void CollideShieldsWithAlien(Shield shields[], int numberOfShields, int alienX, int alienY, const Size& size)
{
	for (int s = 0; s < numberOfShields; s++)
	{
		Shield& shield = shields[s];

		if (alienX < shield.position.x + SHIELD_SPRITE_WIDTH && 
			alienX + size.width >= shield.position.x &&
			alienY < shield.position.y + SHIELD_SPRITE_HEIGHT &&
			alienY + size.height >= shield.position.y)
		{
			// we are colliding

			int dy = alienY - shield.position.y;
			int dx = alienX - shield.position.x;

			for (int h = 0; h < size.height; h++) // check height
			{
				int shieldY = dy + h;
				if (shieldY >= 0 && shieldY < SHIELD_SPRITE_HEIGHT)
				{
					for (int w = 0; w < size.width; w++) // check width
					{
						int shieldX = dx + w;

						if (shieldX >= 0 && shieldX < SHIELD_SPRITE_WIDTH)
						{
							shield.sprite[shieldY][shieldX] = ' ';
						}
					}
				}
			}

			break;

		}
	}
}

void InitShields(const Game& game, Shield shields[], int numberOfShields)
{
	int firstPadding = ceil(float(game.windowSize.width - numberOfShields * SHIELD_SPRITE_WIDTH)/float(numberOfShields+1));
	int xPadding = floor(float(game.windowSize.width - numberOfShields * SHIELD_SPRITE_WIDTH) / float(numberOfShields + 1));

	for (int i = 0; i < numberOfShields; i++)
	{
		Shield& shield = shields[i];
		shield.position.x = firstPadding + i * (SHIELD_SPRITE_WIDTH + xPadding);
		shield.position.y = game.windowSize.height - PLAYER_SPRITE_HEIGHT - 1 - SHIELD_SPRITE_HEIGHT - 2;

		for (int row = 0; row < SHIELD_SPRITE_HEIGHT; row++)
		{
			shield.sprite[row] = new char[SHIELD_SPRITE_WIDTH + 1];
			strcpy_s(shield.sprite[row], SHIELD_SPRITE_WIDTH + 1, SHIELD_SPRITE[row]);
		}
	}
}

void CleanUpShields(Shield shields[], int numberOfShields)
{
	for (int i = 0; i < numberOfShields; i++)
	{
		Shield& shield = shields[i];

		for (int row = 0; row < SHIELD_SPRITE_HEIGHT; row++) 
		{
			delete[] shield.sprite[row];
		}
	}
}

bool ShouldShootBomb(const AlienSwarm& aliens)
{
	return int(rand() % (70 - int(float(NUM_ALIEN_ROWS * NUM_ALIEN_COLS) / 
								  float(aliens.numAliensLeft+1)))) == 1;
}

void ShootBomb(AlienSwarm& aliens, int columnToShoot)
{
	int bombId = NOT_IN_PLAY;

	for (int i = 0; i < MAX_NUMBER_ALIEN_BOMBS; i++)
	{
		if (aliens.bombs[i].position.x == NOT_IN_PLAY || aliens.bombs[i].position.y == NOT_IN_PLAY)
		{
			bombId = i;
			break;
		}
	}

	for (int r = NUM_ALIEN_ROWS - 1; r >= 0; r--)
	{
		if (aliens.aliens[r][columnToShoot] == AS_ALIVE)
		{
			int x = aliens.position.x + columnToShoot * 
				(aliens.spriteSize.width + ALIENS_X_PADDING) + 1; // roughly middle of the alien
			int y = aliens.position.y + r * 
				(aliens.spriteSize.height + ALIENS_Y_PADDING) + aliens.spriteSize.height; // bottom of alien

			aliens.bombs[bombId].animation = 0;
			aliens.bombs[bombId].position.x = x;
			aliens.bombs[bombId].position.y = y;
			
			aliens.numberOfBombsInPlay++;
			
			break;
		}
	}
}

bool UpdateBombs(const Game& game, AlienSwarm& aliens, Player& player, Shield shields[], int numberOfShields)
{
	int numBombSprites = strlen(ALIEN_BOMB_SPRITE);

	for (int i = 0; i < MAX_NUMBER_ALIEN_BOMBS; i++)
	{
		if (aliens.bombs[i].position.x != NOT_IN_PLAY && aliens.bombs[i].position.y != NOT_IN_PLAY)
		{
			aliens.bombs[i].position.y += ALIEN_BOMB_SPEED;
			aliens.bombs[i].animation = (aliens.bombs[i].animation + 1) % numBombSprites;

			Position collisionPoint;
			int shieldIndex= IsCollision(aliens.bombs[i].position, shields, numberOfShields, collisionPoint);

			if (shieldIndex != NOT_IN_PLAY)
			{
				aliens.bombs[i].position.x = NOT_IN_PLAY;
				aliens.bombs[i].position.y = NOT_IN_PLAY;
				aliens.bombs[i].animation = 0;
				aliens.numberOfBombsInPlay--;
				ResolveShieldCollision(shields, shieldIndex, collisionPoint);
			}
			else if (IsCollision(aliens.bombs[i].position, player.position, player.spriteSize))
			{
				aliens.bombs[i].position.x = NOT_IN_PLAY;
				aliens.bombs[i].position.y = NOT_IN_PLAY;
				aliens.bombs[i].animation = 0;
				aliens.numberOfBombsInPlay--;
				return true;
			}
			else if (aliens.bombs[i].position.y >= game.windowSize.height)
			{
				aliens.bombs[i].position.x = NOT_IN_PLAY;
				aliens.bombs[i].position.y = NOT_IN_PLAY;
				aliens.bombs[i].animation = 0;
				aliens.numberOfBombsInPlay--;
			}
		}
	}
}


void DrawShields(const Shield shields[], int numberOfShields)
{
for (int i = 0; i < numberOfShields; i++)
{
	const Shield shield = shields[i];

	DrawSprite(shield.position.x, shield.position.y, (const char**)shield.sprite, SHIELD_SPRITE_HEIGHT);
}
}

int IsCollision(const Position& projectile, const Shield shields[], int numberOfShields, Position& shieldCollidePoint)
{
	shieldCollidePoint.x = NOT_IN_PLAY;
	shieldCollidePoint.y = NOT_IN_PLAY;

	if (projectile.y != NOT_IN_PLAY)
	{
		for (int i = 0; i < numberOfShields; i++)
		{
			const Shield& shield = shields[i];

			if (
				// if the projectile is within the shield's x boundaries
				projectile.x >= shield.position.x && projectile.x < (shield.position.x + SHIELD_SPRITE_WIDTH) &&
				// and within the y boundaries
				projectile.y >= shield.position.y && projectile.y < (shield.position.y + SHIELD_SPRITE_HEIGHT) &&
				// and the character there isn't a space char
				shield.sprite[projectile.y - shield.position.y][projectile.x - shield.position.x] != ' '
				)
			{
				// then there's a collision
				shieldCollidePoint.x = projectile.x - shield.position.x;
				shieldCollidePoint.y = projectile.y - shield.position.y;
				return i;
			}
		}
	}

	return NOT_IN_PLAY;
}

bool IsCollision(const Player& player, const AlienSwarm& aliens, Position& alienCollidePositionInArray)
{
	alienCollidePositionInArray.x = NOT_IN_PLAY;
	alienCollidePositionInArray.y = NOT_IN_PLAY;

	for (int row = 0; row < NUM_ALIEN_ROWS; row++)
	{
		for (int col = 0; col < NUM_ALIEN_COLS; col++)
		{
			int x = aliens.position.x + col * (aliens.spriteSize.width + ALIENS_X_PADDING);
			int y = aliens.position.y + row * (aliens.spriteSize.height + ALIENS_Y_PADDING);

			if (aliens.aliens[row][col] == AS_ALIVE &&
				player.missile.x >= x && player.missile.x < x + aliens.spriteSize.width &&
				player.missile.y >= y && player.missile.y < y + aliens.spriteSize.height)
			{
				alienCollidePositionInArray.x = col;
				alienCollidePositionInArray.y = row;
				return true;
			}
		}
	}

	return false;
}

bool IsCollision(const Position& projectile, const Position& spritePosition, const Size& spriteSize)
{
	return (projectile.x >= spritePosition.x && projectile.x < (spritePosition.x + spriteSize.width) &&
		projectile.y >= spritePosition.y && projectile.y < (spritePosition.y + spriteSize.height));
}

void ResolveShieldCollision(Shield shields[], int shieldIndex, const Position& shieldCollidePoint)
{
	shields[shieldIndex].sprite[shieldCollidePoint.y][shieldCollidePoint.x] = ' ';
}

int ResolveAlienCollision(AlienSwarm& aliens, const Position& hitPositionInAliensArray)
{
	aliens.aliens[hitPositionInAliensArray.y][hitPositionInAliensArray.x] = AS_EXPLODING;
	aliens.numAliensLeft--;

	if (aliens.explosionTimer == NOT_IN_PLAY)
	{
		aliens.explosionTimer = ALIENS_EXPLOSION_TIME;
	}

	if (hitPositionInAliensArray.y == 0)
	{
		return 30;
	}
	else if (hitPositionInAliensArray.y >= 1 && hitPositionInAliensArray.y < 3)
	{
		return 20;
	}
	else
	{
		return 10;
	}
}

void DestroyShields(const AlienSwarm& aliens, Shield shields[], int numberOfShields)
{
	for (int row = 0; row < NUM_ALIEN_ROWS; row++)
	{
		for (int col = 0; col < NUM_ALIEN_COLS; col++)
		{
			if (aliens.aliens[row][col] == AS_ALIVE)
			{
				int x = aliens.position.x + col * (aliens.spriteSize.width + ALIENS_X_PADDING);
				int y = aliens.position.y + row * (aliens.spriteSize.height + ALIENS_Y_PADDING);

				CollideShieldsWithAlien(shields, numberOfShields, x, y, aliens.spriteSize);
			}
		}
	}
}

void InitAliens(const Game& game, AlienSwarm& aliens)
{
	for (int row = 0; row < NUM_ALIEN_ROWS; row++)
	{
		for (int col = 0; col < NUM_ALIEN_COLS; col++)
		{
			aliens.aliens[row][col] = AS_ALIVE;
		}
	}

	ResetMovementTime(aliens);

	aliens.direction = 1; // going to the right
	aliens.numAliensLeft = NUM_ALIEN_ROWS * NUM_ALIEN_COLS;
	aliens.animation = 0;
	aliens.spriteSize.width = ALIEN_SPRITE_WIDTH;
	aliens.spriteSize.height = ALIEN_SPRITE_HEIGHT;
	aliens.numberOfBombsInPlay = 0;
	aliens.position.x = (game.windowSize.width - NUM_ALIEN_COLS * (aliens.spriteSize.width + ALIENS_X_PADDING))/2;
	aliens.position.y = game.windowSize.height - NUM_ALIEN_COLS -
		NUM_ALIEN_ROWS * aliens.spriteSize.height - ALIENS_Y_PADDING * (NUM_ALIEN_ROWS - 1) - 3 + game.level;
	aliens.line = NUM_ALIEN_COLS - (game.level - 1);
	aliens.explosionTimer = NOT_IN_PLAY;

	for (int i = 0; i < MAX_NUMBER_ALIEN_BOMBS; i++)
	{
		aliens.bombs[i].animation = 0;
		aliens.bombs[i].position.x = NOT_IN_PLAY;
		aliens.bombs[i].position.y = NOT_IN_PLAY;
	}
}

void DrawAliens(const AlienSwarm& aliens)
{
	const int NUM_30_POINT_ALIEN_ROWS = 1;
	// draw one row of 30 point aliens
	for (int col = 0; col < NUM_ALIEN_COLS; col++)
	{
		int x = aliens.position.x + col * (aliens.spriteSize.width + ALIENS_X_PADDING);
		int y = aliens.position.y;

		if (aliens.aliens[0][col] == AS_ALIVE)
		{
			DrawSprite(x, y, ALIEN30_SPRITE, aliens.spriteSize.height, aliens.animation * aliens.spriteSize.height);
		}
		else if (aliens.aliens[0][col] == AS_EXPLODING)
		{
			DrawSprite(x, y, ALIEN_EXPLOSION, aliens.spriteSize.height);
		}
	}

	// draw two rows of 20 point aliens
	const int NUM_20_POINT_ALIEN_ROWS = 2;
	for (int row = 0; row < NUM_20_POINT_ALIEN_ROWS; row++)
	{
		for (int col = 0; col < NUM_ALIEN_COLS; col++)
		{
			int x = aliens.position.x + col * (aliens.spriteSize.width + ALIENS_X_PADDING);
			int y = aliens.position.y + row * (aliens.spriteSize.height + ALIENS_Y_PADDING) + 
				NUM_30_POINT_ALIEN_ROWS * (aliens.spriteSize.height + ALIENS_Y_PADDING);

			if (aliens.aliens[NUM_30_POINT_ALIEN_ROWS + row][col] == AS_ALIVE)
			{
				DrawSprite(x, y, ALIEN20_SPRITE, aliens.spriteSize.height, aliens.animation * aliens.spriteSize.height);
			}
			else if (aliens.aliens[NUM_30_POINT_ALIEN_ROWS + row][col] == AS_EXPLODING)
			{
				DrawSprite(x, y, ALIEN_EXPLOSION, aliens.spriteSize.height);
			}
		}
	}
	// draw two rows of 10 point aliens
	const int NUM_10_POINT_ALIEN_ROWS = 2;
	for (int row = 0; row < NUM_10_POINT_ALIEN_ROWS; row++)
	{
		for (int col = 0; col < NUM_ALIEN_COLS; col++)
		{
			int x = aliens.position.x + col * (aliens.spriteSize.width + ALIENS_X_PADDING);
			int y = aliens.position.y + row * (aliens.spriteSize.height + ALIENS_Y_PADDING) +
				NUM_30_POINT_ALIEN_ROWS * (aliens.spriteSize.height + ALIENS_Y_PADDING) +
				NUM_20_POINT_ALIEN_ROWS * (aliens.spriteSize.height + ALIENS_Y_PADDING);

			if (aliens.aliens[NUM_30_POINT_ALIEN_ROWS + NUM_20_POINT_ALIEN_ROWS + row][col] == AS_ALIVE)
			{
				DrawSprite(x, y, ALIEN10_SPRITE, aliens.spriteSize.height, aliens.animation * aliens.spriteSize.height);
			}
			else if (aliens.aliens[NUM_30_POINT_ALIEN_ROWS + NUM_20_POINT_ALIEN_ROWS + row][col] == AS_EXPLODING)
			{
				DrawSprite(x, y, ALIEN_EXPLOSION, aliens.spriteSize.height);
			}
		}
	}

	if (aliens.numberOfBombsInPlay > 0)
	{
		for (int i = 0; i < MAX_NUMBER_ALIEN_BOMBS; i++)
		{
			if (aliens.bombs[i].position.x != NOT_IN_PLAY && aliens.bombs[i].position.y != NOT_IN_PLAY)
			{
				DrawCharacter(aliens.bombs[i].position.x, aliens.bombs[i].position.y, 
					ALIEN_BOMB_SPRITE[aliens.bombs[i].animation]);
			}
		}
	}
}
