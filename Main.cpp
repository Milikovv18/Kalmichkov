// Файл Drawing.h содержит графический движок от Milikovv.
//
#define WIN32_LEAN_AND_MEAN

#include <conio.h>

#include "resource.h"
#include "Drawing.h"
#include "Qlmichkov.h"

#include "Maps.h"
#include "Snake.h"
#include "Effects.h"
#include "Network.h"
#include "Enemy.h"

using namespace std::chrono;

constexpr int winRate = 192;
constexpr double TIME_TICK = 0.08; // Квант времени
constexpr double DEFAULT_MIN_MOV = 0.16;

Network::Type showMenu();
void youLost(Drawing &pic);
void youWin(Drawing& pic);

double minMovAccum(DEFAULT_MIN_MOV); // Минимальная граница разрешения движения

/*
 * 2 - Hunger
 * 3 - Blindness
 * 4 - Destroyer
 * 5 - Undead
 * 6 - Slowness
**/


static objectsArray applesToSend, wallsToSend;
void genFoodObject(Drawing &cnv, MapHandler& gameMap)
{
	std::random_device r;
	std::mt19937 gen{ r() };
	std::uniform_int_distribution<> uidEffects(1, 100);

	// Generating food
	Pos foodPos = gameMap.getEmptyPlace();
	gameMap.placeObj(foodPos, '2');
	applesToSend.push_back(std::pair(foodPos, '2'));

	cnv.circle(VOX_SIZE * foodPos.x, VOX_SIZE * foodPos.y, VOX_SIZE / 2, RGB(255, 0, 0));

	int guess(uidEffects(gen));
	if (guess < 80)
		return;

	// Generating bonuses
	Pos bonusPos = gameMap.getEmptyPlace();

	Effects::Types typ;

	if (guess < 85) {
		typ = Effects::Types::HUNGER;
	}
	else if (guess < 90) {
		typ = Effects::Types::BLINDNESS;
	}
	else if (guess < 95) {
		typ = Effects::Types::DESTROYER;
	}
	else if (guess < 99) {
		typ = Effects::Types::UNDEAD;
	}
	else {
		typ = Effects::Types::SLOWNESS;
	}

	gameMap.placeObj(bonusPos, (char)typ);
	COLORREF effectColor = Effects::getEffectColor(typ);
	applesToSend.push_back(std::pair(bonusPos, (char)typ));

	cnv.circle(VOX_SIZE * bonusPos.x, 
		VOX_SIZE * bonusPos.y, VOX_SIZE / 2, effectColor);

	return;
}


void keyPress(Snake& snake)
{
	bool keysPressed[3]{};
	while (true)
	{
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			keysPressed[1] = false;
			if (!keysPressed[0])
			{
				snake.turnLeft();
				keysPressed[0] = true;
			}
		}
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			keysPressed[0] = false;
			if (!keysPressed[1])
			{
				snake.turnRight();
				keysPressed[1] = true;
			}
		}
		else
		{
			keysPressed[0] = false;
			keysPressed[1] = false;
		}

		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			if (!keysPressed[2] && !Effects::isSlow())
			{
				minMovAccum = TIME_TICK;
				keysPressed[2] = true;
				Effects::addEffect(Effects::Types::PARTICLES);
			}
		}
		else if (!Effects::isSlow())
		{
			minMovAccum = DEFAULT_MIN_MOV;
			keysPressed[2] = false;
			Effects::removeEffect(Effects::Types::PARTICLES);
		}
	}
}


#ifdef _SERVER
#define N_TYPE SERVER
#elif _CLIENT
#define N_TYPE CLIENT
#elif _DEBUG
#define _CLIENT
#define N_TYPE CLIENT // Debug only
#endif // !N_TYPE


int main()
{
	//// Showing menu ////
#ifndef _DEBUG
	const Network::Type curGameType = showMenu();
#else
	const Network::Type curGameType = Network::Type::N_TYPE;
#endif

	//// Setting up network and waiting for connections ////
	Network::startup(curGameType, "172.18.21.77");
	Pos startPos;
	if (Network::type() == Network::Type::SERVER)
		startPos = Pos(MAP_WIDTH / 5, MAP_HEIGHT / 3);
	else
		startPos = Pos(4 * MAP_WIDTH / 5, 2 * MAP_HEIGHT / 3);

	//// Creating map ////
	MapHandler* gameMap = new MapHandler(map_samples::mapDefault);

	//// Setting up graphics ////
	// Header
	Drawing cnvHead(700, 150, false);
	cnvHead.align(Aligns::CENTER, Aligns::TOP, 0, 5);
	cnvHead.loadResource(BMP_HEADER);
	cnvHead.draw();

	// Walls map
	Drawing cnvMap(CNV_WIDTH, CNV_HEIGHT, false);
	cnvMap.align(Aligns::CENTER, Aligns::BOTTOM, 0, 15);
	cnvMap.fill(RGB(0, 0, 0));

	// Effects map
	Drawing cnvVisualEffects(CNV_WIDTH, CNV_HEIGHT, false);
	cnvVisualEffects.align(Aligns::CENTER, Aligns::BOTTOM, 0, 15);

	for (int x(0); x < MAP_WIDTH; ++x)
		for (int y(0); y < MAP_HEIGHT; ++y)
			if (gameMap->isObstacle(x, y))
			{
				double randGray = 255 * (0.3 * rand() / RAND_MAX + 0.7);
				cnvMap.rect(VOX_SIZE * x, VOX_SIZE * y, VOX_SIZE, RGB(randGray, randGray, randGray));
			}

	// Creating player snake and enemy
	Snake snake(startPos, Pos(1, 0));
	Enemy enemy;

	// Detecting keys presses thread
	std::thread keysThread(keyPress, std::ref(snake));
	keysThread.detach();

	// Generating first food
	genFoodObject(cnvMap, *gameMap);

	unsigned eaten(0), fitness(0);
	double curMovAccum(0);
	bool firstPackSent(false), ate(false);
	high_resolution_clock::time_point curT, prevT = high_resolution_clock::now();
	while (true)
	{
		ate = false;

		// Movements
		if (curMovAccum >= minMovAccum)
		{
			if (snake.moveAndEat(*gameMap))
			{
				genFoodObject(cnvMap, *gameMap);
				ate = true;
				++eaten;
				curMovAccum = 0;
			}
			Effects::update();
			curMovAccum = 0.0;
			++fitness;
		}

		if (snake.isDead(*gameMap))
			break;

		if (Network::isConnected())
		{
			if (!firstPackSent)
			{
				if (Network::type() == Network::Type::SERVER) {
					snake.sendMap(*gameMap);
				}
				else {
					enemy.getMap(cnvMap, *gameMap);
					applesToSend.clear();
					snake.sendStat(ate, applesToSend, wallsToSend);
				}
				firstPackSent = true;
			}
			else // Every time but the first one
			{
				enemy.getStat(eaten, *gameMap);
				snake.sendStat(ate, applesToSend, wallsToSend);
			}
			enemy.redraw(cnvMap);
		}
		snake.redraw(cnvMap);

		// Progress and effects
		applesToSend.clear(); // Не перемещать!! Обновляется в perform и отсылается
		wallsToSend.clear();  // Не перемещать!! Обновляется в perform и отсылается
		cnvVisualEffects.setArray(cnvMap.getArray());
		Effects::perform(cnvMap, cnvVisualEffects, snake.getHead(),
			*gameMap, minMovAccum, applesToSend, wallsToSend);

		enemy.doParticles(cnvVisualEffects); // TODO: temporary solution

		SetConsoleTitle((L"Completed " + std::to_wstring(eaten * 100 / winRate) +
			L"%, Fitness " + std::to_wstring(fitness) + Effects::getStatusString()).c_str());

		cnvVisualEffects.draw();
		//system("cls");
		//gameMap->printFull();

		gameMap->convertTempObjs();

		if (Network::type() == Network::Type::SERVER)
		{
			do {
				curT = high_resolution_clock::now();
			} while (duration_cast<duration<float>>(curT - prevT).count() < TIME_TICK);
			prevT = high_resolution_clock::now();
		}
		curMovAccum += TIME_TICK;
	}

	// Finish (loss only)
	// TODO: win
	Network::shutdown();

	cnvHead.loadResource(BMP_GAMEOVER);
	cnvHead.draw();
	Sleep(5000);

	return 0;
}