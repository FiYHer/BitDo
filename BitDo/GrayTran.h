#pragma once

#include "BaseBit.h"

class GrayTran :public BaseBit
{
private:
	BitInfo m_stBit_New;//位图结构体
	LPRGBQUAD m_pRgb;//调色板指针

private:
	/*
	数据的替换
	*/
	BOOL ReplaceBitBuffer(CONST LPBYTE pBit,
		LPRGBQUAD pRGB = nullptr);

public:
	GrayTran();
	~GrayTran();

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
	virtual BOOL BitInfoTo(PBitInfo pBit);
public:
	/*
	二值化
	*/
	VOID Binarization(INT nThreshold = 100);

	/*
	RGB位图转灰度图
	*/
	VOID ColorToGray();

	/*
	颜色反转
	*/
	VOID Reverse();

};

