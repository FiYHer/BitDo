#include "BaseBit.h"



BaseBit::BaseBit()
{

}


BaseBit::~BaseBit()
{
	ClearBitInfo(&m_pBit);
}


LPBYTE BaseBit::AllocBuf(INT nWidth /*= NULL*/, 
	INT nHeight /*= NULL*/, 
	INT nBitCount /*= NULL*/)
{
	//����Ϊ0
	if (!nWidth || !nHeight || !nBitCount)
		return nullptr;

	//һ�е�������
	INT nLineByte = (nWidth*nBitCount / 8 + 3) / 4 * 4;
	//����λͼ��������
	INT nSumByte = nLineByte * nHeight;

	//�����ڴ�
	LPBYTE pBuf = (LPBYTE)VirtualAlloc(NULL, nSumByte, MEM_COMMIT, PAGE_READWRITE);
	if (pBuf)
		ZeroMemory(pBuf, nSumByte);
	return pBuf;
}

HPALETTE BaseBit::GetPalette(INT nColorTable, LPRGBQUAD pRgb)
{
	//�����ɫ����Ϊ0����ȫ��ɫλͼ������Ҫ�����ɫ��
	if (!nColorTable)
		return NULL;

	//����ռ������ɫ�ṹ
	LPLOGPALETTE pLogPalette = (LPLOGPALETTE)VirtualAlloc(NULL, 2 * sizeof(WORD) +
		nColorTable * sizeof(PALETTEENTRY), MEM_COMMIT, PAGE_READWRITE);
	if (!pLogPalette)
		return NULL;

	//�ṹ�����
	pLogPalette->palVersion = 0x300;
	pLogPalette->palNumEntries = nColorTable;
	for (INT nIndex = NULL; nIndex < nColorTable; nIndex++)
	{
		pLogPalette->palPalEntry[nIndex].peRed = pRgb[nIndex].rgbRed;
		pLogPalette->palPalEntry[nIndex].peGreen = pRgb[nIndex].rgbGreen;
		pLogPalette->palPalEntry[nIndex].peBlue = pRgb[nIndex].rgbBlue;
		pLogPalette->palPalEntry[nIndex].peFlags = NULL;
	}

	//�����߼���ɫ����
	HPALETTE hPalette = CreatePalette(pLogPalette);
	VirtualFree(pLogPalette, NULL, MEM_RELEASE);
	return hPalette;
}

VOID BaseBit::ClearBitInfo(PBitInfo pBit)
{
	if (!pBit)
		return;

	if (pBit->pBitmap)//λͼ������
		VirtualFree(pBit->pBitmap, NULL, MEM_RELEASE);
	ZeroMemory(pBit, sizeof(BitInfo));
}

VOID BaseBit::InitBitInfo(PBitInfo pBit,
	INT nBitCount)
{
	//�ṹ��ָ��λ��
	if (!pBit)
		return;

	//��λͼָ��Ϊ��
	if (!pBit->pBitmap)
		return;

	LPBYTE pBitmap = pBit->pBitmap;
	pBit->pBitFile = (LPBITMAPFILEHEADER)pBitmap;
	pBit->pBitInfo = (LPBITMAPINFOHEADER)(pBitmap + SIZE__FILE);
	INT nColorTable = GetColorTable(nBitCount);
	if (nColorTable)//����е�ɫ��Ļ�
		pBit->pRGBTable = (LPRGBQUAD)(pBitmap + SIZE__FILE + SIZE__INFO);
	pBit->pBuf = pBitmap + SIZE__OFBITS(nColorTable);
}

BOOL BaseBit::LoadBitInfo(PBitInfo pBit)
{
	if (!pBit)
		return FALSE;

	ClearBitInfo(&m_pBit);

	//�ܴ�С
	INT nSize = pBit->pBitFile->bfOffBits + pBit->pBitInfo->biSizeImage;

	LPBYTE pBuffer = (LPBYTE)VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE);
	if (!pBuffer)
		return FALSE;

	//�����ݵĸ���
	CopyMemory(pBuffer, pBit->pBitmap, nSize);
	m_pBit.pBitmap = pBuffer;
	InitBitInfo(&m_pBit);
	return TRUE;
}

PBitInfo BaseBit::GetBitInfo()
{
	return &m_pBit;
}

BOOL BaseBit::ReadBit(CONST CHAR* szBitPath)
{
	BOOL bStatu = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	INT nColorTable = NULL;
	INT nSize = NULL;
	DWORD dwByte = NULL;
	LPBYTE pBitmap = nullptr;
	do 
	{
		//�������
		ClearBitInfo(&m_pBit);

		//·�����
		if (!szBitPath || !strlen(szBitPath))
			break;

		//��λͼ�ļ�
		hFile = CreateFileA(szBitPath,GENERIC_READ, 
			FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			break;

		//��ȡλͼ���ܴ�С
		nSize = GetFileSize(hFile, NULL);
		if (!nSize)
			break;

		//�����ڴ�
		pBitmap = (LPBYTE)VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE);
		if (!pBitmap)
			break;

		//��ȡλͼ��ȫ������
		ReadFile(hFile, pBitmap, nSize, &dwByte, NULL);
		if (!dwByte)
			break;

		m_pBit.pBitmap = pBitmap;//λͼ������
		m_pBit.pBitInfo = (LPBITMAPINFOHEADER)(pBitmap + SIZE__FILE);//λͼ��Ϣͷ
		InitBitInfo(&m_pBit, m_pBit.pBitInfo->biBitCount);

		//��һЩλͼ��Ϣͷ��BiSizeImage��Աû�еģ������ֶ�����һ��
		if (!m_pBit.pBitInfo->biSizeImage)
		{
			nColorTable = GetColorTable(m_pBit.pBitInfo->biBitCount);
			m_pBit.pBitInfo->biSizeImage = nSize - SIZE__OFBITS(nColorTable);
		}

		bStatu = TRUE;
	} while (FALSE);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	return bStatu;
}

BOOL BaseBit::WriteBit(CONST CHAR* szBitPath, PBitInfo pBit /*= NULL*/)
{
	BOOL bStatu = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwByte = NULL;
	INT nSize = NULL;
	PBitInfo pTempBit = nullptr;
	if (pBit)
		pTempBit = pBit;
	else
		pTempBit = &m_pBit;

	do 
	{
		//�ļ��ַ����ж�
		if (!szBitPath || !strlen(szBitPath))
			break;

		//����λͼ�ļ��ļ�
		hFile = CreateFileA(szBitPath, GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			break;

		//д��λͼ����
		if (pTempBit->pBitmap)
		{
			//Ϊ0��������޷��ж�λͼ���ݵĴ�С
			if(!pTempBit->pBitInfo->biSizeImage)
				break;

			//λͼ�ܴ�С
			INT nSize = pTempBit->pBitFile->bfOffBits + pTempBit->pBitInfo->biSizeImage;
			WriteFile(hFile, pTempBit->pBitmap, nSize, &dwByte, NULL);
			if (!dwByte)
				break;
		}

		//����ˢ�µ��ļ�
		FlushFileBuffers(hFile);
		bStatu = TRUE;
	} while (FALSE);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	return bStatu;
}

BOOL BaseBit::DrawBit(HDC hDc, PBitInfo pBit /*= NULL*/)
{
	PBITMAPINFOHEADER pBitInfo = nullptr;
	LPBYTE pBuf = nullptr;
	LPRGBQUAD pRgb = nullptr;
	HPALETTE hOldPalette = NULL;
	HPALETTE hNewPalettr = NULL;
	if (pBit)
	{
		pBitInfo = pBit->pBitInfo;
		pBuf = pBit->pBuf;
		pRgb = pBit->pRGBTable;
	}
	else
	{
		pBitInfo = m_pBit.pBitInfo;
		pBuf = m_pBit.pBuf;
		pRgb = m_pBit.pRGBTable;
	}

	//���ƾ������λ0
	if (!hDc)
		return FALSE;

	//�е�ɫ��Ļ�
	if (pRgb)
	{
		//����һ����ɫ����
		hNewPalettr = GetPalette(GetColorTable(pBitInfo->biBitCount), pRgb);
		if (hNewPalettr)
		{
			//ѡ��DC�ٳ�ʼ��һ��
			hOldPalette = SelectPalette(hDc, hNewPalettr, TRUE);
			RealizePalette(hDc);
		}
	}

	//���û���ģʽλ����ģʽ
	SetStretchBltMode(hDc, COLORONCOLOR);

	//����λͼ����
	StretchDIBits(hDc, NULL, NULL,
		pBitInfo->biWidth, pBitInfo->biHeight,
		0, 0, pBitInfo->biWidth, pBitInfo->biHeight,
		pBuf, (LPBITMAPINFO)pBitInfo, DIB_RGB_COLORS, SRCCOPY);
	
	//�ָ�ԭ���ĵ�ɫ��
	if (hOldPalette)
	{
		SelectPalette(hDc, hOldPalette, TRUE);
		DeleteObject(hNewPalettr);
	}
	return TRUE;
}
