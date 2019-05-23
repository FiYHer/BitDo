#include "GrayTran.h"



BOOL GrayTran::ReplaceBitBuffer(CONST LPBYTE pBit, LPRGBQUAD pRGB)
{
	//λͼ������ָ�벻��Ϊ��
	if (!pBit)
		return FALSE;

	//����һ��λͼ���������ݣ���Ϊ����Ҫ�滻λͼ��RGB���ݺ͵�ɫ�����ݶ���
	BITMAPFILEHEADER stBitFile = { 0 };
	BITMAPINFOHEADER stBitInfo = { 0 };
	LPRGBQUAD pRgb = nullptr;

	//�ȱ���һ��λͼ������
	CopyMemory(&stBitFile, m_stBit_New.pBitFile, SIZE__FILE);
	CopyMemory(&stBitInfo, m_stBit_New.pBitInfo, SIZE__INFO);
	//��ȡ��ɫ����
	INT nColorTable = GetColorTable(m_stBit_New.pBitInfo->biBitCount);
	if (nColorTable)//����е�ɫ��
	{
		//�����ڴ�
		pRgb = (LPRGBQUAD)VirtualAlloc(NULL, SIZE__PALETTE(nColorTable), MEM_COMMIT, PAGE_READWRITE);
		if (!pRgb)
			return FALSE;

		if (m_stBit_New.pRGBTable)//ԭ���е�ɫ��
			CopyMemory(pRgb, m_stBit_New.pRGBTable, SIZE__PALETTE(nColorTable));
		if (pRGB)//����������˵�ɫ��ָ�룬�����óɽ�����
			CopyMemory(pRgb, pRGB, SIZE__PALETTE(nColorTable));

	}

	//λͼ���ݴ�С
	INT nSize = stBitInfo.biSizeImage;

	//�ܴ�С
	INT nSumSize = m_stBit_New.pBitFile->bfOffBits + m_stBit_New.pBitInfo->biSizeImage;

	//��������ڴ�
	ClearBitInfo(&m_stBit_New);

	//�����ڴ��ٱ����ٱ���
	LPBYTE pBitmap = (LPBYTE)VirtualAlloc(NULL, nSumSize, MEM_COMMIT, PAGE_READWRITE);
	if (!pBitmap)
	{
		if(pRgb)
			VirtualFree(pRgb, NULL, MEM_RELEASE);
		return FALSE;
	}

	m_stBit_New.pBitmap = pBitmap;
	InitBitInfo(&m_stBit_New, stBitInfo.biBitCount);

	//���ݵĿ���
	CopyMemory(m_stBit_New.pBitFile, &stBitFile, SIZE__FILE);
	CopyMemory(m_stBit_New.pBitInfo, &stBitInfo, SIZE__INFO);
	if (pRgb)
		CopyMemory(m_stBit_New.pRGBTable, pRgb, SIZE__PALETTE(nColorTable));
	CopyMemory(m_stBit_New.pBuf, pBit, nSize);

	if(pRgb)
		VirtualFree(pRgb, NULL, MEM_RELEASE);
	return TRUE;
}



GrayTran::GrayTran():BaseBit()
{
	m_pRgb = nullptr;
}


GrayTran::~GrayTran()
{
	ClearBitInfo(&m_stBit_New);
	if (m_pRgb)
	{
		VirtualFree(m_pRgb, NULL, MEM_RELEASE);
		m_pRgb = nullptr;
	}
}

BOOL GrayTran::AgainCopyBitInfo()
{
	ClearBitInfo(&m_stBit_New);

	//λͼ���ܴ�С
	INT nSize = m_pBit.pBitFile->bfOffBits + m_pBit.pBitInfo->biSizeImage;
	m_stBit_New.pBitmap = (LPBYTE)VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE);
	if (!m_stBit_New.pBitmap)
		return FALSE;
	CopyMemory(m_stBit_New.pBitmap, m_pBit.pBitmap, nSize);
	InitBitInfo(&m_stBit_New, m_pBit.pBitInfo->biBitCount);
	return TRUE;
}

VOID GrayTran::Binarization(INT nThreshold /*= 100*/)
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//����������8λ��λͼ�Ļ����Ǿ�ֱ�Ӷ�ֵ��
	if (m_stBit_New.pBitInfo->biBitCount == 8)
	{
		INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
		INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
		INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
		INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;

		LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountIn);
		if (!pBuffer)
			return;
		CopyMemory(pBuffer, m_stBit_New.pBuf, m_stBit_New.pBitInfo->biSizeImage);

		for (INT i = NULL; i < nHeightIn; i++)
		{
			for (INT j = NULL; j < nWidthIn; j++)
			{
				if (pBuffer[i * nLineByteIn + j] <= nThreshold)
					pBuffer[i * nLineByteIn + j] = 0;
				else
					pBuffer[i * nLineByteIn + j] = 255;
			}
		}
		ReplaceBitBuffer(pBuffer);
		return;
	}

	//Դλͼ��Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//���λͼ����ɫλ
	INT nBitCountOut = 8;
	INT nPeixlByteIn = nBitCountIn / 8;
	INT nLineByteIn = (nWidthIn*nPeixlByteIn + 3) / 4 * 4;
	INT nLineByteOut = (nWidthIn*nBitCountOut / 8 + 3) / 4 * 4;

	//���û�ͷž����ͷ�
	if (m_pRgb)
	{
		VirtualFree(m_pRgb, NULL, MEM_RELEASE);
		m_pRgb = nullptr;
	}

	INT nColorTable = GetColorTable(nBitCountOut);
	m_pRgb = (LPRGBQUAD)VirtualAlloc(NULL, SIZE__PALETTE(nColorTable), MEM_COMMIT, PAGE_READWRITE);
	if (!m_pRgb)
		return;
	for (INT nIndex = NULL; nIndex < nColorTable; nIndex++)
	{
		m_pRgb[nIndex].rgbRed = nIndex;
		m_pRgb[nIndex].rgbGreen = nIndex;
		m_pRgb[nIndex].rgbBlue = nIndex;
		m_pRgb[nIndex].rgbReserved = NULL;
	}

	LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountOut);
	if(!pBuffer)
		return;

	//�Ҷȼ��㹫ʽ
	INT nRed, nGreen, nBlue;
	for (INT i = NULL; i < nHeightIn; i++)
	{
		for (INT j = NULL; j < nWidthIn; j++)
		{
			nRed = (INT)(0.11*m_stBit_New.pBuf[i * nLineByteIn + j * nPeixlByteIn + 0]);
			nGreen = (INT)(0.59*m_stBit_New.pBuf[i * nLineByteIn + j * nPeixlByteIn + 1]);
			nBlue = (INT)(0.30*m_stBit_New.pBuf[i * nLineByteIn + j * nPeixlByteIn + 2]);
			pBuffer[i * nLineByteOut + j] = (INT)(nRed + nGreen + nBlue + 0.5);
		}
	}

	//��ֵ�����㹫ʽ
	for (INT i = NULL; i < nHeightIn; i++)
	{
		for (INT j = NULL; j < nWidthIn; j++)
		{
			if (pBuffer[i * nLineByteOut + j] <= nThreshold)
				pBuffer[i * nLineByteOut + j] = 0;
			else
				pBuffer[i * nLineByteOut + j] = 255;
		}
	}
	//ƫ��
	m_stBit_New.pBitFile->bfOffBits = SIZE__OFBITS(nColorTable);
	//�ļ��ܴ�С
	m_stBit_New.pBitFile->bfSize = m_stBit_New.pBitFile->bfOffBits + (nLineByteOut * nHeightIn);
	//λ��
	m_stBit_New.pBitInfo->biBitCount = nBitCountOut;
	m_stBit_New.pBitInfo->biClrUsed = nColorTable;
	m_stBit_New.pBitInfo->biClrImportant = nColorTable;
	m_stBit_New.pBitInfo->biSizeImage = nLineByteOut * nHeightIn;//λͼ���ݴ�С
	//�����滻
	ReplaceBitBuffer(pBuffer, m_pRgb);
}

VOID GrayTran::ColorToGray()
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//��ǰλͼ��λ��
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;

	//�Ѿ��ǻҶ�λͼ�˵Ļ��Ͳ���Ҫ�ٽ���
	if (nBitCountIn == 8)
		return;

	//Դλͼ�Ŀ�Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;

	//����ͼ��� 24 ����Ӧ����3
	INT nPiexlbyteIn = nBitCountIn / 8;
	//8λ
	INT nBitCountOut = 8;
	//����ͼ��һ�г���
	INT nLinebyteIn = (nWidthIn*nPiexlbyteIn + 3) / 4 * 4;
	//���ͼ��һ�г���
	INT nLinebyteOut = (nWidthIn*nBitCountOut / 8 + 3) / 4 * 4;

	//����оɵĵ�ɫ��Ļ�����ʾ�ͷ�
	if (m_pRgb)
	{
		VirtualFree(m_pRgb, NULL, MEM_RELEASE);
		m_pRgb = nullptr;
	}

	//��ȡ��ɫ���Ⱥ������ڴ�
	INT nColorTable = GetColorTable(nBitCountOut);
	m_pRgb = (LPRGBQUAD)VirtualAlloc(NULL, SIZE__PALETTE(nColorTable), MEM_COMMIT, PAGE_READWRITE);
	if (!m_pRgb)
		return;

	//��ɫ������
	for (INT nIndex = NULL; nIndex < nColorTable; nIndex++)
	{
		m_pRgb[nIndex].rgbRed = nIndex;
		m_pRgb[nIndex].rgbGreen = nIndex;
		m_pRgb[nIndex].rgbBlue = nIndex;
		m_pRgb[nIndex].rgbReserved = NULL;
	}

	//�������ͼ����ڴ�
	LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountOut);
	if (!pBuffer)
		return;

	//�Ҷ��㷨
	INT nRed, nGreen, nBlue;
	for (INT i = NULL; i < nHeightIn; i++)
	{
		for (INT j = NULL; j < nWidthIn; j++)
		{
			nRed = (INT)(0.11*m_stBit_New.pBuf[i * nLinebyteIn + j * nPiexlbyteIn + 0]);
			nGreen = (INT)(0.59*m_stBit_New.pBuf[i * nLinebyteIn + j * nPiexlbyteIn + 1]);
			nBlue = (INT)(0.30*m_stBit_New.pBuf[i * nLinebyteIn + j * nPiexlbyteIn + 2]);
			pBuffer[i * nLinebyteOut + j] = (INT)(nRed + nGreen + nBlue + 0.5);
		}
	}
	//λͼ��ʼ��λͼ���ݵ�ƫ��
	m_stBit_New.pBitFile->bfOffBits = SIZE__OFBITS(nColorTable);
	//�ܴ�С
	m_stBit_New.pBitFile->bfSize = m_stBit_New.pBitFile->bfOffBits + (nLinebyteOut * nHeightIn);
	//��ɫλ��
	m_stBit_New.pBitInfo->biBitCount = nBitCountOut;
	m_stBit_New.pBitInfo->biClrUsed = nColorTable;
	m_stBit_New.pBitInfo->biClrImportant = nColorTable;
	m_stBit_New.pBitInfo->biSizeImage = nLinebyteOut * nHeightIn;//λͼ���ݴ�С
	ReplaceBitBuffer(pBuffer, m_pRgb);
}

VOID GrayTran::Reverse()
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	INT nPiexlByeIn = nBitCountIn / 8;
	INT nLineByteOut = (nWidthIn*nBitCountIn / 8 + 3 / 4 * 4);
	LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountIn);
	if (!pBuffer)
		return;

	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightIn; i++)
	{
		for (INT j = NULL; j < nWidthIn; j++)
		{
			nRowTo = i * nLineByteOut;
			nColumnTo = j * nPiexlByeIn;
			nRowFrom = nRowTo;
			nColumnFrom = nColumnTo;

			//��ɫ�ķ�ת
			for (INT n = NULL; n < nPiexlByeIn; n++)
				pBuffer[nRowTo + nColumnTo + n] = 255 - m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];
		}
	}
	ReplaceBitBuffer(pBuffer);
}

BOOL GrayTran::DrawBit(HDC hDc)
{
	if (m_stBit_New.pBitmap)
		return BaseBit::DrawBit(hDc, &m_stBit_New);
	return BaseBit::DrawBit(hDc);
}

BOOL GrayTran::WriteBit(CONST CHAR* szBitPath)
{
	if (m_stBit_New.pBitmap)
		return BaseBit::WriteBit(szBitPath, &m_stBit_New);
	return BaseBit::WriteBit(szBitPath);
}

BOOL GrayTran::LoadBitInfo(PBitInfo pBit)
{
	return TRUE;
}

BOOL GrayTran::BitInfoTo(PBitInfo pBit)
{
	return TRUE;
}
