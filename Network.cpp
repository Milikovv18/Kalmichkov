#include "Network.h"

bool Network::startServer(const char* ip)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	m_conSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		int a = WSAGetLastError();
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	m_conSocket = accept(ListenSocket, NULL, NULL);
	if (m_conSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	m_connected = true;
	return 0;
}


bool Network::startClient(const char* ip)
{
	WSADATA wsaData;
	m_conSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return false;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		m_conSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (m_conSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return false;
		}

		// Connect to server.
		iResult = connect(m_conSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(m_conSocket);
			m_conSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (m_conSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return false;
	}

	m_connected = true;
	return true;
}


// Returns false if dead or connection lost
// съедено? размер(0 или 2) [newHead] размер(2 * размер массива) [tail array] (2 * размер массива) [undead, particles]
// (3 * размер массива) [applesToSend] (3 * размер массива) [wallsToSend]
bool Network::sendData(bool ate, bool moved, const Pos& head,
	const std::vector<Pos>& erasedTails, const short effectsToSend[2],
	const objectsArray& applesToSend, const objectsArray& wallsToSend)
{
	std::string toSend = "";

	// If ate some food
	toSend += BYTE(ate);

	// Shifted head coords
	toSend += BYTE(moved) * 2;
	if (moved)
	{
		toSend += BYTE(head.x);
		toSend += BYTE(head.y);
	}

	// Last erased tail coords
	toSend += BYTE(2 * erasedTails.size());
	for (const Pos& tail : erasedTails)
	{
		toSend += BYTE(tail.x); // Pos.x
		toSend += BYTE(tail.y); // Pos.y
	}

	// Online effects
	for (int i(0); i < 2; ++i)
	{
		toSend += BYTE(effectsToSend[i] >> 0); // First byte
		toSend += BYTE(effectsToSend[i] >> 8); // Second byte
	}

	// Sending apples
	toSend += BYTE(3 * applesToSend.size());
	for (const auto& apple : applesToSend)
	{
		toSend += BYTE(apple.first.x); // Pos.x
		toSend += BYTE(apple.first.y); // Pos.y
		toSend += BYTE(apple.second); // char
	}

	// Sending broken walls
	toSend += BYTE(3 * wallsToSend.size());
	for (const auto& wall : wallsToSend)
	{
		toSend += BYTE(wall.first.x); // Pos.x
		toSend += BYTE(wall.first.y); // Pos.y
		toSend += BYTE(wall.second); // char
	}

	int result = send(m_conSocket, toSend.c_str(), (int)toSend.size(), 0);
	return result > 0;
}

// Returns false if dead or connection lost
// съедено? размер(0 или 2) [newHead] размер(2 * размер массива) [tail array] (2 * размер массива) [undead, particles]
// (3 * размер массива) [applesToSend] (3 * размер массива) [wallsToSend]
bool Network::recvData(bool& ate, bool& moved, Pos& head,
	std::vector<Pos>& erasedTails, short effectsToRecv[2],
	objectsArray& applesToRecv, objectsArray& wallsToSend)
{
	BYTE buff[m_maxLen];
	int res = recv(m_conSocket, (char*)buff, m_maxLen, 0);
	if (res <= 0) {
		return false;
	}

	BYTE offset(0);

	// Check if eaten
	ate = buff[offset++];

	// Shifted head coords
	moved = buff[offset++];
	if (moved)
	{
		head.x = buff[offset++];
		head.y = buff[offset++];
	}

	// Last erased tail coords
	unsigned len = buff[offset++];
	for (unsigned i(0); i < len; i += 2) {
		erasedTails.push_back(Pos(buff[offset + i], buff[offset + 1 + i]));
	}
	offset += len;

	// Online effects
	for (int i(0); i < 4/*bytes*/; i += 2) {
		effectsToRecv[i / 2] |= (buff[offset + i + 1] << 8) | buff[offset + i];
	}
	offset += 2/*bytes*/ * 2/*effects num*/;

	// Apples array
	len = buff[offset++];
	for (unsigned i(0); i < len; i += 3) {
		applesToRecv.push_back(std::pair(Pos(
			buff[offset + 0 + i],
			buff[offset + 1 + i]),
			buff[offset + 2 + i]));
	}

	// Broken walls array
	offset += len;
	len = buff[offset++];
	for (unsigned i(0); i < len; i += 3) {
		wallsToSend.push_back(std::pair(Pos(
			buff[offset + 0 + i],
			buff[offset + 1 + i]),
			buff[offset + 2 + i]));
	}

	return true;
}


bool Network::sendMap(const MapHandler& gameMap)
{
	std::string serialized = "";
	for (int y(0); y < MAP_HEIGHT + 1; ++y)
		for (int x(0); x < MAP_WIDTH; ++x)
			serialized += BYTE(gameMap.getObject(x, y));

	int result = send(m_conSocket, serialized.c_str(), (int)serialized.size(), 0);
	return result > 0;
}

bool Network::recvMap(MapHandler& gameMap)
{
	char buff[MAP_WIDTH * (MAP_HEIGHT + 1)];
	int res = recv(m_conSocket, buff, MAP_WIDTH * (MAP_HEIGHT + 1), 0);
	if (res <= 0) {
		return false;
	}

	// Restoring map
	for (int y(0); y < MAP_HEIGHT; ++y)
		for (int x(0); x < MAP_WIDTH; ++x)
			gameMap.placeObj(Pos(x, y), buff[y * MAP_WIDTH + x]);

	return true;
}


bool Network::isConnected()
{
	return m_connected;
}


Network::Type Network::type()
{
	return m_type;
}