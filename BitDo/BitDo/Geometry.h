#pragma once

#include <math.h>
#include "BaseBit.h"

class Geometry :public BaseBit
{
private:
	BitInfo m_stBit_New;//当前的位图信息

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
	保存文件
	*/
	BOOL WriteBit(CONST CHAR* szBitPath);

	/*
	绘制位图
	*/
	BOOL DrawBit(HDC hDc);

	virtual BOOL LoadBitInfo(PBitInfo pBit);
	virtual BOOL BitInfoTo(PBitInfo pBit);
public:
	Geometry();
	~Geometry();

public:
	/*
	平移位图
	*/
	VOID Move(INT nX = NULL, 
		INT nY = NULL);

	/*
	缩放位图
	*/
	VOID Zoom(double dX = NULL,
		double dY = NULL);

	/*
	水平镜像
	*/
	VOID Level();

	/*
	垂直镜像
	*/
	VOID Vertical();

	/*
	顺时针旋转90
	*/
	VOID Clockwise90();

	/*
	逆时针旋转90
	*/
	VOID AntiClockwise90();

	/*
	旋转180
	*/
	VOID Rotate180();

	/*
	旋转任意角度
	*/
	VOID Rotate(INT nAngle = NULL);

};

