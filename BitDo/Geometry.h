#pragma once

#include <math.h>
#include "BaseBit.h"

class Geometry :public BaseBit
{
private:
	BitInfo m_stBit_New;//��ǰ��λͼ��Ϣ

private:
	/*
	���ݵ��滻
	*/
	BOOL ReplaceBitBuffer(CONST LPBYTE pBit,
		LPRGBQUAD pRGB = nullptr);

public:
	/*
	���»�ȡԴλͼ������
	*/
	BOOL AgainCopyBitInfo();

	/*
	�����ļ�
	*/
	BOOL WriteBit(CONST CHAR* szBitPath);

	/*
	����λͼ
	*/
	BOOL DrawBit(HDC hDc);

	virtual BOOL LoadBitInfo(PBitInfo pBit);
	virtual BOOL BitInfoTo(PBitInfo pBit);
public:
	Geometry();
	~Geometry();

public:
	/*
	ƽ��λͼ
	*/
	VOID Move(INT nX = NULL, 
		INT nY = NULL);

	/*
	����λͼ
	*/
	VOID Zoom(double dX = NULL,
		double dY = NULL);

	/*
	ˮƽ����
	*/
	VOID Level();

	/*
	��ֱ����
	*/
	VOID Vertical();

	/*
	˳ʱ����ת90
	*/
	VOID Clockwise90();

	/*
	��ʱ����ת90
	*/
	VOID AntiClockwise90();

	/*
	��ת180
	*/
	VOID Rotate180();

	/*
	��ת����Ƕ�
	*/
	VOID Rotate(INT nAngle = NULL);

};

