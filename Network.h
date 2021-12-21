#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <atomic>

#include "Qlmichkov.h"
#include "Maps.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

constexpr const char* DEFAULT_PORT = "7007";
typedef std::vector<std::pair<Pos, char>> objectsArray;


class Network
{
public:
	enum class Type
	{
		SERVER,
		CLIENT
	};

	static void startup(Type type, const char* ip)
	{
		m_type = type;

		if (m_type == Type::SERVER) {
			std::thread waitingServCon(startServer, ip);
			waitingServCon.detach();
		}
		else if (m_type == Type::CLIENT) {
			if (!startClient(ip)) exit(-1);
		}
	}

	static void shutdown()
	{
		closesocket(m_connected);
		closesocket(m_conSocket);
		WSACleanup();
	}

	// Если данные в [] не изменились с последней отправки, то они снова не отправляются
	// размер(0 или 2) [newHead] размер(2 * размер массива) [tail array] (3 * размер массива) [applesToSend]
	static bool sendData(bool ate, bool moved, const Pos& head, const std::vector<Pos>& erasedTails, const short effectsToSend[2],
		const objectsArray& applesToSend, const objectsArray& wallsToSend);
	static bool recvData(bool& ate, bool& moved, Pos& head, std::vector<Pos>& erasedTails, short effectsToRecv[2],
		objectsArray& applesToRecv, objectsArray& applesToSend);
	static bool sendMap(const MapHandler& gameMap);
	static bool recvMap(MapHandler& gameMap);
	static bool isConnected();
	static Type type();

private:

	inline static Type m_type{ Type::SERVER };
	inline static SOCKET m_conSocket{ INVALID_SOCKET };
	inline static std::atomic<bool> m_connected{ false };
	static const unsigned m_maxLen = 64;

	static bool startServer(const char* ip);
	static bool startClient(const char* ip);
};
