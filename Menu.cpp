#include <iomanip>
#include <conio.h>
#include "Network.h"


Network::Type showMenu()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	int halfWidth(csbi.srWindow.Right / 2), halfHeight(csbi.srWindow.Bottom / 2);

	for (int i(0); i < 0.4 * halfHeight; ++i)
		std::cout << std::endl;
	std::cout << std::setw(halfWidth + 31LL) << " _   __      _           _      _     _              _____ \n";
	std::cout << std::setw(halfWidth + 31LL) << "| | / /     | |         (_)    | |   | |            |____ |\n";
	std::cout << std::setw(halfWidth + 31LL) << "| |/ /  __ _| |_ __ ___  _  ___| |__ | | _______   __   / /\n";
	std::cout << std::setw(halfWidth + 31LL) << "|    \\ / _` | | '_ ` _ \\| |/ __| '_ \\| |/ / _ \\ \\ / /   \\ \\\n";
	std::cout << std::setw(halfWidth + 31LL) << "| |\\  \\ (_| | | | | | | | | (__| | | |   < (_) \\ V /.___/ /\n";
	std::cout << std::setw(halfWidth + 31LL) << "\\_| \\_/\\__,_|_|_| |_| |_|_|\\___|_| |_|_|\\_\\___/ \\_/ \\____/ \n";

	std::cout << "\n\n" << std::setw(halfWidth + 13LL) << "Choose your game mode\n\n";

	short state(0);
	std::cout << '\r' << std::setw(halfWidth + 12LL) << "[ < SERVER (single) > ]";
	while (true)
	{
		int key = _getch();
		if (key == 75 || key == 77)
		{
			state = (state + 1) % 2;

			if (state == 0)
				std::cout << '\r' << std::setw(halfWidth + 12LL) << "[ < SERVER (single) > ]";
			else if (state == 1)
				std::cout << '\r' << std::setw(halfWidth + 12LL) << "[     < CLIENT >      ]";
		}
		else if (key == '\r')
			break;
	}

	std::cout << "\n\n";
	for (int i(3); i > 0; --i)
	{
		std::cout << '\r' << std::setw(halfWidth + 8LL) << "Game starts in " << i;
		Sleep(500);
	}
	system("cls");
	Sleep(100);
	return Network::Type(state);
}