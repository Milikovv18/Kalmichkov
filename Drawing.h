#pragma once
#include <thread>
#include <string>
#include <Windows.h>

enum class Aligns
{
	LEFT,
	TOP,
	CENTER,
	RIGHT,
	BOTTOM
};

class Drawing
{
	// Рисование в отдельном потоке
	bool threading = false;

	// Window size
	short width;
	short height;

	// Располодение полотна относительно
	// левой и верхней строн консоли
	short alignX = 0;
	short alignY = 0;

	// Масштабирование изображения
	short scaleV = 1;

	// 4 bytes per pixel but if not 32 bit, round pitch up to multiple of 4
	const int pitch = 4 * width;
	HDC hDC = nullptr;
	HDC hDCMem = nullptr;
	HBITMAP bitmap = nullptr;
	HGDIOBJ oldbmp = nullptr;

	// Массив пикселей изображения
	unsigned char* pixels;

public:

	// Изменяемые переменные
	bool showFPS = false;

	// Constructors & Destructors
	Drawing(short width, short height, bool fps = true);
	~Drawing();

	// Getters
	short getWidth();
	short getHeight();
	int getPosX();
	int getPosY();

	COLORREF getPixel(int x, int y) const;
	unsigned char* getArray() const;

	// Methods
	void draw();
	void startDraw();
	void stopDraw();
	void setPixel(short x, short y, COLORREF rgb, float a = 1);
	void setArray(unsigned char* arr, bool copy = false);
	void fill(COLORREF rgb);
	void loadPic(std::wstring path);
	void loadResource(int resId);
	void rect(int left, int top, int width, COLORREF rgb, float a = 1);
	void circle(int left, int top, int radius, COLORREF rgb, float a = 1);
	void roundedRect(int left, int top, int power, int radius, COLORREF rgb, float a = 1);
	void align(int X, int Y);
	void align(Aligns hType = Aligns::LEFT, Aligns vType = Aligns::TOP, int offsetX = 0, int offsetY = 0);
	void scale(short scaleX);
};