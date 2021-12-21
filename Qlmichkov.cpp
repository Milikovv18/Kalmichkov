#include "Qlmichkov.h"

void InitForm(Form& A)
{ A.head = NULL; A.last = NULL; }


void InitDeque(ElemDeque* p)
{
	p->next = NULL;
	p->pInf = NULL;
}


void InitDeque(ElemDeque* p, Info* pData)
{
	p->next = NULL;
	p->pInf = pData;
}


void InsOne(Form& A)
{
	A.head = new ElemDeque;
	InitDeque(A.head);
	A.last = A.head;
}


void InsEnd(Form& A)
{
	if (A.head == NULL) InsOne(A); // ��������� ������� �������
	else
	{
		ElemDeque* pNew = new ElemDeque;
		InitDeque(pNew);
		A.last->next = pNew;
		A.last = pNew;
	}
}


Info* DelBeg(Form& A)
{
	Info* inf = NULL;
	if (A.head != NULL) // ��� ������ ������� ������ �� ������
	// ������� ������ ������� �������
		if (A.head == A.last) // ���� �� ������������, �� ������� ������ �������
		{
			inf = A.head->pInf;
			delete A.head;
			A.head = NULL; A.last = NULL;
		}
		else
		{
			ElemDeque* pDel = A.head;
			inf = pDel->pInf;
			A.head = A.head->next;
			delete pDel;
		}
	return inf;
}


void DelDeque(Form& A)
{
	Info* inf;
	while (A.head != NULL)
	{
		inf = DelBeg(A);
		if (inf != NULL) delete A.head->pInf; // ����� ��� ��� �������� � "�������" � ������
	}
}