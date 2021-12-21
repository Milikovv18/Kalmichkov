#pragma once

#include "Drawing.h"
#include "Qlmichkov.h"
#include "Maps.h"
#include "Effects.h"
#include "Network.h"

typedef std::vector<std::pair<Pos, char>> objectsArray;

class Snake
{
private:
	Form m_body;
	Pos m_curSpeed;
	Pos m_prevHead;
	std::vector<Pos> m_erasedTails;
	bool m_isDead{ false };
	COLORREF m_color;

public:
	Snake(const Pos& head, const Pos& speed)
	{
		InitForm(m_body);
		InsOne(m_body);
		m_body.head->pInf = new Info(head);

		m_curSpeed = speed;
		m_prevHead = Pos(-1, -1);
		m_color = RGB(0, 255, 0);
	}


	void turnLeft()
	{
		if (!m_curSpeed.x) {
			std::swap(m_curSpeed.x, m_curSpeed.y);
		}
		else
		{
			m_curSpeed = Pos(-m_curSpeed.x, -m_curSpeed.y);
			std::swap(m_curSpeed.x, m_curSpeed.y);
		}
	}

	void turnRight()
	{
		if (m_curSpeed.x) {
			std::swap(m_curSpeed.x, m_curSpeed.y);
		}
		else
		{
			m_curSpeed = Pos(-m_curSpeed.x, -m_curSpeed.y);
			std::swap(m_curSpeed.x, m_curSpeed.y);
		}
	}

	// Returns true if food has been eaten
	bool moveAndEat(MapHandler& gameMap)
	{
		Pos curPos = getHead();
		Pos nextPos((curPos.x + m_curSpeed.x) % MAP_WIDTH, (curPos.y + m_curSpeed.y) % MAP_HEIGHT);
		if (nextPos.x < 0) nextPos.x += MAP_WIDTH;
		if (nextPos.y < 0) nextPos.y += MAP_HEIGHT;

		InsEnd(m_body);
		m_body.last->pInf = new Info(nextPos);

		char prevObj = gameMap.placeObj(nextPos, '1', true);
		if (handleObject(prevObj))
			return true;

		// Check death from wall
		if (m_isDead || gameMap.isObstacle(nextPos))
		{
			m_isDead = true;
			return false;
		}

		// Shortening tail
		unsigned maxShortening = 2;
		do {
			Info* tailEndMem = DelBeg(m_body);
			if (!m_body.head)
			{
				m_isDead = true;
				return false;
			}

			m_erasedTails.push_back(tailEndMem->pos);
			gameMap.placeObj(tailEndMem->pos, ' ');
			delete tailEndMem;
		} while (--maxShortening && Effects::isStarving());

		return false;
	}


	void sendStat(bool& ate, objectsArray& applesToSend, objectsArray& wallsToSend)
	{
		bool moved = (getHead().x != m_prevHead.x) || (getHead().y != m_prevHead.y);
		short effectsToSend[2];
		Effects::getOnlineEffects(effectsToSend);

		bool sent = Network::sendData(ate, moved, getHead(), m_erasedTails,
			effectsToSend, applesToSend, wallsToSend);
		if (!sent) m_isDead = true;
	}

	void sendMap(const MapHandler& map)
	{
		bool sent = Network::sendMap(map);
		if (!sent) m_isDead = true;
	}


	bool isDead(const MapHandler& gameMap) const
	{
		return m_isDead;
	}

	Pos getSpeed() const
	{
		return m_curSpeed;
	}

	Pos getHead() const
	{
		return m_body.last->pInf->pos;
	}


	bool handleObject(char marker)
	{
		switch (marker)
		{
		case ' ':
			return false;
		case '0':
		case '1':
			m_isDead = true;
			return false;
		case '2':
			Effects::removeEffect(Effects::Types::UNDEAD);
			return true;
		default:
			Effects::addEffect(Effects::Types(marker));
		}
		return false;
	}


	void redraw(Drawing& cnv)
	{
		Pos head = getHead();
		cnv.rect(VOX_SIZE * m_prevHead.x + 1, VOX_SIZE * m_prevHead.y + 1, VOX_SIZE - 2, m_color);
		cnv.rect(VOX_SIZE * head.x + 1, VOX_SIZE * head.y + 1, VOX_SIZE - 2, m_color);
		cnv.rect(VOX_SIZE * head.x + 2, VOX_SIZE * head.y + 2, VOX_SIZE - 4, RGB(0, 127, 0));
		m_prevHead = head;

		while (m_erasedTails.size())
		{
			cnv.rect(VOX_SIZE * m_erasedTails.front().x,
				VOX_SIZE * m_erasedTails.front().y, VOX_SIZE, RGB(0, 0, 0));
			m_erasedTails.erase(m_erasedTails.begin());
		}
	}
};

