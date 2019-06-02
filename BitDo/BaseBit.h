#pragma once

#include <windows.h>

#define SIZE__FILE sizeof(BITMAPFILEHEADER)//�ļ�ͷ��С
#define SIZE__INFO sizeof(BITMAPINFOHEADER)//��Ϣͷ��С
#define SIZE__PALETTE(x) (sizeof(RGBQUAD)*(x)) //��ɫ���С 
#define SIZE__OFBITS(x) (SIZE__PALETTE((x))+SIZE__FILE+SIZE__INFO)

//���ǵ�λͼ�ṹ��
typedef struct _BitInfo
{
	LPBYTE pBitmap;//λͼ�����ڴ�
	LPBITMAPFILEHEADER pBitFile;//λͼ�ļ�ͷָ��
	LPBITMAPINFOHEADER pBitInfo;//λͼ��Ϣͷָ��
	LPBYTE pBuf;//λͼRGB����ָ��
	LPRGBQUAD pRGBTable;//λͼ��ɫ������ָ��
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
	BitInfo m_pBit;//λͼ����

public:
	BaseBit();
	virtual ~BaseBit();

public:
	/*
	��ȡλͼ�ļ�
	*/
	BOOL ReadBit(CONST CHAR* szBitPath);

	/*
	����λͼ�ļ�
	*/
	BOOL WriteBit(CONST CHAR* szBitPath,
		PBitInfo pBit = NULL);

	/*
	��ȡ��ɫ��ĳ���
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
	����λͼ
	*/
	BOOL DrawBit(HDC hDc, 
		PBitInfo pBit = NULL);

	/*
	����λͼ�����ڴ�
	*/
	LPBYTE AllocBuf(INT nWidth = NULL,
		INT nHeight = NULL,
		INT nBitCount = NULL);

	/*
	������ɫ��
	*/
	HPALETTE GetPalette(INT nColorTable,
		LPRGBQUAD pRgb);

	/*
	�ͷ��ڴ�
	*/
	VOID ClearBitInfo(PBitInfo pBit);

	/*
	��ʼ���ṹ���ָ���ָ��
	*/
	VOID InitBitInfo(PBitInfo pBit, INT nBitCount = NULL);

	/*
	���������
	*/
	virtual BOOL LoadBitInfo(PBitInfo pBit);

	/*
	���Ƶ�����
	*/
	virtual PBitInfo GetBitInfo();
};

