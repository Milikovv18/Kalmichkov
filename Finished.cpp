#include "resource.h"
#include "Drawing.h"

void youLost(Drawing &pic)
{
	pic.loadResource(BMP_GAMEOVER);
	pic.draw();
	Sleep(10000);
}



void youWin(Drawing& pic)
{
	pic.loadResource(BMP_VICTORY);
	pic.draw();
	Sleep(5000);
}