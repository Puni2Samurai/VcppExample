#include <windows.h>
#include <stdlib.h>  // rand
#include <time.h>    // time
#include "CExample.h"

extern LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);

LONG GetMyRandVal(LONG lMin, LONG lMax)
{
    LONG lRand, lRange;

    do
    {
        lRand = (LONG)rand();
    } while(lRand == RAND_MAX);

    lRange = lMax - lMin + 1;
    lRand = (LONG)((DOUBLE)lRand / RAND_MAX * lRange) + lMin;

    return lRand;
}

////////////////////////////////////////////////////////////////
// アプリ共通クラス
////////////////////////////////////////////////////////////////
// エラーダイアログ表示
BOOL CMyExample::DispErrorMsg(LPCWSTR lpszCaption, LPCWSTR lpszMessage)
{
    MessageBoxW(hwnd, lpszMessage, lpszCaption, MB_OK);
    return FALSE;
}

////////////////////////////////////////////////////////////////
// アプリ固有クラス
////////////////////////////////////////////////////////////////
// ウィンドウ生成
BOOL CMyExample::CreateExampleWindow(HINSTANCE hAppInst, int nShowCmd, int nWidth, int nHeight)
{
    hThisInst = hAppInst;
    nWinMode = nShowCmd;
    nWindowWidth = nWidth;
    nWindowHeight = nHeight;

    WNDCLASSEXA wcl;
    const char szWinName[] = "ExampleWin";
    const char szWinTitle[] = "Example: Mouse & Keyboard";

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
    srand((UINT)time(NULL));

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
    sprite.rcImage.left = sprite.rcImage.top = 0;
    sprite.rcImage.right = sprite.rcImage.bottom = 64;
    sprite.nPosX = (pDibOffScreen->GetCDibWidth()  - 64) / 2;
    sprite.nPosY = (pDibOffScreen->GetCDibHeight() - 64) / 2;
    sprite.nMoveX = sprite.nMoveY = 0;
    sprite.nState = SPRITE_STATE_INIT;
    sprite.nCurrentCell = 0;
    sprite.nMaxCell = 1;
    sprite.dwDelay = SPRITE_DELAY;
    sprite.dwLastTickCount = GetTickCount();
    byAlphaBlend = 0;
    nAlphaBlendInc = 8;

    dwLastTickCount = sprite.dwLastTickCount;

    return TRUE;
}

////////////////////////////////////////////////////////////////
// 解放処理
void CMyExample::FreeExample()
{
    if(pDibSprite != NULL)
    {
        delete pDibSprite;
        pDibSprite = NULL;
    }

    if(pDibBackground != NULL)
    {
        delete pDibBackground;
        pDibBackground = NULL;
    }

    if(pDibOffScreen != NULL)
    {
        delete pDibOffScreen;
        pDibOffScreen = NULL;
    }
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
// オフスクリーンへ自スプライトの描画
void CMyExample::DrawSprite(DWORD dwTickCount)
{
    RECT rcDst;

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

    // スプライトの表示セルをセット
    sprite.rcImage.left = sprite.nCurrentCell * 64;
    sprite.rcImage.right = sprite.rcImage.left + 64;

    // スプライトの表示座標をセット
    rcDst.left   = sprite.nPosX;
    rcDst.top    = sprite.nPosY;
    rcDst.right  = rcDst.left + 64;
    rcDst.bottom = rcDst.top  + 64;

    // 画面外にはみ出る部分を切り取る
    if(rcDst.right > pDibOffScreen->GetCDibWidth())
    {
        rcDst.right = pDibOffScreen->GetCDibWidth();
    }

    if(rcDst.bottom > pDibOffScreen->GetCDibHeight())
    {
        rcDst.bottom = pDibOffScreen->GetCDibHeight();
    }

    if(rcDst.left < 0)
    {
        sprite.rcImage.left += -rcDst.left;
        rcDst.left = 0;
    }

    if(rcDst.top < 0)
    {
        sprite.rcImage.top += -rcDst.top;
        rcDst.top = 0;
    }

    // 描画
    pDibOffScreen->CopyDibBits(*pDibSprite, sprite.rcImage, rcDst);
}

////////////////////////////////////////////////////////////////
// オフスクリーンへ他スプライトの描画
void CMyExample::DrawOtherSprite(DWORD dwTickCount)
{
    RECT rcSrc, rcDst;
    LONG lSpriteWidth, lSpriteHeight;
    BYTE byAlpha = 255;

    // スプライトの更新周期
    if(dwTickCount - spriteCPU.dwLastTickCount >= spriteCPU.dwDelay)
    {
        spriteCPU.nCurrentCell = (spriteCPU.nCurrentCell + 1) % spriteCPU.nMaxCell;
        spriteCPU.dwLastTickCount = dwTickCount;

        if(spriteCPU.nState == SPRITE_STATE_GLAD)
        {
            if(byAlphaBlend + nAlphaBlendInc > 255)
            {
                byAlphaBlend = 255;
                nAlphaBlendInc = -nAlphaBlendInc;
            }
            else if(byAlphaBlend + nAlphaBlendInc < 0)
            {
                byAlphaBlend = 0;
                nAlphaBlendInc = -nAlphaBlendInc;
            }
            else
            {
                byAlphaBlend += nAlphaBlendInc;
            }
        }
    }

    // スプライトの表示セルをセット
    lSpriteWidth = spriteCPU.rcImage.right - spriteCPU.rcImage.left;
    lSpriteHeight = spriteCPU.rcImage.bottom - spriteCPU.rcImage.top;
    rcSrc.left = spriteCPU.rcImage.left + spriteCPU.nCurrentCell * lSpriteWidth;
    rcSrc.right = rcSrc.left + lSpriteWidth;
    rcSrc.top = spriteCPU.rcImage.top;
    rcSrc.bottom = rcSrc.top + lSpriteHeight;

    // スプライトの表示座標をセット
    rcDst.left   = spriteCPU.nPosX;
    rcDst.top    = spriteCPU.nPosY;
    rcDst.right  = rcDst.left + lSpriteWidth;
    rcDst.bottom = rcDst.top  + lSpriteHeight;

    // 画面外にはみ出る部分を切り取る
    if(rcDst.right > pDibOffScreen->GetCDibWidth())
    {
        rcDst.right = pDibOffScreen->GetCDibWidth();
    }

    if(rcDst.bottom > pDibOffScreen->GetCDibHeight())
    {
        rcDst.bottom = pDibOffScreen->GetCDibHeight();
    }

    if(rcDst.left < 0)
    {
        rcSrc.left += -rcDst.left;
        rcDst.left = 0;
    }

    if(rcDst.top < 0)
    {
        rcSrc.top += -rcDst.top;
        rcDst.top = 0;
    }

    // 透過率設定
    if(spriteCPU.nState == SPRITE_STATE_GLAD)
    {
        byAlpha = byAlphaBlend;
    }

    // 描画
    pDibOffScreen->CopyDibBits(*pDibSprite, rcSrc, rcDst, TRUE, byAlpha);
}

////////////////////////////////////////////////////////////////
// キー入力チェック
void CMyExample::CheckKeyInput()
{
    int nMoveX = 0, nMoveY = 0;
    POINT pt;

    // テンキー押下チェック
    if(mykey.CheckMyKeyState(VK_NUMPAD4, MYKEYSTATE_DOWN))
    {
        nMoveX = -4;
    }

    if(mykey.CheckMyKeyState(VK_NUMPAD6, MYKEYSTATE_DOWN))
    {
        nMoveX = 4;
    }

    if(mykey.CheckMyKeyState(VK_NUMPAD8, MYKEYSTATE_DOWN))
    {
        nMoveY = -4;
    }

    if(mykey.CheckMyKeyState(VK_NUMPAD2, MYKEYSTATE_DOWN))
    {
        nMoveY = 4;
    }

    // スプライト移動
    MoveSpriteCoord(sprite, nMoveX, nMoveY);

    // マウス左クリックチェック
    if(mymouse.CheckButtonState(VK_LBUTTON, MYKEYSTATE_DOWN))
    {
        mymouse.GetMouseCoord((int &)pt.x, (int &)pt.y);
        if((ScreenToClient(hwnd, &pt) == TRUE)
        && (pt.x > 0 && pt.x + 64 < nWindowWidth && pt.y > 0 && pt.y + 64 < nWindowHeight))
        {
            SetSpriteCoord(pt.x, pt.y);
        }
    }

    // ZXキー、もしくは、マウス右クリックチェック
    if(mymouse.CheckButtonState(VK_RBUTTON, MYKEYSTATE_PRESS)
    || mykey.CheckMyKeyState(VK_Z, MYKEYSTATE_PRESS) 
    || mykey.CheckMyKeyState(VK_X, MYKEYSTATE_PRESS))
    {
        ChangeSpriteState();
    }
}

////////////////////////////////////////////////////////////////
// フレームの更新
void CMyExample::UpdateFrame()
{
    DWORD dwTickCount = GetTickCount();

    // 自スプライト状態に応じたフレーム更新
    switch(sprite.nState)
    {
        // 初期状態
        case SPRITE_STATE_INIT:
            if(dwTickCount - dwLastTickCount >= 500)
            {
                dwLastTickCount = dwTickCount;

                // 自スプライトの状態遷移
                sprite.nState = SPRITE_STATE_WALK;
                sprite.dwLastTickCount = dwTickCount;

                // 他スプライトの初期化
                spriteCPU.rcImage.left = 128;
                spriteCPU.rcImage.right = spriteCPU.rcImage.left + 32;
                spriteCPU.rcImage.top = 64;
                spriteCPU.rcImage.bottom = spriteCPU.rcImage.top + 32;
                spriteCPU.nPosX = 0;
                spriteCPU.nPosY = 0;
                spriteCPU.nState = SPRITE_STATE_WALK;
                spriteCPU.nCurrentCell = 0;
                spriteCPU.nMaxCell = 4;
                spriteCPU.dwDelay = 80;
                spriteCPU.dwLastTickCount = dwTickCount;
                SetOtherSpriteMove();
            }

            // 描画
            DrawBackground();
            DrawSprite(dwTickCount);

            InvalidateRect(hwnd, NULL, FALSE);
            break;

        // 操作可能(立ち/歩き)状態
        case SPRITE_STATE_STAY:
        case SPRITE_STATE_WALK:
            // フレーム更新周期
            if(dwTickCount - dwLastTickCount >= MYEXAMPLE_REFLASH_RATE)
            {
                // キー入力状態セット
                mykey.SetMyKeyState();
                mymouse.SetMouseState();

                // キー入力状態チェック
                CheckKeyInput();

                // キー状態遷移
                dwLastTickCount = dwTickCount - (dwTickCount - dwLastTickCount - MYEXAMPLE_REFLASH_RATE);
                mykey.SwitchCurrentIndex();
                mymouse.SwitchCurrentIndex();

                // 描画
                DrawBackground();
                DrawSprite(dwTickCount);
                MoveOtherSpriteCoord();
                DrawOtherSprite(dwTickCount);

                InvalidateRect(hwnd, NULL, FALSE);

                // 衝突判定
                if(CollisionDetect() == TRUE)
                {
                    // 自スプライトの状態遷移
                    sprite.rcImage.left = 0;
                    sprite.rcImage.right = 64;
                    sprite.rcImage.top = 64;
                    sprite.rcImage.bottom = 128;
                    sprite.nState = SPRITE_STATE_GLAD;
                    sprite.nCurrentCell = 0;
                    sprite.nMaxCell = 1;
                    sprite.dwLastTickCount = dwTickCount;

                    // 他スプライトの状態遷移
                    spriteCPU.rcImage.left = 64;
                    spriteCPU.rcImage.right = spriteCPU.rcImage.left + 64;
                    spriteCPU.rcImage.top = 64;
                    spriteCPU.rcImage.bottom = spriteCPU.rcImage.top + 64;
                    spriteCPU.nPosX = sprite.nPosX;
                    spriteCPU.nPosY = sprite.nPosY;
                    spriteCPU.nState = SPRITE_STATE_GLAD;
                    spriteCPU.nCurrentCell = 0;
                    spriteCPU.nMaxCell = 1;
                    spriteCPU.dwDelay = 30;
                    spriteCPU.dwLastTickCount = dwTickCount;
                }
            }
            break;

        // スプライト衝突状態
        case SPRITE_STATE_GLAD:
            if(dwTickCount - dwLastTickCount >= 2000)
            {
                dwLastTickCount = dwTickCount;

                // 自スプライトの状態遷移
                sprite.rcImage.left = 0;
                sprite.rcImage.right = 0;
                sprite.rcImage.top = 0;
                sprite.rcImage.bottom = 64;
                sprite.nState = SPRITE_STATE_WALK;
                sprite.dwLastTickCount = dwTickCount;

                // 他スプライトの初期化
                spriteCPU.rcImage.left = 128;
                spriteCPU.rcImage.right = spriteCPU.rcImage.left + 32;
                spriteCPU.rcImage.top = 64;
                spriteCPU.rcImage.bottom = spriteCPU.rcImage.top + 32;
                spriteCPU.nPosX = ((sprite.nPosX + 32) + (nWindowWidth  / 2)) % nWindowWidth;
                spriteCPU.nPosY = ((sprite.nPosY + 32) + (nWindowHeight / 2)) % nWindowHeight;
                spriteCPU.nState = SPRITE_STATE_WALK;
                spriteCPU.nCurrentCell = 0;
                spriteCPU.nMaxCell = 4;
                spriteCPU.dwDelay = 80;
                spriteCPU.dwLastTickCount = dwTickCount;
                SetOtherSpriteMove();
                byAlphaBlend = 0;
                nAlphaBlendInc = 8;
            }

            // 描画
            DrawBackground();
            DrawSprite(dwTickCount);
            DrawOtherSprite(dwTickCount);

            InvalidateRect(hwnd, NULL, FALSE);
            break;
    }
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

////////////////////////////////////////////////////////////////
// スプライトの座標移動
void CMyExample::MoveSpriteCoord(SPRITE& sp, int nMoveX, int nMoveY)
{
    sp.nPosX += nMoveX;
    sp.nPosY += nMoveY;

    int nSpriteWidth, nSpriteHeight;
    nSpriteWidth  = sp.rcImage.right - sp.rcImage.left;
    nSpriteHeight = sp.rcImage.bottom - sp.rcImage.top;

    if(sp.nPosX < 0)
    {
        sp.nPosX = 0;
        sp.nMoveX = -sp.nMoveX;
    }
    else if(sp.nPosX + nSpriteWidth > nWindowWidth)
    {
        sp.nPosX = nWindowWidth - nSpriteWidth;
        sp.nMoveX = -sp.nMoveX;
    }

    if(sp.nPosY < 0)
    {
        sp.nPosY = 0;
        sp.nMoveY = -sp.nMoveY;
    }
    else if(sp.nPosY + nSpriteHeight > nWindowHeight)
    {
        sp.nPosY = nWindowHeight - nSpriteHeight;
        sp.nMoveY = -sp.nMoveY;
    }
}

////////////////////////////////////////////////////////////////
// 他スプライトの座標移動
void CMyExample::MoveOtherSpriteCoord()
{
    MoveSpriteCoord(spriteCPU, spriteCPU.nMoveX, spriteCPU.nMoveY);
}

////////////////////////////////////////////////////////////////
// 他スプライトの移動量をセット
void CMyExample::SetOtherSpriteMove()
{
    LONG lRand;
    int nMove[12][2] = {
        {-3, -1},
        {-2, -2},
        {-1, -3},
        { 1, -3},
        { 2, -2},
        { 3, -1},
        {-3,  1},
        {-2,  2},
        {-1,  3},
        { 1,  3},
        { 2,  2},
        { 3,  1}
    };

    lRand = GetMyRandVal(0, 11);
    spriteCPU.nMoveX = nMove[lRand][0];
    spriteCPU.nMoveY = nMove[lRand][1];
}

////////////////////////////////////////////////////////////////
// スプライトの衝突判定
BOOL CMyExample::CollisionDetect()
{
    int nMySpWidth, nMySpHeight, nOtherSpWidth, nOtherSpHeight;

    nMySpWidth  = sprite.rcImage.right - sprite.rcImage.left;
    nMySpHeight = sprite.rcImage.bottom - sprite.rcImage.top;
    nOtherSpWidth  = spriteCPU.rcImage.right - spriteCPU.rcImage.left;
    nOtherSpHeight = spriteCPU.rcImage.bottom - spriteCPU.rcImage.top;

    // 自スプライトと他スプライトの衝突判定
    if((sprite.nPosX <= spriteCPU.nPosX + nOtherSpWidth)  && (sprite.nPosX + nMySpWidth  > spriteCPU.nPosX)
    && (sprite.nPosY <= spriteCPU.nPosY + nOtherSpHeight) && (sprite.nPosY + nMySpHeight > spriteCPU.nPosY))
    {
        return TRUE;
    }

    return FALSE;
}

