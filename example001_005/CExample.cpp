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
    int x, y, nAdjustWidth, nAdjustHeight;
    DWORD dwStyle;
    RECT rcAppWin, rcDisplay;

    x = y = 0;
    nAdjustWidth = nWidth;
    nAdjustHeight = nHeight;
    dwStyle = WS_CAPTION | WS_VISIBLE | WS_POPUP;
    rcAppWin.left = 0;
    rcAppWin.top = 0;
    rcAppWin.right = nWidth;
    rcAppWin.bottom = nHeight;
    if(AdjustWindowRectEx(&rcAppWin, dwStyle, FALSE, 0) != FALSE)
    {
        nAdjustWidth  = rcAppWin.right - rcAppWin.left;
        nAdjustHeight = rcAppWin.bottom - rcAppWin.top;
        if(SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDisplay, 0) != FALSE)
        {
            x = rcDisplay.left + ((rcDisplay.right - rcDisplay.left) - nAdjustWidth)  / 2;
            y = rcDisplay.top  + ((rcDisplay.bottom - rcDisplay.top) - nAdjustHeight) / 2;
        }
    }

    // ウィンドウ生成
    hwnd = CreateWindowA(
        szWinName,
        szWinTitle,
        dwStyle,
        x, y,
        nAdjustWidth, nAdjustHeight,
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

    // スプライトデータ初期化
    sprite.nPosX = (pDibOffScreen->GetCDibWidth()  - 64) / 2;
    sprite.nPosY = (pDibOffScreen->GetCDibHeight() - 64) / 2;
    sprite.nState = SPRITE_STATE_STAY;
    sprite.nCurrentCell = 0;
    sprite.dwDelay = SPRITE_DELAY;
    sprite.dwLastTickCount = GetTickCount();

    return TRUE;
}

////////////////////////////////////////////////////////////////
// オフスクリーンへ背景の描画
void CMyExample::DrawBackground()
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
}

////////////////////////////////////////////////////////////////
// オフスクリーンへスプライトの描画
void CMyExample::DrawSprite(DWORD& dwTickCount)
{
    RECT rcSrc, rcDst;
    RECT rcSrcPortion, rcDstPortion;
    LONG lPortionX, lPortionY;

    // スプライトの更新周期
    if(dwTickCount - sprite.dwLastTickCount >= sprite.dwDelay)
    {
        // 状態に応じて表示セルを更新
        switch(sprite.nState)
        {
            case SPRITE_STATE_STAY:
                sprite.nCurrentCell = (sprite.nCurrentCell + 1) % SPRITE_STAY_CELL_LEN;
                break;

            case SPRITE_STATE_WALK:
                sprite.nCurrentCell = (sprite.nCurrentCell + 1) % SPRITE_WALK_CELL_LEN;
                break;
        }

        sprite.dwLastTickCount = dwTickCount;
    }

    // スプライトの標示セルをセット
    rcSrc.left   = sprite.nCurrentCell * 64;
    rcSrc.top    = 0;
    rcSrc.right  = rcSrc.left + 64;
    rcSrc.bottom = rcSrc.top  + 64;
    rcSrcPortion = rcSrc;

    // スプライトの表示座標をセット
    rcDst.left   = sprite.nPosX;
    rcDst.top    = sprite.nPosY;
    rcDst.right  = rcDst.left + 64;
    rcDst.bottom = rcDst.top  + 64;
    rcDstPortion = rcDst;

    // 画面外にはみ出る部分を切り取る
    lPortionX = lPortionY = 0;
    if(rcDst.right > pDibOffScreen->GetCDibWidth())
    {
        lPortionX = rcDst.right - pDibOffScreen->GetCDibWidth();
        rcDst.right = pDibOffScreen->GetCDibWidth();
    }

    if(rcDst.bottom > pDibOffScreen->GetCDibHeight())
    {
        lPortionY = rcDst.bottom - pDibOffScreen->GetCDibHeight();
        rcDst.bottom = pDibOffScreen->GetCDibHeight();
    }

    // 描画
    pDibOffScreen->CopyDibBits(*pDibSprite, rcSrc, rcDst);

    // 描画:はみ出た部分(X)
    rcSrc = rcSrcPortion;
    rcDst = rcDstPortion;
    if(lPortionX > 0)
    {
        rcSrc.left = rcSrc.right - lPortionX;
        rcDst.left = 0;
        rcDst.right = lPortionX;
        if(lPortionY > 0)
        {
            rcDst.bottom = pDibOffScreen->GetCDibHeight();
        }

        pDibOffScreen->CopyDibBits(*pDibSprite, rcSrc, rcDst);
    }

    // 描画:はみ出た部分(Y)
    rcSrc = rcSrcPortion;
    rcDst = rcDstPortion;
    if(lPortionY > 0)
    {
        rcSrc.top = rcSrc.bottom - lPortionY;
        rcDst.top = 0;
        rcDst.bottom = lPortionY;
        if(lPortionX > 0)
        {
            rcDst.right = pDibOffScreen->GetCDibWidth();
        }

        pDibOffScreen->CopyDibBits(*pDibSprite, rcSrc, rcDst);
    }

    // 描画:はみ出た部分(X,Y)
    rcSrc = rcSrcPortion;
    rcDst = rcDstPortion;
    if((lPortionX > 0) && (lPortionY > 0))
    {
        rcSrc.left = rcSrc.right - lPortionX;
        rcSrc.top = rcSrc.bottom - lPortionY;
        rcDst.left = rcDst.top = 0;
        rcDst.right = lPortionX;
        rcDst.bottom = lPortionY;

        pDibOffScreen->CopyDibBits(*pDibSprite, rcSrc, rcDst);
    }
}

////////////////////////////////////////////////////////////////
// フレームの更新
void CMyExample::UpdateFrame()
{
    DWORD dwTickCount = GetTickCount();

    DrawBackground();
    DrawSprite(dwTickCount);

    InvalidateRect(hwnd, NULL, FALSE);
}

////////////////////////////////////////////////////////////////
// 描画内容の更新
void CMyExample::UpdateScreen(HDC hDC)
{
    if(pDibOffScreen != NULL)
    {
        pDibOffScreen->DrawBits(hDC, 0, 0);
    }
}

////////////////////////////////////////////////////////////////
// スプライトの状態変更
void CMyExample::ChangeSpriteState()
{
    sprite.nState = 1 - sprite.nState;
}

////////////////////////////////////////////////////////////////
// スプライトの座標変更
void CMyExample::SetSpriteCoord(int nPosX, int nPosY)
{
    sprite.nPosX = nPosX;
    sprite.nPosY = nPosY;
}

void CMyExample::MoveSpriteCoord(int nMoveX, int nMoveY)
{
    sprite.nPosX = (sprite.nPosX + nMoveX + pDibOffScreen->GetCDibWidth())  % pDibOffScreen->GetCDibWidth();
    sprite.nPosY = (sprite.nPosY + nMoveY + pDibOffScreen->GetCDibHeight()) % pDibOffScreen->GetCDibHeight();
}

