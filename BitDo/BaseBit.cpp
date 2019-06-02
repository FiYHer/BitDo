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
	//不能为0
	if (!nWidth || !nHeight || !nBitCount)
		return nullptr;

	//一行的数据量
	INT nLineByte = (nWidth*nBitCount / 8 + 3) / 4 * 4;
	//整个位图的数据量
	INT nSumByte = nLineByte * nHeight;

	//申请内存
	LPBYTE pBuf = (LPBYTE)VirtualAlloc(NULL, nSumByte, MEM_COMMIT, PAGE_READWRITE);
	if (pBuf)
		ZeroMemory(pBuf, nSumByte);
	return pBuf;
}

HPALETTE BaseBit::GetPalette(INT nColorTable, LPRGBQUAD pRgb)
{
	//如果颜色表长度为0就是全彩色位图，不需要这个调色板
	if (!nColorTable)
		return NULL;

	//申请空间填充颜色结构
	LPLOGPALETTE pLogPalette = (LPLOGPALETTE)VirtualAlloc(NULL, 2 * sizeof(WORD) +
		nColorTable * sizeof(PALETTEENTRY), MEM_COMMIT, PAGE_READWRITE);
	if (!pLogPalette)
		return NULL;

	//结构的填充
	pLogPalette->palVersion = 0x300;
	pLogPalette->palNumEntries = nColorTable;
	for (INT nIndex = NULL; nIndex < nColorTable; nIndex++)
	{
		pLogPalette->palPalEntry[nIndex].peRed = pRgb[nIndex].rgbRed;
		pLogPalette->palPalEntry[nIndex].peGreen = pRgb[nIndex].rgbGreen;
		pLogPalette->palPalEntry[nIndex].peBlue = pRgb[nIndex].rgbBlue;
		pLogPalette->palPalEntry[nIndex].peFlags = NULL;
	}

	//创建逻辑调色板句柄
	HPALETTE hPalette = CreatePalette(pLogPalette);
	VirtualFree(pLogPalette, NULL, MEM_RELEASE);
	return hPalette;
}

VOID BaseBit::ClearBitInfo(PBitInfo pBit)
{
	if (!pBit)
		return;

	if (pBit->pBitmap)//位图总数据
		VirtualFree(pBit->pBitmap, NULL, MEM_RELEASE);
	ZeroMemory(pBit, sizeof(BitInfo));
}

VOID BaseBit::InitBitInfo(PBitInfo pBit,
	INT nBitCount)
{
	//结构体指针位空
	if (!pBit)
		return;

	//总位图指针为空
	if (!pBit->pBitmap)
		return;

	LPBYTE pBitmap = pBit->pBitmap;
	pBit->pBitFile = (LPBITMAPFILEHEADER)pBitmap;
	pBit->pBitInfo = (LPBITMAPINFOHEADER)(pBitmap + SIZE__FILE);
	INT nColorTable = GetColorTable(nBitCount);
	if (nColorTable)//如果有调色板的话
		pBit->pRGBTable = (LPRGBQUAD)(pBitmap + SIZE__FILE + SIZE__INFO);
	pBit->pBuf = pBitmap + SIZE__OFBITS(nColorTable);
}

BOOL BaseBit::LoadBitInfo(PBitInfo pBit)
{
	if (!pBit)
		return FALSE;

	ClearBitInfo(&m_pBit);

	//总大小
	INT nSize = pBit->pBitFile->bfOffBits + pBit->pBitInfo->biSizeImage;

	LPBYTE pBuffer = (LPBYTE)VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE);
	if (!pBuffer)
		return FALSE;

	//总数据的复制
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
		//清空所有
		ClearBitInfo(&m_pBit);

		//路径检查
		if (!szBitPath || !strlen(szBitPath))
			break;

		//打开位图文件
		hFile = CreateFileA(szBitPath,GENERIC_READ, 
			FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			break;

		//获取位图的总大小
		nSize = GetFileSize(hFile, NULL);
		if (!nSize)
			break;

		//申请内存
		pBitmap = (LPBYTE)VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE);
		if (!pBitmap)
			break;

		//读取位图的全部数据
		ReadFile(hFile, pBitmap, nSize, &dwByte, NULL);
		if (!dwByte)
			break;

		m_pBit.pBitmap = pBitmap;//位图总数据
		m_pBit.pBitInfo = (LPBITMAPINFOHEADER)(pBitmap + SIZE__FILE);//位图信息头
		InitBitInfo(&m_pBit, m_pBit.pBitInfo->biBitCount);

		//有一些位图信息头的BiSizeImage成员没有的，这里手动设置一下
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
		//文件字符串判断
		if (!szBitPath || !strlen(szBitPath))
			break;

		//创建位图文件文件
		hFile = CreateFileA(szBitPath, GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			break;

		//写入位图数据
		if (pTempBit->pBitmap)
		{
			//为0的情况下无法判断位图数据的大小
			if(!pTempBit->pBitInfo->biSizeImage)
				break;

			//位图总大小
			INT nSize = pTempBit->pBitFile->bfOffBits + pTempBit->pBitInfo->biSizeImage;
			WriteFile(hFile, pTempBit->pBitmap, nSize, &dwByte, NULL);
			if (!dwByte)
				break;
		}

		//马上刷新到文件
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

	//绘制句柄不能位0
	if (!hDc)
		return FALSE;

	//有调色板的话
	if (pRgb)
	{
		//创建一个调色板句柄
		hNewPalettr = GetPalette(GetColorTable(pBitInfo->biBitCount), pRgb);
		if (hNewPalettr)
		{
			//选入DC再初始化一下
			hOldPalette = SelectPalette(hDc, hNewPalettr, TRUE);
			RealizePalette(hDc);
		}
	}

	//设置绘制模式位拉伸模式
	SetStretchBltMode(hDc, COLORONCOLOR);

	//绘制位图数据
	StretchDIBits(hDc, NULL, NULL,
		pBitInfo->biWidth, pBitInfo->biHeight,
		0, 0, pBitInfo->biWidth, pBitInfo->biHeight,
		pBuf, (LPBITMAPINFO)pBitInfo, DIB_RGB_COLORS, SRCCOPY);
	
	//恢复原来的调色板
	if (hOldPalette)
	{
		SelectPalette(hDc, hOldPalette, TRUE);
		DeleteObject(hNewPalettr);
	}
	return TRUE;
}
