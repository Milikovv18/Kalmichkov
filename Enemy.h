#pragma once

#include "Qlmichkov.h"
#include "Drawing.h"
#include "Maps.h"
#include "Network.h"


class Enemy
{
private:
	Pos m_head{ -1, -1 };
	Pos m_prevHead{ -1, -1 };
	std::vector<Pos> m_erasedTails;
	objectsArray m_applesToRecv;
	objectsArray m_wallsToSend;
	MapHandler m_gameMap;

	struct Particle
	{
		int timer{ 0 };
		Pos pos{ 0, 0 };
		Pos speed{ 0, 0 };
		int size{ 0 };
		COLORREF color{ 0 };
	};

	COLORREF m_color;
	bool m_particles;
	std::vector<Particle> m_partsArr;

public:
	Enemy() : m_gameMap(map_samples::mapDefault)
	{
		m_prevHead = Pos(-1, -1);
		m_color = RGB(200, 0, 200);
		m_particles = false;
	}


	void getStat(unsigned& eaten, MapHandler& curGameMap)
	{
		bool moved, ate;
		short effectsToRecv[2]{}; // [undead, particles]
		bool recvd = Network::recvData(ate, moved, m_head, m_erasedTails,
			effectsToRecv, m_applesToRecv, m_wallsToSend);

		if (!recvd) {
			return;
		}

		// Check if ate food
		if (ate) {
			++eaten;
		}

		// Set new head
		if (moved) {
			curGameMap.placeObj(m_head, '1', true);
		}

		// Erase tails
		for (int i(0); i < m_erasedTails.size(); ++i) {
			curGameMap.placeObj(m_erasedTails[i], ' ');
		}

		// Set online effects
		for (int i(0); i < 2; ++i) {
			Effects::setOnlineEffects(effectsToRecv);
			m_particles = effectsToRecv[1];
		}

		// Set apples
		for (int i(0); i < m_applesToRecv.size(); ++i)
		{
			char marker = m_applesToRecv[i].second;
			curGameMap.placeObj(m_applesToRecv[i].first, marker);
		}

		// Break walls
		for (int i(0); i < m_wallsToSend.size(); ++i)
		{
			char marker = m_wallsToSend[i].second;
			curGameMap.placeObs(m_wallsToSend[i].first, marker);
		}
	}


	void getMap(Drawing& permCnv, MapHandler& curGameMap)
	{
		bool recvd = Network::recvMap(m_gameMap);

		for (int y(0); y < MAP_HEIGHT; ++y)
		{
			for (int x(0); x < MAP_WIDTH; ++x)
			{
				Pos pos(x, y);
				char obj = m_gameMap.getObject(pos);
				if (curGameMap.getObject(pos) == obj)
					continue;

				if (obj == '1')
					obj = ' ';

				curGameMap.placeObj(pos, obj);

				switch (obj)
				{
				case '0': // <-- Can never be, bc on the start snake head is '1'
				case '1':
					permCnv.rect(VOX_SIZE * x + 1, VOX_SIZE * y + 1, VOX_SIZE - 2, m_color);
					break;
				default:
					Effects::drawEffectObjs(permCnv, pos, obj);
				}
			}
		}
	}


	void redraw(Drawing& permCnv)
	{
		// Head and prevHead
		permCnv.rect(VOX_SIZE * m_prevHead.x + 1, VOX_SIZE * m_prevHead.y + 1, VOX_SIZE - 2, m_color);
		permCnv.rect(VOX_SIZE * m_head.x + 1, VOX_SIZE * m_head.y + 1, VOX_SIZE - 2, m_color);
		permCnv.rect(VOX_SIZE * m_head.x + 2, VOX_SIZE * m_head.y + 2, VOX_SIZE - 4, RGB(127, 0, 127));
		m_prevHead = m_head; // Обновление только после рисования

		// Erased tails
		while (m_erasedTails.size())
		{
			permCnv.rect(VOX_SIZE * m_erasedTails.front().x,
				VOX_SIZE * m_erasedTails.front().y, VOX_SIZE, RGB(0, 0, 0));
			m_erasedTails.erase(m_erasedTails.begin());
		}

		// Place new objects (apples)
		while (m_applesToRecv.size())
		{
			int x = m_applesToRecv.front().first.x;
			int y = m_applesToRecv.front().first.y;
			char marker = m_applesToRecv.front().second;

			permCnv.rect(VOX_SIZE * x, VOX_SIZE * y, VOX_SIZE, RGB(0, 0, 0));
			Effects::drawEffectObjs(permCnv, m_applesToRecv.front().first, marker);

			m_applesToRecv.erase(m_applesToRecv.begin());
		}

		// Beak some walls
		while (m_wallsToSend.size())
		{
			int x = m_wallsToSend.front().first.x;
			int y = m_wallsToSend.front().first.y;
			char marker = m_wallsToSend.front().second;

			switch (marker)
			{
			case '0':
				permCnv.rect(VOX_SIZE * x, VOX_SIZE * y, VOX_SIZE, RGB(255, 255, 255));
				break;
			case ' ':
				permCnv.rect(VOX_SIZE * x, VOX_SIZE * y, VOX_SIZE, RGB(0, 0, 0));
				break;
			default:
				break;
			}

			m_wallsToSend.erase(m_wallsToSend.begin());
		}
	}


	// TODO: Do something with that!!!
	void doParticles(Drawing& cnvEffects)
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

		Pos tempHead(m_head.x * VOX_SIZE, m_head.y * VOX_SIZE);

		// Create
		while (m_particles && m_partsArr.size() <= 10)
		{
			Particle newPart{ uidLifeTime(gen), tempHead, Pos(uidSpeed(gen),
				uidSpeed(gen)), uidSize(gen), RGB(uidColor(gen), 255 - uidColor(gen), uidColor(gen)) };
			m_partsArr.push_back(newPart);
		}
	}
};