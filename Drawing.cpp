#include <string>
#include <time.h>
#include "Drawing.h"


Drawing::Drawing(short width, short height, bool fps)
	: width(width), height(height), showFPS(fps)
{
	// Hide cursor
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO sci;
	GetConsoleCursorInfo(handle, &sci);
	sci.bVisible = FALSE;
	SetConsoleCursorInfo(handle, &sci);

	// Get HDC for painting
	HWND hwnd = GetConsoleWindow();

	hDC = GetDC(hwnd);
	hDCMem = CreateCompatibleDC(hDC);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = width;
	bi.bmiHeader.biHeight = -height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;

	// Initializing
	bitmap = ::CreateDIBSection(hDCMem, &bi, DIB_RGB_COLORS, (VOID**)&pixels, NULL, 0);

	if (!bitmap)
		throw "Cant create bitmap";

	oldbmp = ::SelectObject(hDCMem, bitmap);

	// Fill canvas white
	memset(pixels, 255, (size_t)width * (size_t)height * 4);

	if (showFPS)
		SetConsoleTitleA("---fps");
}


Drawing::~Drawing()
{
	SelectObject(hDCMem, oldbmp);
	DeleteDC(hDCMem);
	DeleteObject(bitmap);
}


short Drawing::getWidth()
{
	return width;
}

short Drawing::getHeight()
{
	return height;
}


int Drawing::getPosX()
{
	return alignX;
}


int Drawing::getPosY()
{
	return alignY;
}


COLORREF Drawing::getPixel(int x, int y) const
{
	if (x < 0 || y < 0 || x > width || y > height)
		return 0;

	int idx = y * pitch + x * 4;

	return RGB(pixels[idx + 2], pixels[idx + 1], pixels[idx + 0]);
}


unsigned char* Drawing::getArray() const
{
	unsigned char* arr = new unsigned char[4 * (size_t)width * height];

	memcpy(arr, pixels, sizeof(unsigned char) * 4 * width * height);

	return arr;
}


void Drawing::draw()
{
	do
	{
		StretchBlt(hDC, alignX, alignY, width * scaleV, height * scaleV, hDCMem, 0, 0, width, height, SRCCOPY);

		if (showFPS)
		{
			static auto freq = clock();
			static int fps(0);

			if (clock() - freq > 1000)
			{
				SetConsoleTitleA((std::to_string(fps) + "fps").c_str());
				freq = clock();
				fps = 0;
			}
			else
				++fps;
		}
	} while (threading);
}


void Drawing::startDraw()
{
	threading = true;

	std::thread dThread(&Drawing::draw, this);
	dThread.detach();
}

void Drawing::stopDraw()
{
	threading = false;
}


void Drawing::setPixel(short x, short y, COLORREF rgb, float a)
{
	if (x < 0 || y < 0 || x > width || y > height)
		return;

	if (a < 0) a = 0;
	if (a > 1) a = 1;

	int index = y * pitch + x * 4;

	pixels[index + 0] = UCHAR(GetBValue(rgb) * a + pixels[index + 0] * (1 - a));
	pixels[index + 1] = UCHAR(GetGValue(rgb) * a + pixels[index + 1] * (1 - a));
	pixels[index + 2] = UCHAR(GetRValue(rgb) * a + pixels[index + 2] * (1 - a));
}


void Drawing::setArray(unsigned char* arr, bool copy)
{
	memcpy(pixels, arr, sizeof(unsigned char) * 4 * width * height);
	if (!copy)
		delete[] arr;
}


void Drawing::fill(COLORREF rgb)
{
	UCHAR b = GetBValue(rgb);
	UCHAR g = GetGValue(rgb);
	UCHAR r = GetRValue(rgb);

	for (int x(0); x < width; ++x)
		for (int y(0); y < height; ++y)
		{
			int index = y * pitch + x * 4;

			pixels[index + 0] = b;  // blue
			pixels[index + 1] = g;  // green
			pixels[index + 2] = r;  // red
		}
}


void Drawing::loadPic(std::wstring path)
{
	bitmap = (HBITMAP)LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	BITMAP BMP;
	GetObject(bitmap, sizeof(BMP), &BMP); // Here we get the BMP header info.

	SelectObject(hDCMem, bitmap);
}


void Drawing::loadResource(int resId)
{
	bitmap = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(resId));

	BITMAP BMP;
	GetObject(bitmap, sizeof(BMP), &BMP); // Here we get the BMP header info.

	SelectObject(hDCMem, bitmap);
}


void Drawing::rect(int left, int top, int width, COLORREF rgb, float a)
{
	for (int x(left); x < left + width; ++x)
		for (int y(top); y < top + width; ++y)
			setPixel(x, y, rgb, a);
}


void Drawing::circle(int left, int top, int radius, COLORREF rgb, float a)
{
	double centerX = double(left) + radius;
	double centerY = double(top) + radius;

	for (int x(left); x < left + radius * 2; ++x)
		for (int y(top); y < top + radius * 2; ++y)
			if (pow(x - centerX, 2) + pow(y - centerY, 2) < pow(radius, 2))
				setPixel(x, y, rgb, a);
}


void Drawing::roundedRect(int left, int top, int power, int radius, COLORREF rgb, float a)
{
	double centerX = double(left) + radius;
	double centerY = double(top) + radius;

	for (int x(left); x < left + radius * 2; ++x)
		for (int y(top); y < top + radius * 2; ++y)
			if (pow(x - centerX, power) + pow(y - centerY, power) < radius)
				setPixel(x, y, rgb, a);
}


void Drawing::align(int X, int Y)
{
	alignX = X;
	alignY = Y;
}
void Drawing::align(Aligns hType, Aligns vType, int offsetX, int offsetY)
{
	auto scaled = GetSystemMetrics(SM_CXSCREEN);

	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_READ, &hKey);
	if (lRes == ERROR_FILE_NOT_FOUND) MessageBox(GetConsoleWindow(), L"Cant find directory", L"One little error", MB_OK);

	WCHAR szBuffer[2];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError = RegQueryValueEx(hKey, L"MaxMonitorDimension", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (nError != ERROR_SUCCESS) MessageBox(GetConsoleWindow(), L"Cant read value", L"One little error", MB_OK);
	RegCloseKey(hKey);

	float factor = (float)szBuffer[0] / scaled;

	HWND hWnd = GetConsoleWindow();
	RECT rc;
	GetClientRect(hWnd, &rc);
	int w = int(rc.right * factor);  // Ширина рабочей области консоли
	int h = int(rc.bottom * factor); // Высота рабочей области консоли


	switch (hType)
	{
	case Aligns::LEFT:
		alignX = offsetX;
		break;
	case Aligns::CENTER:
		alignX = ((w - this->getWidth() * scaleV) >> 1) + offsetX;
		break;
	case Aligns::RIGHT:
		alignX = (w - this->getWidth() * scaleV) - offsetX;
		break;
	}


	switch(vType)
	{
	case Aligns::TOP:
		alignY = offsetY;
		break;
	case Aligns::CENTER:
		alignY = ((h - this->getHeight() * scaleV) >> 1) + offsetY;
		break;
	case Aligns::BOTTOM:
		alignY = (h - this->getHeight() * scaleV) - offsetY;
		break;
	}
}


void Drawing::scale(short scaleX)
{
	scaleV = scaleX;
}