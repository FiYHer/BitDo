#include "GrayTran.h"



BOOL GrayTran::ReplaceBitBuffer(CONST LPBYTE pBit, LPRGBQUAD pRGB)
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
		if(pRgb)
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

	//位图的总大小
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
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//如果本身就是8位的位图的话，那就直接二值化
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

	//源位图宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//输出位图的颜色位
	INT nBitCountOut = 8;
	INT nPeixlByteIn = nBitCountIn / 8;
	INT nLineByteIn = (nWidthIn*nPeixlByteIn + 3) / 4 * 4;
	INT nLineByteOut = (nWidthIn*nBitCountOut / 8 + 3) / 4 * 4;

	//如果没释放就先释放
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

	//灰度计算公式
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

	//二值化计算公式
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
	//偏移
	m_stBit_New.pBitFile->bfOffBits = SIZE__OFBITS(nColorTable);
	//文件总大小
	m_stBit_New.pBitFile->bfSize = m_stBit_New.pBitFile->bfOffBits + (nLineByteOut * nHeightIn);
	//位数
	m_stBit_New.pBitInfo->biBitCount = nBitCountOut;
	m_stBit_New.pBitInfo->biClrUsed = nColorTable;
	m_stBit_New.pBitInfo->biClrImportant = nColorTable;
	m_stBit_New.pBitInfo->biSizeImage = nLineByteOut * nHeightIn;//位图数据大小
	//数据替换
	ReplaceBitBuffer(pBuffer, m_pRgb);
}

VOID GrayTran::ColorToGray()
{
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//当前位图的位数
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;

	//已经是灰度位图了的话就不需要再进行
	if (nBitCountIn == 8)
		return;

	//源位图的宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;

	//输入图像的 24 这里应该是3
	INT nPiexlbyteIn = nBitCountIn / 8;
	//8位
	INT nBitCountOut = 8;
	//输入图像一行长度
	INT nLinebyteIn = (nWidthIn*nPiexlbyteIn + 3) / 4 * 4;
	//输出图像一行长度
	INT nLinebyteOut = (nWidthIn*nBitCountOut / 8 + 3) / 4 * 4;

	//如果有旧的调色板的话就显示释放
	if (m_pRgb)
	{
		VirtualFree(m_pRgb, NULL, MEM_RELEASE);
		m_pRgb = nullptr;
	}

	//获取颜色表长度和申请内存
	INT nColorTable = GetColorTable(nBitCountOut);
	m_pRgb = (LPRGBQUAD)VirtualAlloc(NULL, SIZE__PALETTE(nColorTable), MEM_COMMIT, PAGE_READWRITE);
	if (!m_pRgb)
		return;

	//调色板的填充
	for (INT nIndex = NULL; nIndex < nColorTable; nIndex++)
	{
		m_pRgb[nIndex].rgbRed = nIndex;
		m_pRgb[nIndex].rgbGreen = nIndex;
		m_pRgb[nIndex].rgbBlue = nIndex;
		m_pRgb[nIndex].rgbReserved = NULL;
	}

	//申请输出图像的内存
	LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountOut);
	if (!pBuffer)
		return;

	//灰度算法
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
	//位图开始到位图数据的偏移
	m_stBit_New.pBitFile->bfOffBits = SIZE__OFBITS(nColorTable);
	//总大小
	m_stBit_New.pBitFile->bfSize = m_stBit_New.pBitFile->bfOffBits + (nLinebyteOut * nHeightIn);
	//颜色位数
	m_stBit_New.pBitInfo->biBitCount = nBitCountOut;
	m_stBit_New.pBitInfo->biClrUsed = nColorTable;
	m_stBit_New.pBitInfo->biClrImportant = nColorTable;
	m_stBit_New.pBitInfo->biSizeImage = nLinebyteOut * nHeightIn;//位图数据大小
	ReplaceBitBuffer(pBuffer, m_pRgb);
}

VOID GrayTran::Reverse()
{
	//获取源位图数据
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

			//颜色的反转
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
