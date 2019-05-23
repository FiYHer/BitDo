
#include <windows.h>
#include "BaseBit.h"
#include "Geometry.h"
#include "GrayTran.h"

GrayTran g_Bit;

HRESULT CALLBACK _Proc(HWND hWnd, UINT uMsg, WPARAM wPara, LPARAM lParam)
{
	static HDC hDc = NULL;
	static PAINTSTRUCT stPs = { 0 };
	switch (uMsg)
	{
	case WM_CREATE:
		g_Bit.ReadBit("H:\\test.bmp");
		break;
	case WM_LBUTTONDOWN:
		g_Bit.Reverse();
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_RBUTTONDOWN:
		if (g_Bit.WriteBit("H:\\test.bmp"))
			MessageBoxA(0, "保存文件成功", 0, 0);
		break;
	case WM_PAINT:
		hDc = BeginPaint(hWnd, &stPs);
		g_Bit.DrawBit(hDc);
		EndPaint(hWnd, &stPs);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
	default:
		return DefWindowProcA(hWnd, uMsg, wPara, lParam);
	}
	return 1;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	WNDCLASSEXA stWindow = { 0 };
	stWindow.cbSize = sizeof(WNDCLASSEXA);
	stWindow.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	stWindow.hCursor = LoadCursorA(NULL, IDC_ARROW);
	stWindow.hInstance = hInstance;
	stWindow.lpfnWndProc = _Proc;
	stWindow.lpszClassName = "Text";
	stWindow.style = CS_VREDRAW | CS_HREDRAW;
	if (!RegisterClassExA(&stWindow))
	{
		return -1;
	}

	HWND hWnd = CreateWindowExA(NULL, "Text", "Text", 
		WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return -1;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG stMsg = { 0 };
	while (GetMessageA(&stMsg, 0, 0, 0))
	{
		TranslateMessage(&stMsg);
		DispatchMessageA(&stMsg);
	}
	return stMsg.wParam;
}