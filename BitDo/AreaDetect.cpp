
#include "AreaDetect.h"



BOOL AreaDetect::ReplaceBitBuffer(CONST LPBYTE pBit, LPRGBQUAD pRGB /*= nullptr*/)
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
		if (pRgb)
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

	if (pRgb)
		VirtualFree(pRgb, NULL, MEM_RELEASE);
	return TRUE;
}

BOOL AreaDetect::AgainCopyBitInfo()
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

BOOL AreaDetect::DrawBit(HDC hDc)
{
	if (m_stBit_New.pBitmap)
		return BaseBit::DrawBit(hDc, &m_stBit_New);
	return BaseBit::DrawBit(hDc);
}

BOOL AreaDetect::WriteBit(CONST CHAR* szBitPath)
{
	if (m_stBit_New.pBitmap)
		return BaseBit::WriteBit(szBitPath, &m_stBit_New);
	return BaseBit::WriteBit(szBitPath);
}

BOOL AreaDetect::LoadBitInfo(PBitInfo pBit)
{
	if (!pBit)
		return FALSE;

	ClearBitInfo(&m_stBit_New);

	//�ܴ�С
	INT nSize = pBit->pBitFile->bfOffBits + pBit->pBitInfo->biSizeImage;

	LPBYTE pBuffer = (LPBYTE)VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE);
	if (!pBuffer)
		return FALSE;

	//�����ݵĸ���
	CopyMemory(pBuffer, pBit->pBitmap, nSize);
	m_stBit_New.pBitmap = pBuffer;
	InitBitInfo(&m_stBit_New);
	return TRUE;

}

PBitInfo AreaDetect::GetBitInfo()
{
	return &m_stBit_New;
}

AreaDetect::AreaDetect():BaseBit()
{

}

AreaDetect::~AreaDetect()
{

}

VOID AreaDetect::Roberts()
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//Դλͼ�Ŀ�� �߶� 
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	INT nPeixlByteIn = nBitCountIn / 8;
	INT nLineByeIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;

	LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountIn);
	if (!pBuffer)
		return;

	INT nIndexX, nIndexY, nSum;
	for (INT i = NULL; i < nHeightIn - 1; i++)
	{
		for (INT j = NULL; j < nWidthIn - 1; j++)
		{
			for (INT n = NULL; n < nPeixlByteIn; n++)
			{
				//x�����ݶ�
				nIndexX = m_stBit_New.pBuf[i*nLineByeIn + j * nPeixlByteIn + n] -
					m_stBit_New.pBuf[(i + 1)*nLineByeIn + j * nPeixlByteIn + n];

				//y�����ݶ�
				nIndexY = m_stBit_New.pBuf[i*nLineByeIn + j * nPeixlByteIn + n] -
					m_stBit_New.pBuf[i*nLineByeIn + (j + 1)*nPeixlByteIn + n];

				nSum = (INT)sqrt(nIndexX*nIndexX + nIndexY * nIndexY) + 0.5;
				nSum = (nSum > 255) ? 255 : nSum;

				pBuffer[i*nLineByeIn + j * nPeixlByteIn + n] = nSum;
			}
		}
	}
	ReplaceBitBuffer(pBuffer);
}
