#pragma once

#include <math.h>
#include "BaseBit.h"

class AreaDetect:public BaseBit
{
private:
	BitInfo m_stBit_New;

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
	����
	*/
	BOOL DrawBit(HDC hDc);

	/*
	�����ļ�
	*/
	BOOL WriteBit(CONST CHAR* szBitPath);

	virtual BOOL LoadBitInfo(PBitInfo pBit);
	virtual PBitInfo GetBitInfo();

public:
	AreaDetect();
	~AreaDetect();

public:
	/*
	Roberts��Ե���
	*/
	VOID Roberts();




};
