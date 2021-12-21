#pragma once

#include <queue>
#include <random>
#include <iostream>

constexpr int MAP_WIDTH = 50;
constexpr int MAP_HEIGHT = 30;

constexpr int VOX_SIZE = 10;
constexpr int CNV_WIDTH = VOX_SIZE * MAP_WIDTH;
constexpr int CNV_HEIGHT = VOX_SIZE * MAP_HEIGHT;


namespace map_samples
{
	inline char mapDefault[MAP_HEIGHT][MAP_WIDTH + 1]
	{
		"00 00000000000000000000000000000000000000000000000",
		"0                                                0",
		"0                                                0",
		"0                                                0",
		"0                                                0",
		"           0000                    0000           ",
		"0          0000                    0000          0",
		"0          0000                    0000          0",
		"0          0000                    0000          0",
		"0                                                0",
		"0                                                0",
		"0                                                0",
		"0                                                0",
		"0                                                0",
		"000000000        0000000000000000        000000000",
		"0                                                0",
		"0                                                0",
		"0                                                0",
		"0                                                0",
		"0                                                0",
		"0                                                0",
		"0        0000000000000      0000000000000        0",
		"0                    0      0                    0",
		"0                    0      0                    0",
		"0                    0      0                    0",
		"0                    0      0                    0",
		"0                    0      0                    0",
		"0                    0      0                    0",
		"0                    0      0                    0",
		"00 00000000000000000000000000000000000000000000000"
	};
};


class MapHandler
{
private:
	char m_landscape[MAP_HEIGHT][MAP_WIDTH + 1]{};
	char m_objects[MAP_HEIGHT][MAP_WIDTH + 1]{};
	std::queue<Pos> tempObjs;

	// Random map position
	std::random_device r;
	std::mt19937 gen;
	std::uniform_int_distribution<> uidX{ 0, 49 };
	std::uniform_int_distribution<> uidY{ 0, 29 };

public:
	MapHandler(char map[MAP_HEIGHT][MAP_WIDTH + 1])
	{
		memcpy(m_landscape, map, MAP_HEIGHT * (MAP_WIDTH + 1));
		memset(m_objects, ' ', MAP_HEIGHT * (MAP_WIDTH + 1));

		gen.seed(r());
	}


	// Object detector
	bool isObstacle(unsigned x, unsigned y) const
	{
		return m_landscape[y][x] == '0';
	}
	bool isObstacle(const Pos& pos) const { return isObstacle(pos.x, pos.y); }

	bool isObject(unsigned x, unsigned y, char marker = '\0') const
	{
		if (marker == '\0')
			return m_objects[y][x] != ' ';
		return m_objects[y][x] == marker;
	}
	bool isObject(const Pos& pos, char marker = '\0') const { return isObject(pos.x, pos.y, marker); }


	// Object getter
	char getObstacle(unsigned x, unsigned y) const
	{
		return m_landscape[y][x];
	}
	char getObstacle(const Pos& pos) const { return getObstacle(pos.x, pos.y); }

	char getObject(unsigned x, unsigned y) const
	{
		return m_objects[y][x];
	}
	char getObject(const Pos& pos) const { return getObject(pos.x, pos.y); }


	// Returns previously set marker
	char placeObj(const Pos& objPos, char marker, bool tempObj = false)
	{
		if (tempObj)
			tempObjs.push(objPos);

		char toRet = m_objects[objPos.y][objPos.x];
		m_objects[objPos.y][objPos.x] = marker;
		return toRet;
	}

	void convertTempObjs()
	{
		while (tempObjs.size())
		{
			Pos tempObj = tempObjs.front();
			m_objects[tempObj.y][tempObj.x] = '0';
			tempObjs.pop();
		}
	}

	// Returns previously set marker
	char placeObs(const Pos& obsPos, char marker)
	{
		char toRet = m_landscape[obsPos.y][obsPos.x];
		m_landscape[obsPos.y][obsPos.x] = marker;
		return toRet;
	}


	//// GET EMPTY PLACES ////
	// When (false, false) returns pure random place on a map
	Pos getEmptyPlace(bool checkObs = true, bool checkObj = true)
	{
		Pos emptyPlace;
		do {
			emptyPlace.x = uidX(gen);
			emptyPlace.y = uidY(gen);
		} while ((checkObs && isObstacle(emptyPlace)) || (checkObj && isObject(emptyPlace)));
		return emptyPlace;
	}

	Pos getEmptyObjectPlace()
	{
		return getEmptyPlace(false, true);
	}

	Pos getEmptyObstaclePlace()
	{
		return getEmptyPlace(true, false);
	}


	//// GET FILLED PLACES ////
	Pos getFilledPlace(bool checkObs = true, bool checkObj = true)
	{
		Pos emptyPlace;
		do {
			emptyPlace.x = uidX(gen);
			emptyPlace.y = uidY(gen);
		} while ((checkObs && !isObstacle(emptyPlace)) || (checkObj && !isObject(emptyPlace)));
		return emptyPlace;
	}

	Pos getFilledObjectPlace()
	{
		return getFilledPlace(false, true);
	}

	Pos getFilledObstaclePlace()
	{
		return getFilledPlace(true, false);
	}


	//// PRINTS ////
	void printWalls()
	{
		for (int y(0); y < MAP_HEIGHT; ++y)
		{
			for (int x(0); x < MAP_WIDTH; ++x)
				std::cout << (isObstacle(x, y) ? '#' : ' ');
			std::cout << std::endl;
		}
	}

	void printObjects()
	{
		for (int y(0); y < MAP_HEIGHT; ++y)
		{
			for (int x(0); x < MAP_WIDTH; ++x)
				std::cout << getObject(x, y);
			std::cout << std::endl;
		}
	}

	void printFull()
	{
		for (int y(0); y < MAP_HEIGHT; ++y)
		{
			for (int x(0); x < MAP_WIDTH; ++x)
			{
				Pos pos(x, y);
				if (isObject(pos)) {
					std::cout << getObject(pos);
				}
				else if (isObstacle(pos)) {
					std::cout << getObstacle(pos);
				}
				else {
					std::cout << ' ';
				}
			}
			std::cout << std::endl;
		}
	}

	//// SEARCH ////
	Pos findAnyObjectPos(char marker)
	{
		auto foundObj = std::find(reinterpret_cast<char*>(m_objects),
			reinterpret_cast<char*>(m_objects) + (MAP_WIDTH + 1) * MAP_HEIGHT, '2');

		if (foundObj == reinterpret_cast<char*>(m_objects) + (MAP_WIDTH + 1) * MAP_HEIGHT)
			return Pos(-1, -1);

		int idx = (int)std::distance(reinterpret_cast<char*>(m_objects), foundObj);
		return Pos(idx % (MAP_WIDTH + 1), idx / (MAP_WIDTH + 1));
	}
};