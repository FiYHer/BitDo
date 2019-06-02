#pragma once

#include <windows.h>

#define SIZE__FILE sizeof(BITMAPFILEHEADER)//文件头大小
#define SIZE__INFO sizeof(BITMAPINFOHEADER)//信息头大小
#define SIZE__PALETTE(x) (sizeof(RGBQUAD)*(x)) //调色板大小 
#define SIZE__OFBITS(x) (SIZE__PALETTE((x))+SIZE__FILE+SIZE__INFO)

//我们的位图结构体
typedef struct _BitInfo
{
	LPBYTE pBitmap;//位图的总内存
	LPBITMAPFILEHEADER pBitFile;//位图文件头指针
	LPBITMAPINFOHEADER pBitInfo;//位图信息头指针
	LPBYTE pBuf;//位图RGB数据指针
	LPRGBQUAD pRGBTable;//位图调色板数据指针
	_BitInfo()
	{
		pBitmap = nullptr;
		pBitFile = nullptr;
		pBitInfo = nullptr;
		pBuf = nullptr;
		pRGBTable = nullptr;
	}
}BitInfo,*PBitInfo;


class BaseBit
{
protected:
	BitInfo m_pBit;//位图数据

public:
	BaseBit();
	virtual ~BaseBit();

public:
	/*
	读取位图文件
	*/
	BOOL ReadBit(CONST CHAR* szBitPath);

	/*
	保存位图文件
	*/
	BOOL WriteBit(CONST CHAR* szBitPath,
		PBitInfo pBit = NULL);

	/*
	获取颜色表的长度
	*/
	inline INT GetColorTable(INT nBitCount)
	{
		INT nColorTable = NULL;
		if (nBitCount == 1)
			nColorTable = 2;
		else if (nBitCount == 4)
			nColorTable = 16;
		else if (nBitCount == 8)
			nColorTable = 256;
		return nColorTable;
	}

	/*
	绘制位图
	*/
	BOOL DrawBit(HDC hDc, 
		PBitInfo pBit = NULL);

	/*
	申请位图数据内存
	*/
	LPBYTE AllocBuf(INT nWidth = NULL,
		INT nHeight = NULL,
		INT nBitCount = NULL);

	/*
	创建调色板
	*/
	HPALETTE GetPalette(INT nColorTable,
		LPRGBQUAD pRgb);

	/*
	释放内存
	*/
	VOID ClearBitInfo(PBitInfo pBit);

	/*
	初始化结构体的指针的指向
	*/
	VOID InitBitInfo(PBitInfo pBit, INT nBitCount = NULL);

	/*
	从哪里加载
	*/
	virtual BOOL LoadBitInfo(PBitInfo pBit);

	/*
	复制到哪里
	*/
	virtual PBitInfo GetBitInfo();
};

