#include "Geometry.h"


BOOL Geometry::ReplaceBitBuffer(CONST LPBYTE pBit,LPRGBQUAD pRGB)
{
	//λͼ������ָ�벻��Ϊ��
	if (!pBit)
		return FALSE;

	//��ʱ���λͼ����
	BITMAPFILEHEADER stBitFile = { 0 };
	BITMAPINFOHEADER stBitInfo = { 0 };
	LPRGBQUAD pRgb = nullptr;

	//�ȱ���һ��λͼ������
	CopyMemory(&stBitFile, m_stBit_New.pBitFile, SIZE__FILE);
	CopyMemory(&stBitInfo, m_stBit_New.pBitInfo, SIZE__INFO);
	//��ɫ��
	INT nColorTable = GetColorTable(m_stBit_New.pBitInfo->biBitCount);
	if (nColorTable)
	{
		//�����ڴ�
		pRgb = (LPRGBQUAD)VirtualAlloc(NULL, SIZE__PALETTE(nColorTable), MEM_COMMIT, PAGE_READWRITE);
		if (!pRgb)
			return FALSE;

		if (m_stBit_New.pRGBTable)//����е�ɫ��Ҳ����
			CopyMemory(pRgb, m_stBit_New.pRGBTable, SIZE__PALETTE(nColorTable));
		if (pRGB)//���ݽ����ĵ�ɫ��
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

	//���ݵĻ�ԭ
	CopyMemory(m_stBit_New.pBitFile, &stBitFile, SIZE__FILE);
	CopyMemory(m_stBit_New.pBitInfo, &stBitInfo, SIZE__INFO);
	if (pRgb)
		CopyMemory(m_stBit_New.pRGBTable, pRgb, SIZE__PALETTE(nColorTable));
	CopyMemory(m_stBit_New.pBuf, pBit, nSize);

	if (pRgb)
		VirtualFree(pRgb, NULL, MEM_RELEASE);
	return TRUE;
}

BOOL Geometry::AgainCopyBitInfo()
{
	ClearBitInfo(&m_stBit_New);

	//λͼ���ܴ�С
	INT nSize = m_pBit.pBitFile->bfOffBits + m_pBit.pBitInfo->biSizeImage;
	m_stBit_New.pBitmap = (LPBYTE)VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE);
	if (!m_stBit_New.pBitmap)
		return FALSE;
	CopyMemory(m_stBit_New.pBitmap, m_pBit.pBitmap, nSize);
	InitBitInfo(&m_stBit_New);
	return TRUE;
}

BOOL Geometry::WriteBit(CONST CHAR* szBitPath)
{
	if (m_stBit_New.pBitmap)
		return BaseBit::WriteBit(szBitPath, &m_stBit_New);
	return BaseBit::WriteBit(szBitPath);
}

Geometry::Geometry():BaseBit()
{

}


Geometry::~Geometry()
{
	ClearBitInfo(&m_stBit_New);
}

BOOL Geometry::DrawBit(HDC hDc)
{
	if(m_stBit_New.pBitmap)
		return BaseBit::DrawBit(hDc, &m_stBit_New);
	return BaseBit::DrawBit(hDc);
}

BOOL Geometry::LoadBitInfo(PBitInfo pBit)
{
	return TRUE;
}

BOOL Geometry::BitInfoTo(PBitInfo pBit)
{
	return TRUE;
}

VOID Geometry::Move(INT nX, INT nY)
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//λͼ�Ŀ�Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//��ɫλ
	INT nPixelByteIn = nBitCountIn / 8;
	//һ�е�������
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	//�����ڴ汣��λͼ����
	LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountIn);
	if (!pBuffer)
		return;

	//�ƶ�����
	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightIn; i++)
	{
		for (INT j = NULL; j < nWidthIn; j++)
		{
			//�ڷ�Χ�ڲ��ܽ��в���
			if (i - nY >= NULL && i - nY < nHeightIn &&
				j - nX >= NULL && j - nX < nWidthIn)
			{
				//Ŀ�����������������
				nRowTo = i * nLineByteIn;
				nColumnTo = j * nPixelByteIn;
				//Դ����������������
				nRowFrom = (i - nY)* nLineByteIn;
				nColumnFrom = (j - nX)*nPixelByteIn;
				for (INT n = NULL; n < nPixelByteIn; n++)
				{
					pBuffer[nRowTo + nColumnTo + n] = m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];
				}
			}
		}
	}
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::Zoom(double dX, double dY)
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//����ԭʼλͼ��Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//��ɫ��
	INT nPixelByteIn = nBitCountIn / 8;
	//Դλͼһ�е�������
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	//���ź��λͼ��Ⱥ͸߶�
	INT nWidthOut = (INT)(nWidthIn*dX + 0.5);
	INT nHeightOut = (INT)(nHeightIn*dY + 0.5);
	//���ź�λͼ��һ��������
	INT nLineByteOut = (nWidthOut*nBitCountIn / 8 + 3) / 4 * 4;
	//�����ڴ�
	LPBYTE pBuffer = AllocBuf(nWidthOut, nHeightOut, nBitCountIn);
	if (!pBuffer)
		return;

	//����ֵλ������
	INT nCoodinateX, nCoodinateY;

	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightOut; i++)
	{
		for (INT j = NULL; j < nWidthOut; j++)
		{
			//�����ӳ��
			nCoodinateX = (INT)(j / dX + 0.5);
			nCoodinateY = (INT)(i / dY + 0.5);

			//���������������ļ���
			nRowTo = i * nLineByteOut;
			nColumnTo = j * nPixelByteIn;
			nRowFrom = nCoodinateY * nLineByteIn;
			nColumnFrom = nCoodinateX * nPixelByteIn;

			//Դλͼ��Ⱥ͸߶��ڷ�Χ��
			if (NULL <= nCoodinateX && nCoodinateX < nWidthIn &&
				NULL <= nCoodinateY && nCoodinateY < nHeightIn)
			{
				for (INT n = NULL; n < nPixelByteIn; n++)
				{
					pBuffer[nRowTo + nColumnTo + n] = m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];
				}
			}
			else
			{
				for (INT n = NULL; n < nPixelByteIn; n++)
				{
					pBuffer[nRowTo + nColumnTo + n] = 255;
				}
			}
		}
	}
	//��λͼ��Ϣ����ã����Ƶ�ʱ��Ҫ�õ���
	m_stBit_New.pBitInfo->biWidth = nWidthOut;
	m_stBit_New.pBitInfo->biHeight = nHeightOut;
	m_stBit_New.pBitInfo->biSizeImage = nLineByteOut * nHeightOut;
	m_stBit_New.pBitFile->bfSize = nLineByteOut * nHeightOut + m_stBit_New.pBitFile->bfOffBits;
	
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::Level()
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//λͼ��Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//λͼһ�е�������
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	//��ɫλ
	INT nPexilByteIn = m_stBit_New.pBitInfo->biBitCount / 8;

	LPBYTE pBuffer = AllocBuf(nWidthIn,nHeightIn, nBitCountIn);
	if (!pBuffer)
		return;

	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightIn; i++)
	{
		for (INT j = NULL; j < nWidthIn; j++)
		{
			nRowTo = i * nLineByteIn;
			nColumnTo = j * nPexilByteIn;
			nRowFrom = i * nLineByteIn;
			nColumnFrom = (nWidthIn - 1 - j)*nPexilByteIn;//�������صĻ���

			for (INT n = NULL; n < nPexilByteIn; n++)
			{
				pBuffer[nRowTo + nColumnTo + n] = m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];
			}
		}
	}
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::Vertical()
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//Դλͼ�Ŀ�Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//λͼһ�е�������
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	INT nPexilByteIn = nBitCountIn / 8;

	LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountIn);
	if (!pBuffer)
		return;

	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightIn; i++)
	{
		for (INT j = NULL; j < nWidthIn; j++)
		{
			nRowTo = i * nLineByteIn;
			nColumnTo = j * nPexilByteIn;
			nRowFrom = (nHeightIn - 1 - i)*nLineByteIn;//�������صĻ���
			nColumnFrom = j * nPexilByteIn;

			for (INT n = NULL; n < nPexilByteIn; n++)
			{
				pBuffer[nRowTo + nColumnTo + n] = m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];
			}
		}
	}
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::Clockwise90()
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//Դλͼ�Ŀ�Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//λͼһ�е�������
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	INT nPiexlByteIn = nBitCountIn / 8;
	//���λͼ�Ŀ�Ⱥ͸߶� ����ת90��λͼ�Ŀ�Ⱥ͸߶Ȼ����ˡ�
	INT nWidthOut = nHeightIn;
	INT nHeightOut = nWidthIn;
	//���λͼ��һ��������
	INT nLinebyteOut = (nWidthOut*nBitCountIn / 8 + 3) / 4 * 4;

	//����ָ����Ⱥ͸߶ȵ��ڴ�
	LPBYTE pBuffer = AllocBuf(nWidthOut, nHeightOut, nBitCountIn);
	if (!pBuffer)
		return;

	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightOut; i++)
	{
		for (INT j = NULL; j < nWidthOut; j++)
		{
			nRowTo = i * nLinebyteOut;
			nColumnTo = j * nPiexlByteIn;
			nRowFrom = j * nLineByteIn;
			nColumnFrom = (nWidthOut - 1 - i)*nPiexlByteIn;

			for (INT n = NULL; n < nPiexlByteIn; n++)
			{
				// i j�������к��У����ﻥ��λ��ʵ����תЧ��
				pBuffer[nRowTo + nColumnTo + n] = m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];

			}
		}
	}
	//һ��Ҫ�ǵ��޸�λͼ�ṹ�壬��Ȼ����Ķ���������
	m_stBit_New.pBitInfo->biWidth = nWidthOut;
	m_stBit_New.pBitInfo->biHeight = nHeightOut;
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::AntiClockwise90()
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//Դλͼ�Ŀ�Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//Դλͼһ�е�������
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	//λͼ����ɫλ���������24��RGBһ��8λ
	INT nPiexlByteIn = nBitCountIn / 8;
	//���λͼ�Ŀ�Ⱥ͸߶�
	INT nWidthOut = nHeightIn;
	INT nHeightOut = nWidthIn;
	//���λͼ��һ�е�������
	INT nLinebyteOut = (nWidthOut*nBitCountIn / 8 + 3) / 4 * 4;

	LPBYTE pBuffer = AllocBuf(nWidthOut, nHeightOut, nBitCountIn);
	if (!pBuffer)
		return;

	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightOut; i++)
	{
		for (INT j = NULL; j < nWidthOut; j++)
		{
			nRowTo = i * nLinebyteOut;
			nColumnTo = j * nPiexlByteIn;
			nRowFrom = (nHeightIn - 1 - j)*nLineByteIn;
			nColumnFrom = i * nPiexlByteIn;

			for (INT n = NULL; n < nPiexlByteIn; n++)
			{
				pBuffer[nRowTo + nColumnTo + n] = m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];
			}
		}
	}
	//һ��Ҫ�ǵ��޸�λͼ�ṹ�壬��Ȼ����Ķ���������
	m_stBit_New.pBitInfo->biWidth = nWidthOut;
	m_stBit_New.pBitInfo->biHeight = nHeightOut;
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::Rotate180()
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//Դλͼ�Ŀ�Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountBit = m_stBit_New.pBitInfo->biBitCount;
	//Դλͼ��һ��������
	INT nLineByteIn = (nWidthIn*nBitCountBit / 8 + 3) / 4 * 4;
	INT nPiexlByteIn = nBitCountBit / 8;
	//��ת180������Ŀ�Ⱥ͸߶���Ҫ��Ϊ���������صĻ���Ч��
	INT nWidthOut = nWidthIn;
	INT nHeightOut = nHeightIn;
	//��Ⱥ͸߶�û�з����ı�
	LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountBit);
	if (!pBuffer)
		return;

	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightOut; i++)
	{
		for (INT j = NULL; j < nWidthOut; j++)
		{
			nRowTo = i * nLineByteIn;
			nColumnTo = j * nPiexlByteIn;
			//�Խ����صĻ���ʵ����ת180
			nRowFrom = (nHeightIn - 1 - i)*nLineByteIn;
			nColumnFrom = (nWidthIn - 1 - j)*nPiexlByteIn;

			for (INT n = NULL; n < nPiexlByteIn; n++)
			{
				pBuffer[nRowTo + nColumnTo + n] = m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];
			}
		}
	}
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::Rotate(INT nAngle)
{
	//��ȡԴλͼ����
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//Դλͼ�Ŀ�Ⱥ͸߶�
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//��ɫλ
	INT nPexilByteIn = nBitCountIn / 8;
	//Դλͼһ�е�������
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;

	//��ת�Ƕ�ת����
	double dRotateAngle = (double)(2 * 3.1415926*nAngle / 360);

	//Դλͼ�ĸ�������
	double dOldX_1, dOldX_2, dOldX_3, dOldX_4;
	double dOldY_1, dOldY_2, dOldY_3, dOldY_4;

	//��λͼ���ĸ�������
	double dNewX_1, dNewX_2, dNewX_3, dNewX_4;
	double dNewY_1, dNewY_2, dNewY_3, dNewY_4;

	//��ȡ��ת�Ƕȵ�����
	double dSin = sin(dRotateAngle);
	//��ȡ��ת�Ƕȵ�����
	double dCos = cos(dRotateAngle);

	//����Դλͼ���ĸ������꣬��λͼ����Ϊԭ��
	dOldX_1 = (-(nWidthIn - 1) / 2);
	dOldX_2 = ((nWidthIn - 1) / 2);
	dOldX_3 = (-(nWidthIn - 1) / 2);
	dOldX_4 = ((nWidthIn - 1) / 2);

	dOldY_1 = ((nHeightIn - 1) / 2);
	dOldY_2 = ((nHeightIn - 1) / 2);
	dOldY_3 = (-(nHeightIn - 1) / 2);
	dOldY_4 = (-(nHeightIn - 1) / 2);

	//�������λͼ���ĸ�������
	dNewX_1 = dCos * dOldX_1 + dSin * dOldY_1;
	dNewX_2 = dCos * dOldX_2 + dSin * dOldY_2;
	dNewX_3 = dCos * dOldX_3 + dSin * dOldY_3;
	dNewX_4 = dCos * dOldX_4 + dSin * dOldY_4;

	dNewY_1 = -dSin * dOldX_1 + dCos * dOldY_1;
	dNewY_2 = -dSin * dOldX_2 + dCos * dOldY_2;
	dNewY_3 = -dSin * dOldX_3 + dCos * dOldY_3;
	dNewY_4 = -dSin * dOldX_4 + dCos * dOldY_4;

	//���λͼ�Ŀ�Ⱥ͸߶�
	INT nWidthOut = (INT)(max(fabs(dNewX_4 - dNewX_1), fabs(dNewX_3 - dNewX_2)) + 0.5);
	INT nHeightOut = (INT)(max(fabs(dNewY_4 - dNewY_1), fabs(dNewY_3 - dNewY_2)) + 0.5);
	//���ͼ��һ�е�������
	INT nLinebyteOut = (nWidthOut*nBitCountIn / 8 + 3) / 4 * 4;
	LPBYTE pBuffer = AllocBuf(nWidthOut, nHeightOut,nBitCountIn);
	if (!pBuffer)
		return;

	double dPos1 = (-0.5*(nWidthOut - 1)*dCos + 0.5*(nHeightOut - 1)*dSin + 0.5*(nWidthIn - 1));
	double dPos2 = (-0.5*(nWidthOut - 1)*dSin - 0.5*(nHeightOut - 1)*dCos + 0.5*(nHeightIn - 1));

	INT dCoodinateX, dCoodinateY;

	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightOut; i++)
	{
		for (INT j = NULL; j < nWidthOut; j++)
		{
			dCoodinateX = (INT)(j*dCos - i * dSin + dPos1 + 0.5);
			dCoodinateY = (INT)(j*dSin + i * dCos + dPos2 + 0.5);

			nRowTo = i * nLinebyteOut;
			nColumnTo = j * nPexilByteIn;
			nRowFrom = dCoodinateY * nLineByteIn;
			nColumnFrom = dCoodinateX * nPexilByteIn;

			//�ڷ�Χ��
			if ((dCoodinateX >= 0) && (dCoodinateX < nWidthIn) &&
				(dCoodinateY >= 0) && (dCoodinateY < nHeightIn))
			{
				for (INT n = NULL; n < nPexilByteIn; n++)
				{
					pBuffer[nRowTo + nColumnTo + n] = m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];
				}
			}
			else
			{
				for (INT n = NULL; n < nPexilByteIn; n++)
				{
					pBuffer[nRowTo + nColumnTo + n] = 255;
				}
			}
		}
	}
	m_stBit_New.pBitInfo->biWidth = nWidthOut;
	m_stBit_New.pBitInfo->biHeight = nHeightOut;
	m_stBit_New.pBitInfo->biSizeImage = nLinebyteOut * nHeightOut;
	m_stBit_New.pBitFile->bfSize = m_stBit_New.pBitInfo->biSizeImage + m_stBit_New.pBitFile->bfOffBits;
	
	ReplaceBitBuffer(pBuffer);
}
