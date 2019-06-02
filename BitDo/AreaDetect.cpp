
#include "AreaDetect.h"



BOOL AreaDetect::ReplaceBitBuffer(CONST LPBYTE pBit, LPRGBQUAD pRGB /*= nullptr*/)
{
	//位图的数据指针不能为空
	if (!pBit)
		return FALSE;

	//保存一下位图的其它数据，因为我们要替换位图的RGB数据和调色板数据而已
	BITMAPFILEHEADER stBitFile = { 0 };
	BITMAPINFOHEADER stBitInfo = { 0 };
	LPRGBQUAD pRgb = nullptr;

	//先保存一下位图的数据
	CopyMemory(&stBitFile, m_stBit_New.pBitFile, SIZE__FILE);
	CopyMemory(&stBitInfo, m_stBit_New.pBitInfo, SIZE__INFO);
	//获取颜色表长度
	INT nColorTable = GetColorTable(m_stBit_New.pBitInfo->biBitCount);
	if (nColorTable)//如果有调色板
	{
		//申请内存
		pRgb = (LPRGBQUAD)VirtualAlloc(NULL, SIZE__PALETTE(nColorTable), MEM_COMMIT, PAGE_READWRITE);
		if (!pRgb)
			return FALSE;

		if (m_stBit_New.pRGBTable)//原本有调色板
			CopyMemory(pRgb, m_stBit_New.pRGBTable, SIZE__PALETTE(nColorTable));
		if (pRGB)//如果传进来了调色板指针，就设置成进来的
			CopyMemory(pRgb, pRGB, SIZE__PALETTE(nColorTable));

	}

	//位图数据大小
	INT nSize = stBitInfo.biSizeImage;

	//总大小
	INT nSumSize = m_stBit_New.pBitFile->bfOffBits + m_stBit_New.pBitInfo->biSizeImage;

	//清空所有内存
	ClearBitInfo(&m_stBit_New);

	//申请内存再保存再保存
	LPBYTE pBitmap = (LPBYTE)VirtualAlloc(NULL, nSumSize, MEM_COMMIT, PAGE_READWRITE);
	if (!pBitmap)
	{
		if (pRgb)
			VirtualFree(pRgb, NULL, MEM_RELEASE);
		return FALSE;
	}

	m_stBit_New.pBitmap = pBitmap;
	InitBitInfo(&m_stBit_New, stBitInfo.biBitCount);

	//数据的拷贝
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

	//位图的总大小
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

	//总大小
	INT nSize = pBit->pBitFile->bfOffBits + pBit->pBitInfo->biSizeImage;

	LPBYTE pBuffer = (LPBYTE)VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE);
	if (!pBuffer)
		return FALSE;

	//总数据的复制
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
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//源位图的宽度 高度 
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
				//x方向梯度
				nIndexX = m_stBit_New.pBuf[i*nLineByeIn + j * nPeixlByteIn + n] -
					m_stBit_New.pBuf[(i + 1)*nLineByeIn + j * nPeixlByteIn + n];

				//y方向梯度
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
