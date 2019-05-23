#include "Geometry.h"


BOOL Geometry::ReplaceBitBuffer(CONST LPBYTE pBit,LPRGBQUAD pRGB)
{
	//位图的数据指针不能为空
	if (!pBit)
		return FALSE;

	//暂时存放位图数据
	BITMAPFILEHEADER stBitFile = { 0 };
	BITMAPINFOHEADER stBitInfo = { 0 };
	LPRGBQUAD pRgb = nullptr;

	//先保存一下位图的数据
	CopyMemory(&stBitFile, m_stBit_New.pBitFile, SIZE__FILE);
	CopyMemory(&stBitInfo, m_stBit_New.pBitInfo, SIZE__INFO);
	//颜色表
	INT nColorTable = GetColorTable(m_stBit_New.pBitInfo->biBitCount);
	if (nColorTable)
	{
		//申请内存
		pRgb = (LPRGBQUAD)VirtualAlloc(NULL, SIZE__PALETTE(nColorTable), MEM_COMMIT, PAGE_READWRITE);
		if (!pRgb)
			return FALSE;

		if (m_stBit_New.pRGBTable)//如果有调色板也保存
			CopyMemory(pRgb, m_stBit_New.pRGBTable, SIZE__PALETTE(nColorTable));
		if (pRGB)//传递进来的调色板
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

	//数据的还原
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

	//位图的总大小
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
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//位图的宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//颜色位
	INT nPixelByteIn = nBitCountIn / 8;
	//一行的数据量
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	//申请内存保存位图数据
	LPBYTE pBuffer = AllocBuf(nWidthIn, nHeightIn, nBitCountIn);
	if (!pBuffer)
		return;

	//移动操作
	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightIn; i++)
	{
		for (INT j = NULL; j < nWidthIn; j++)
		{
			//在范围内才能进行操作
			if (i - nY >= NULL && i - nY < nHeightIn &&
				j - nX >= NULL && j - nX < nWidthIn)
			{
				//目标的行索引和列索引
				nRowTo = i * nLineByteIn;
				nColumnTo = j * nPixelByteIn;
				//源的行索引和列索引
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
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//保存原始位图宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//颜色表
	INT nPixelByteIn = nBitCountIn / 8;
	//源位图一行的数据量
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	//缩放后的位图宽度和高度
	INT nWidthOut = (INT)(nWidthIn*dX + 0.5);
	INT nHeightOut = (INT)(nHeightIn*dY + 0.5);
	//缩放后位图的一行数据量
	INT nLineByteOut = (nWidthOut*nBitCountIn / 8 + 3) / 4 * 4;
	//申请内存
	LPBYTE pBuffer = AllocBuf(nWidthOut, nHeightOut, nBitCountIn);
	if (!pBuffer)
		return;

	//待插值位置坐标
	INT nCoodinateX, nCoodinateY;

	INT nRowFrom, nRowTo;
	INT nColumnFrom, nColumnTo;
	for (INT i = NULL; i < nHeightOut; i++)
	{
		for (INT j = NULL; j < nWidthOut; j++)
		{
			//坐标的映射
			nCoodinateX = (INT)(j / dX + 0.5);
			nCoodinateY = (INT)(i / dY + 0.5);

			//行索引和列索引的计算
			nRowTo = i * nLineByteOut;
			nColumnTo = j * nPixelByteIn;
			nRowFrom = nCoodinateY * nLineByteIn;
			nColumnFrom = nCoodinateX * nPixelByteIn;

			//源位图宽度和高度在范围内
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
	//将位图信息保存好，绘制的时候要用到的
	m_stBit_New.pBitInfo->biWidth = nWidthOut;
	m_stBit_New.pBitInfo->biHeight = nHeightOut;
	m_stBit_New.pBitInfo->biSizeImage = nLineByteOut * nHeightOut;
	m_stBit_New.pBitFile->bfSize = nLineByteOut * nHeightOut + m_stBit_New.pBitFile->bfOffBits;
	
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::Level()
{
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//位图宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//位图一行的数据量
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	//颜色位
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
			nColumnFrom = (nWidthIn - 1 - j)*nPexilByteIn;//左右像素的互换

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
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//源位图的宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//位图一行的数据量
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
			nRowFrom = (nHeightIn - 1 - i)*nLineByteIn;//上下像素的互换
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
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//源位图的宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//位图一行的数据量
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	INT nPiexlByteIn = nBitCountIn / 8;
	//输出位图的宽度和高度 【旋转90，位图的宽度和高度互换了】
	INT nWidthOut = nHeightIn;
	INT nHeightOut = nWidthIn;
	//输出位图的一行数据量
	INT nLinebyteOut = (nWidthOut*nBitCountIn / 8 + 3) / 4 * 4;

	//申请指定宽度和高度的内存
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
				// i j代表了行和列，这里互换位置实现旋转效果
				pBuffer[nRowTo + nColumnTo + n] = m_stBit_New.pBuf[nRowFrom + nColumnFrom + n];

			}
		}
	}
	//一定要记得修改位图结构体，不然输出的东西会乱码
	m_stBit_New.pBitInfo->biWidth = nWidthOut;
	m_stBit_New.pBitInfo->biHeight = nHeightOut;
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::AntiClockwise90()
{
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//源位图的宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//源位图一行的数据量
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;
	//位图的颜色位，这里就是24，RGB一个8位
	INT nPiexlByteIn = nBitCountIn / 8;
	//输出位图的宽度和高度
	INT nWidthOut = nHeightIn;
	INT nHeightOut = nWidthIn;
	//输出位图的一行的数据量
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
	//一定要记得修改位图结构体，不然输出的东西会乱码
	m_stBit_New.pBitInfo->biWidth = nWidthOut;
	m_stBit_New.pBitInfo->biHeight = nHeightOut;
	ReplaceBitBuffer(pBuffer);
}

VOID Geometry::Rotate180()
{
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//源位图的宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountBit = m_stBit_New.pBitInfo->biBitCount;
	//源位图的一行数据量
	INT nLineByteIn = (nWidthIn*nBitCountBit / 8 + 3) / 4 * 4;
	INT nPiexlByteIn = nBitCountBit / 8;
	//旋转180，这里的宽度和高度主要是为了上下像素的互换效果
	INT nWidthOut = nWidthIn;
	INT nHeightOut = nHeightIn;
	//宽度和高度没有发生改变
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
			//对角像素的互换实现旋转180
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
	//获取源位图数据
	if (!m_stBit_New.pBuf)
		if (!AgainCopyBitInfo())
			return;

	//源位图的宽度和高度
	INT nWidthIn = m_stBit_New.pBitInfo->biWidth;
	INT nHeightIn = m_stBit_New.pBitInfo->biHeight;
	INT nBitCountIn = m_stBit_New.pBitInfo->biBitCount;
	//颜色位
	INT nPexilByteIn = nBitCountIn / 8;
	//源位图一行的数据量
	INT nLineByteIn = (nWidthIn*nBitCountIn / 8 + 3) / 4 * 4;

	//旋转角度转弧度
	double dRotateAngle = (double)(2 * 3.1415926*nAngle / 360);

	//源位图四个角坐标
	double dOldX_1, dOldX_2, dOldX_3, dOldX_4;
	double dOldY_1, dOldY_2, dOldY_3, dOldY_4;

	//新位图的四个角坐标
	double dNewX_1, dNewX_2, dNewX_3, dNewX_4;
	double dNewY_1, dNewY_2, dNewY_3, dNewY_4;

	//获取旋转角度的正弦
	double dSin = sin(dRotateAngle);
	//获取旋转角度的余弦
	double dCos = cos(dRotateAngle);

	//计算源位图的四个角坐标，以位图中心为原点
	dOldX_1 = (-(nWidthIn - 1) / 2);
	dOldX_2 = ((nWidthIn - 1) / 2);
	dOldX_3 = (-(nWidthIn - 1) / 2);
	dOldX_4 = ((nWidthIn - 1) / 2);

	dOldY_1 = ((nHeightIn - 1) / 2);
	dOldY_2 = ((nHeightIn - 1) / 2);
	dOldY_3 = (-(nHeightIn - 1) / 2);
	dOldY_4 = (-(nHeightIn - 1) / 2);

	//计算输出位图的四个角坐标
	dNewX_1 = dCos * dOldX_1 + dSin * dOldY_1;
	dNewX_2 = dCos * dOldX_2 + dSin * dOldY_2;
	dNewX_3 = dCos * dOldX_3 + dSin * dOldY_3;
	dNewX_4 = dCos * dOldX_4 + dSin * dOldY_4;

	dNewY_1 = -dSin * dOldX_1 + dCos * dOldY_1;
	dNewY_2 = -dSin * dOldX_2 + dCos * dOldY_2;
	dNewY_3 = -dSin * dOldX_3 + dCos * dOldY_3;
	dNewY_4 = -dSin * dOldX_4 + dCos * dOldY_4;

	//输出位图的宽度和高度
	INT nWidthOut = (INT)(max(fabs(dNewX_4 - dNewX_1), fabs(dNewX_3 - dNewX_2)) + 0.5);
	INT nHeightOut = (INT)(max(fabs(dNewY_4 - dNewY_1), fabs(dNewY_3 - dNewY_2)) + 0.5);
	//输出图像一行的数据量
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

			//在范围内
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
