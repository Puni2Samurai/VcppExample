#include <windows.h>
#include "CExample.h"

extern LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);

////////////////////////////////////////////////////////////////
// アプリ共通クラス
////////////////////////////////////////////////////////////////
BOOL CMyExample::DispErrorMsg(LPCWSTR lpszCaption, LPCWSTR lpszMessage)
{
    MessageBoxW(hwnd, lpszMessage, lpszCaption, MB_OK);
    return FALSE;
}

////////////////////////////////////////////////////////////////
// アプリ固有クラス
////////////////////////////////////////////////////////////////
// 初期化処理
BOOL CMyExample::CreateExampleWindow(HINSTANCE hAppInst, int nShowCmd, int nWidth, int nHeight)
{
    hThisInst = hAppInst;
    nWinMode = nShowCmd;
    nWindowWidth = nWidth;
    nWindowHeight = nHeight;

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
    rcAppWin.right = nWidth;
    rcAppWin.bottom = nHeight;
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
        nWidth, nHeight,
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
    // スプライトDIBクラス初期化
    pDibSprite = new CDib24(this);
    if(pDibSprite == NULL)
    {
        return FALSE;
    }

    // スプライトBMPファイル読み込み
    if(pDibSprite->ReadBMPFile(L"sprite001.bmp") == FALSE)
    {
        return FALSE;
    }

    // 背景DIBクラス初期化
    pDibBackground = new CDib24(this);
    if(pDibBackground == NULL)
    {
        return FALSE;
    }

    // 背景BMPファイル読み込み
    if(pDibBackground->ReadBMPFile(L"bkgrd001.bmp") == FALSE)
    {
        return FALSE;
    }

    // オフスクリーンクラス初期化
    pDibOffScreen = new CDib24(this);
    if(pDibOffScreen->CreateDibObject((LONG)nWindowWidth, (LONG)nWindowHeight) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////
// オフスクリーンへの描画
void CMyExample::DrawOffScreen()
{
    int x, y;
    RECT rcSrc, rcDst;

    // 透明色を設定
    pDibSprite->SetTransparentColor(0, 0);
    pDibOffScreen->SetTransparentColor(pDibSprite->GetTransparentColor());

    // 背景描画
    rcSrc.left   = 0;
    rcSrc.top    = 0;
    rcSrc.right  = pDibBackground->GetCDibWidth();
    rcSrc.bottom = pDibBackground->GetCDibHeight();

    for(y = 0; y < pDibOffScreen->GetCDibHeight(); y += pDibBackground->GetCDibHeight())
    {
        for(x = 0; x < pDibOffScreen->GetCDibWidth(); x += pDibBackground->GetCDibWidth())
        {
            // 背景の描画位置をセット
            rcDst.left   = x;
            rcDst.top    = y;
            rcDst.right  = rcDst.left + pDibBackground->GetCDibWidth();
            rcDst.bottom = rcDst.top  + pDibBackground->GetCDibHeight();

            // 描画領域がサイズオーバーの場合は補正
            if(rcDst.right > pDibOffScreen->GetCDibWidth())
            {
                rcDst.right = pDibOffScreen->GetCDibWidth();
            }

            if(rcDst.bottom > pDibOffScreen->GetCDibHeight())
            {
                rcDst.bottom = pDibOffScreen->GetCDibHeight();
            }

            // 背景コピー
            pDibOffScreen->CopyDibBits(*pDibBackground, rcSrc, rcDst, FALSE);
        }
    }

    // ウィンドウ中央にスプライト描画
    rcSrc.left   = 64 * 6;
    rcSrc.top    = 0;
    rcSrc.right  = rcSrc.left + 64;
    rcSrc.bottom = rcSrc.top  + 64;

    rcDst.left   = (pDibOffScreen->GetCDibWidth()  - 64) / 2;
    rcDst.top    = (pDibOffScreen->GetCDibHeight() - 64) / 2;
    rcDst.right  = rcDst.left + 64;
    rcDst.bottom = rcDst.top  + 64;

    pDibOffScreen->CopyDibBits(*pDibSprite, rcSrc, rcDst);

    // 描画メッセージ要求
    InvalidateRect(hwnd, NULL, FALSE);
}

////////////////////////////////////////////////////////////////
// 描画内容の更新
void CMyExample::UpdateScreen(HDC hDC)
{
    if(pDibOffScreen != NULL)
    {
        int x, y;

        // 背景の描画
        y = 0;
        while(y < nWindowHeight)
        {
            x = 0;
            while(x < nWindowWidth)
            {
                pDibBackground->DrawBits(hDC, x, y);
                x += pDibBackground->GetCDibWidth();
            }

            y += pDibBackground->GetCDibHeight();
        }

        // スプライトの描画
        pDibOffScreen->DrawBits(hDC, 0, 0);
    }
}

