#pragma once

#include "BaseBit.h"

class GrayTran :public BaseBit
{
private:
	BitInfo m_stBit_New;//λͼ�ṹ��
	LPRGBQUAD m_pRgb;//��ɫ��ָ��

private:
	/*
	���ݵ��滻
	*/
	BOOL ReplaceBitBuffer(CONST LPBYTE pBit,
		LPRGBQUAD pRGB = nullptr);

public:
	GrayTran();
	~GrayTran();

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
	virtual BOOL BitInfoTo(PBitInfo pBit);
public:
	/*
	��ֵ��
	*/
	VOID Binarization(INT nThreshold = 100);

	/*
	RGBλͼת�Ҷ�ͼ
	*/
	VOID ColorToGray();

	/*
	��ɫ��ת
	*/
	VOID Reverse();

};

