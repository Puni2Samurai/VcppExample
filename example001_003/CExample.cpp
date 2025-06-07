#include <windows.h>
#include "CExample.h"

extern LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);

////////////////////////////////////////////////////////////////
// アプリ固有クラス
////////////////////////////////////////////////////////////////
// 初期化処理
BOOL CMyExample::CreateExampleWindow(HINSTANCE hAppInst, int nShowCmd)
{
	hThisInst = hAppInst;
	nWinMode = nShowCmd;

	WNDCLASSEXA wcl;
    const char szWinName[] = "ExampleWin";
    const char szWinTitle[] = "Example: Device Independent Bitmap";

    // ウィンドウクラス定義
    wcl.hInstance = hThisInst;
    wcl.lpszClassName = szWinName;
    wcl.lpfnWndProc = WindowFunc;
    wcl.style = 0;
    wcl.cbSize = sizeof(WNDCLASSEXA);
    wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcl.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcl.lpszMenuName = NULL;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if(!RegisterClassExA(&wcl))
    {
        return FALSE;
    }

    // ウィンドウ表示位置を算出
	int x, y;
	DWORD dwStyle;
	RECT rcAppWin, rcDisplay;

	x = y = 0;
	dwStyle = WS_CAPTION | WS_VISIBLE | WS_POPUP;
	rcAppWin.left = 0;
	rcAppWin.top = 0;
	rcAppWin.right = 640;
	rcAppWin.bottom = 480;
	if(AdjustWindowRectEx(&rcAppWin, dwStyle, FALSE, 0) != FALSE)
	{
		if(SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDisplay, 0) != FALSE)
		{
			x = rcDisplay.left + ((rcDisplay.right - rcDisplay.left) - (rcAppWin.right - rcAppWin.left)) / 2;
			y = rcDisplay.top  + ((rcDisplay.bottom - rcDisplay.top) - (rcAppWin.bottom - rcAppWin.top)) / 2;
		}
	}

    // ウィンドウ生成
    hwnd = CreateWindowA(
        szWinName,
        szWinTitle,
        dwStyle,
        x, y,
        640, 480,
        HWND_DESKTOP,
        NULL,
        hThisInst,
        NULL
    );

    ShowWindow(hwnd, nWinMode);
    UpdateWindow(hwnd);
    SetFocus(hwnd);

	return TRUE;
}

////////////////////////////////////////////////////////////////
// 初期化処理
BOOL CMyExample::InitExample()
{
	// DIBクラス初期化
	pDibSprite = new CDib24(hwnd);
	if(pDibSprite == NULL)
	{
		return FALSE;
	}

	// BMPファイル読み込み
	if(pDibSprite->ReadBMPFile("example001_003.bmp") == FALSE)
	{
		return FALSE;
	}

	InvalidateRect(hwnd, NULL, FALSE);

	return TRUE;
}

////////////////////////////////////////////////////////////////
// 描画内容の更新
void CMyExample::UpdateScreen(HDC hDC)
{
	if(pDibSprite != NULL)
	{
		pDibSprite->DrawBits(hDC, 0, 0);
	}
}

