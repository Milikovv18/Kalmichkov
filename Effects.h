#pragma once

#include "Snake.h"

typedef std::vector<std::pair<Pos, char>> objectsArray;

/*
 * 2 - Hunger
 * 3 - Blindness
 * 4 - Destroyer
 * 5 - Undead
 * 6 - Slowness
**/


class Effects
{
private:
	struct Effect
	{
		int timer{ 0 };
		Drawing cnv{ 200, 50, false };

		Effect(int offset)
		{
			cnv.align(Aligns::LEFT, Aligns::BOTTOM, 15, offset * 50 + 65);
		}
	};

	struct Particle
	{
		int timer{ 0 };
		Pos pos{ 0, 0 };
		Pos speed{ 0, 0 };
		int size{ 0 };
		COLORREF color{ 0 };
	};

	inline static Effect m_hunger{ 0 };
	inline static Effect m_blindness{ 1 };
	inline static Effect m_destroyer{ -1 };
	inline static Effect m_undead{ 2 };
	inline static Effect m_slowness{ 3 };
	inline static bool m_particles{ false };
	inline static std::vector<Particle> m_partsArr;

public:
	enum class Types
	{
		HUNGER = '3',
		BLINDNESS = '4',
		DESTROYER = '5',
		UNDEAD = '6',
		SLOWNESS = '7',
		PARTICLES = '8'
	};


	static void addEffect(Types effect)
	{
		switch (effect)
		{
		case Types::HUNGER:
			m_hunger.timer += 60;
			m_hunger.cnv.loadResource(BMP_HUNGER);
			m_hunger.cnv.draw();
			break;
		case Types::BLINDNESS:
			m_blindness.timer += 50;
			m_blindness.cnv.loadResource(BMP_BLINDNESS);
			m_blindness.cnv.draw();
			break;
		case Types::DESTROYER:
			m_destroyer.timer = 1; // No "+=", bc one time effect
			break;
		case Types::UNDEAD:
			m_undead.timer += 300;
			m_undead.cnv.loadResource(BMP_UNDEAD);
			m_undead.cnv.draw();
			break;
		case Types::SLOWNESS:
			m_slowness.timer += 15;
			m_slowness.cnv.loadResource(BMP_SLOWNESS);
			m_slowness.cnv.draw();
			break;
		case Types::PARTICLES:
			m_particles = true;
			break;
		default:
			break;
		}
	}

	static void setOnlineEffects(const short effectsToSet[2])
	{
		// Only drawing
		if (effectsToSet[0] > 1) // Undead
		{
			m_undead.timer = -effectsToSet[0];
			m_undead.cnv.loadResource(BMP_UNDEAD);
			m_undead.cnv.draw();
		}
		else if (effectsToSet[0] == 1) {
			removeEffect(Types::UNDEAD);
		}
	}

	static void removeEffect(Types effect)
	{
		switch (effect)
		{
		case Types::HUNGER:
			m_hunger.timer = 0;
			m_hunger.cnv.loadResource(BMP_EMPTY);
			m_hunger.cnv.draw();
			break;
		case Types::BLINDNESS:
			m_blindness.timer = 0;
			m_blindness.cnv.loadResource(BMP_EMPTY);
			m_blindness.cnv.draw();
			break;
		case Types::DESTROYER:
			m_destroyer.timer = 0;
			break;
		case Types::UNDEAD:
			m_undead.timer = 0;
			m_undead.cnv.loadResource(BMP_EMPTY);
			m_undead.cnv.draw();
			break;
		case Types::SLOWNESS:
			m_slowness.timer = 0;
			m_slowness.cnv.loadResource(BMP_EMPTY);
			m_slowness.cnv.draw();
			break;
		case Types::PARTICLES:
			m_particles = false;
			break;
		default:
			break;
		}
	}


	static void update()
	{
		if (m_hunger.timer > 0) {
			if (!--m_hunger.timer)
				removeEffect(Types::HUNGER);
		}
		if (m_blindness.timer > 0) {
			if (!--m_blindness.timer)
				removeEffect(Types::BLINDNESS);
		}
		if (m_destroyer.timer > 0) {
			if (!m_destroyer.timer) // No "--", bc one time effect
				removeEffect(Types::DESTROYER);
		}
		if (m_undead.timer > 0) {
			if (!--m_undead.timer)
				removeEffect(Types::UNDEAD);
		}
		if (m_slowness.timer > 0) {
			if (!--m_slowness.timer)
				removeEffect(Types::SLOWNESS);
		}
	}


	static void perform(Drawing& permCnv,
		Drawing& cnvEffects,
		const Pos& playerHead,
		MapHandler& gameMap,
		double& minMovAccum,
		objectsArray& applesToSend,
		objectsArray& wallsToSend)
	{
		// nothing to perform when hunger
		if (m_blindness.timer > 0) doBlindness(cnvEffects, playerHead);
		if (m_destroyer.timer > 0) doDestroy(permCnv, gameMap, wallsToSend);
		if (m_undead.timer > 0) doUndead(permCnv, gameMap, applesToSend);
		if (m_slowness.timer > 0) doSlowness(permCnv, cnvEffects, minMovAccum);
		if (m_particles || m_partsArr.size()) doParticles(cnvEffects, playerHead);
	}


	static bool isStarving()
	{
		return !((m_hunger.timer + 1) % 5);
	}

	static bool isSlow()
	{
		return m_slowness.timer > 0;
	}

	static void doBlindness(Drawing& cnvEffects, Pos playerHead)
	{
		playerHead.x *= VOX_SIZE;
		playerHead.y *= VOX_SIZE;
		for (int y(0); y < cnvEffects.getHeight(); ++y)
			for (int x(0); x < cnvEffects.getWidth(); ++x)
				if (cnvEffects.getPixel(x, y) != RGB(255, 0, 0))
					cnvEffects.setPixel(x, y, RGB(0, 0, 0),
						float((pow(x - playerHead.x, 2) + pow(y - playerHead.y, 2)) / 2000));
	}

	static void doDestroy(Drawing& permCnv, MapHandler& gameMap, objectsArray& wallsToSend)
	{
		Pos holePos = gameMap.getFilledObstaclePlace();

		gameMap.placeObs(holePos, ' ');
		permCnv.rect(VOX_SIZE * holePos.x, VOX_SIZE * holePos.y, VOX_SIZE, RGB(0, 0, 0));
		wallsToSend.push_back(std::pair(holePos, ' '));

		if (!holePos.x || !holePos.y || holePos.x == MAP_WIDTH - 1 || holePos.y == MAP_HEIGHT - 1)
		{
			if (holePos.x == 0)
				holePos.x = MAP_WIDTH - 1;
			else if (holePos.x == MAP_WIDTH - 1)
				holePos.x = 0;

			if (holePos.y == 0)
				holePos.y = MAP_HEIGHT - 1;
			else if (holePos.y == MAP_HEIGHT - 1)
				holePos.y = 0;

			gameMap.placeObs(holePos, ' ');
			permCnv.rect(VOX_SIZE * holePos.x, VOX_SIZE * holePos.y, VOX_SIZE, RGB(0, 0, 0));
			wallsToSend.push_back(std::pair(holePos, ' '));
		}

		m_destroyer.timer = 0; // Immediately reseting effect
	}

	static void doUndead(Drawing& permCnv, MapHandler& gameMap, objectsArray& applesToSend)
	{
		static Pos curFoodPos(0, 0);
		if (!gameMap.isObject(curFoodPos, '2'))
		{
			curFoodPos = gameMap.findAnyObjectPos('2');
			if (curFoodPos.x == -1 && curFoodPos.y == -1)
				return;
		}

		std::random_device r;
		std::mt19937 gen{ r() };
		// All possible moves
		std::vector<Pos> matrixFoodPos{
			Pos(curFoodPos.x, curFoodPos.y - 1),
			Pos(curFoodPos.x - 1, curFoodPos.y),
			curFoodPos,
			Pos(curFoodPos.x + 1, curFoodPos.y),
			Pos(curFoodPos.x, curFoodPos.y + 1)
		};

		// Randomly choosing new place
		Pos newFoodPos;
		do {
			// Every time new dist, because vector is shorter
			std::uniform_int_distribution<> uidFoodMatrixId(0, int(matrixFoodPos.size() - 1));
			unsigned idx = uidFoodMatrixId(gen);
			// Fix if new pos is out of bounds
			newFoodPos.x = matrixFoodPos[idx].x % MAP_WIDTH;
			newFoodPos.y = matrixFoodPos[idx].y % MAP_HEIGHT;
			if (newFoodPos.x < 0) newFoodPos.x += MAP_WIDTH;
			if (newFoodPos.y < 0) newFoodPos.y += MAP_HEIGHT;
			// If place isnt an obstacle, exit
			if (!gameMap.isObstacle(newFoodPos) &&
				!gameMap.isObject(newFoodPos, '0') &&
				!gameMap.isObject(newFoodPos, '1'))
				break;
			matrixFoodPos.erase(matrixFoodPos.begin() + idx);
		} while (matrixFoodPos.size());

		if (!matrixFoodPos.size())
			return;

		// Swap 2 objects if there are some
		char prev = gameMap.placeObj(newFoodPos, '2');
		gameMap.placeObj(curFoodPos, prev);
		Effects::drawEffectObjs(permCnv, curFoodPos, prev);

		// Get ready to send
		applesToSend.push_back(std::pair(curFoodPos, ' '));
		applesToSend.push_back(std::pair(newFoodPos, '2'));

		// Drawing changes
		permCnv.rect(VOX_SIZE * curFoodPos.x, VOX_SIZE * curFoodPos.y, VOX_SIZE, RGB(0, 0, 0));
		permCnv.circle(VOX_SIZE * newFoodPos.x, VOX_SIZE * newFoodPos.y, VOX_SIZE / 2, RGB(255, 0, 0));

		curFoodPos = newFoodPos;
	}

	static void doSlowness(const Drawing& permCnv, Drawing& cnvEffects, double& minMovAccum)
	{
		minMovAccum = m_slowness.timer / 10.0;
		for (int y(0); y < cnvEffects.getHeight(); ++y)
		{
			int offsetX = rand() % m_slowness.timer - m_slowness.timer / 2;
			int offsetY = rand() % m_slowness.timer - m_slowness.timer / 2;
			for (int x(0); x < cnvEffects.getWidth(); ++x)
				cnvEffects.setPixel(x + offsetX, y + offsetY, permCnv.getPixel(x, y));
		}
	}


	static void doParticles(Drawing& cnvEffects, Pos playerHead)
	{
		const int partLifeTime{ 6 };

		// Update
		for (int i(0); i < m_partsArr.size(); ++i)
		{
			if (!m_partsArr[i].timer)
			{
				m_partsArr.erase(m_partsArr.begin() + i--);
				continue;
			}

			m_partsArr[i].pos.x += m_partsArr[i].speed.x;
			m_partsArr[i].pos.y += m_partsArr[i].speed.y;
			m_partsArr[i].speed.x = int(m_partsArr[i].speed.x * 0.9);
			m_partsArr[i].speed.y = int(m_partsArr[i].speed.y * 0.9);
			m_partsArr[i].timer--;
			cnvEffects.rect(m_partsArr[i].pos.x, m_partsArr[i].pos.y, m_partsArr[i].size, m_partsArr[i].color, 0.8f);
		}

		std::random_device r;
		std::mt19937 gen{ r() };
		std::uniform_int_distribution<> uidSpeed(-8, 8);
		std::uniform_int_distribution<> uidColor(127, 255);
		std::uniform_int_distribution<> uidLifeTime(1, partLifeTime);
		std::uniform_int_distribution<> uidSize(1, 4);

		playerHead.x *= VOX_SIZE;
		playerHead.y *= VOX_SIZE;

		// Create
		while (m_particles && m_partsArr.size() <= 10)
		{
			Particle newPart{ uidLifeTime(gen), playerHead, Pos(uidSpeed(gen),
				uidSpeed(gen)), uidSize(gen), RGB(255 - uidColor(gen), uidColor(gen), 255 - uidColor(gen)) };
			m_partsArr.push_back(newPart);
		}
	}


	static std::wstring getStatusString()
	{
		std::wstring stat = L"";

		if (m_hunger.timer)
			stat += L", Hunger " + std::to_wstring(abs(m_hunger.timer));
		if (m_blindness.timer)
			stat += L", Blindness " + std::to_wstring(abs(m_blindness.timer));
		if (m_destroyer.timer)
			stat += L", Destroyer " + std::to_wstring(abs(m_destroyer.timer));
		if (m_undead.timer)
			stat += L", Undead " + std::to_wstring(abs(m_undead.timer));
		if (m_slowness.timer)
			stat += L", Slowness " + std::to_wstring(abs(m_slowness.timer));

		return stat;
	}


	static COLORREF getEffectColor(Types effect)
	{
		return RGB(255 / ((int)effect - '2'), 255 / ((int)effect - '2'), 0);
	}


	static void getOnlineEffects(short effectsToGet[2])
	{
		effectsToGet[0] = (short)m_undead.timer;
		effectsToGet[1] = m_particles;
	}


	static void drawEffectObjs(Drawing& cnv, const Pos& pos, char marker)
	{
		switch (marker)
		{
		case '2':
			cnv.circle(VOX_SIZE * pos.x, VOX_SIZE * pos.y, VOX_SIZE / 2, RGB(255, 0, 0));
			break;
		case ' ':
			cnv.rect(VOX_SIZE * pos.x, VOX_SIZE * pos.y, VOX_SIZE, RGB(0, 0, 0));
			break;
		default:
			cnv.circle(VOX_SIZE * pos.x, VOX_SIZE * pos.y, VOX_SIZE / 2, getEffectColor(Effects::Types(marker)));
			break;
		}
	}
};