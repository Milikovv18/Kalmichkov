#pragma once
#define NULL 0

struct Pos
{
	int x{ 0 }, y{ 0 };

	Pos() {}
	Pos(int x, int y) : x(x), y(y) {}
};


struct Info
{
	Pos pos;

	Info(Pos coord) : pos(coord.x, coord.y) {}
	Info(int x, int y) : pos(x, y) {}
};


struct ElemDeque
{
	Info* pInf;
	ElemDeque* next;
};

struct Form // –екомендованный формул€р дл€ указателей
{ ElemDeque* head, * last; };


void InitForm(Form& A);

void InitDeque(ElemDeque* p);

void InitDeque(ElemDeque* p, Info* pData);

void InsOne(Form& A);

void InsEnd(Form& A);

Info* DelBeg(Form& A);

void DelDeque(Form& A);