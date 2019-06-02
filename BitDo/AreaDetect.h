#pragma once

#include <math.h>
#include "BaseBit.h"

class AreaDetect:public BaseBit
{
private:
	BitInfo m_stBit_New;

private:
	/*
	数据的替换
	*/
	BOOL ReplaceBitBuffer(CONST LPBYTE pBit,
		LPRGBQUAD pRGB = nullptr);

public:
	/*
	重新获取源位图的数据
	*/
	BOOL AgainCopyBitInfo();

	/*
	绘制
	*/
	BOOL DrawBit(HDC hDc);

	/*
	保存文件
	*/
	BOOL WriteBit(CONST CHAR* szBitPath);

	virtual BOOL LoadBitInfo(PBitInfo pBit);
	virtual PBitInfo GetBitInfo();

public:
	AreaDetect();
	~AreaDetect();

public:
	/*
	Roberts边缘检测
	*/
	VOID Roberts();




};
