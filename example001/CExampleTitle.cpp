#include <windows.h>
#include "CExample.h"


////////////////////////////////////////////////////////////////
#define BMP_TITLE_BG   L"bmp\\title_bg.bmp"
#define BMP_TITLE_LOGO L"bmp\\title.bmp"
#define BMP_TITLE_SP   L"bmp\\title_sp.bmp"

#define CURSOR_MENU_START  0
#define CURSOR_MENU_QUIT   1

#define MYCURSOR_SIZE     64
#define GET_MYCURSOR_POS_Y(Y)  (356+Y*96)


////////////////////////////////////////////////////////////////
// タイトル画面クラス
////////////////////////////////////////////////////////////////
// コンストラクタ
CMyExampleTitle::CMyExampleTitle()
{
    taskNext = MYMAINTASK::TASK_TITLEMENU;

    // 画像
    pBG = NULL;         // 背景
    pLogo = NULL;       // タイトルロゴ
    pSprite = NULL;     // スプライト
    pOffscreen = NULL;  // オフスクリーン

    // フォント
    hfont32 = NULL;
    hfont64 = NULL;
    hfont16 = NULL;
}

////////////////////////////////////////////////////////////////
// 初期化
BOOL CMyExampleTitle::InitTitle()
{
    // 背景画像読み込み
    pBG = new CDib24(this);
    if(pBG == NULL)
    {
        return FALSE;
    }

    if(pBG->ReadBMPFile(BMP_TITLE_BG) == FALSE)
    {
        return FALSE;
    }

    // タイトルロゴ画像読み込み
    pLogo = new CDib24(this);
    if(pLogo == NULL)
    {
        return FALSE;
    }

    if(pLogo->ReadBMPFile(BMP_TITLE_LOGO) == FALSE)
    {
        return FALSE;
    }

    // カーソル画像読み込み
    pSprite = new CDib24(this);
    if(pSprite == NULL)
    {
        return FALSE;
    }

    if(pSprite->ReadBMPFile(BMP_TITLE_SP) == FALSE)
    {
        return FALSE;
    }

    // オフスクリーン
    pOffscreen = new CDib24(this);
    if(pOffscreen == NULL)
    {
        return FALSE;
    }

    if(pOffscreen->CreateDibObject(GAMESCREEN_WIDTH, GAMESCREEN_HEIGHT) == FALSE)
    {
        return FALSE;
    }

    // 透明色を設定
    pSprite->SetTransparentColor(0, 0);
    pOffscreen->SetTransparentColor(pSprite->GetTransparentColor());

    // フォント
    hfont16 = CreateFontW(
        16, 0, 0, 0,
        FW_DONTCARE,
        FALSE, FALSE, FALSE,
        SHIFTJIS_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH,
        L"ＭＳ ゴシック"
    );

    hfont32 = CreateFontW(
        32, 0, 0, 0,
        FW_DONTCARE,
        FALSE, FALSE, FALSE,
        SHIFTJIS_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH,
        L"ＭＳ ゴシック"
    );

    hfont64 = CreateFontW(
        64, 0, 0, 0,
        FW_DONTCARE,
        FALSE, FALSE, FALSE,
        SHIFTJIS_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH,
        L"ＭＳ ゴシック"
    );

    // スプライト
    spriteCursor.nState = CURSOR_MENU_START;
    spriteCursor.rcImage.left = 0;
    spriteCursor.rcImage.top = 0;
    spriteCursor.rcImage.right = MYCURSOR_SIZE;
    spriteCursor.rcImage.bottom = MYCURSOR_SIZE;
    spriteCursor.nPosX = 24;
    spriteCursor.nPosY = GET_MYCURSOR_POS_Y(spriteCursor.nState);
    spriteCursor.nCurrentCell = 0;
    spriteCursor.nMaxCell = 8;
    spriteCursor.dwDelay = 125;
    spriteCursor.dwLastTickCount = GetTickCount();

    // 選択メニュー
    selcurMenu[0].nWidth = 224;
    selcurMenu[0].nHeight = 64;
    selcurMenu[0].nPosX = spriteCursor.nPosX + MYCURSOR_SIZE;
    selcurMenu[0].nPosY = GET_MYCURSOR_POS_Y(0);
    selcurMenu[0].nValue = MYMAINTASK::TASK_GAMEINIT;
    selcurMenu[0].nState = 1;
    selcurMenu[0].byAlpha = 128;
    selcurMenu[0].rgbText[0] = RGB(112, 112, 127);
    selcurMenu[0].rgbText[1] = RGB(240, 240, 254);
    selcurMenu[0].rgbBack[0] = RGB(192, 192, 255);
    selcurMenu[0].rgbBack[1] = RGB(192, 192, 255);
    selcurMenu[0].lpszText = L"NewGame";

    selcurMenu[1].nWidth = 224;
    selcurMenu[1].nHeight = 64;
    selcurMenu[1].nPosX = spriteCursor.nPosX + MYCURSOR_SIZE;
    selcurMenu[1].nPosY = GET_MYCURSOR_POS_Y(1);
    selcurMenu[1].nValue = MYMAINTASK::TASK_QUIT;
    selcurMenu[1].nState = 0;
    selcurMenu[1].byAlpha = 128;
    selcurMenu[1].rgbText[0] = RGB(112, 112, 127);
    selcurMenu[1].rgbText[1] = RGB(240, 240, 254);
    selcurMenu[1].rgbBack[0] = RGB(192, 192, 255);
    selcurMenu[1].rgbBack[1] = RGB(192, 192, 255);
    selcurMenu[1].lpszText = L"終了";

    //
    wFPS = 0;
    memset(szFPS, 0, sizeof(szFPS));
    wcscpy(szFPS, L"FPS:");
    dwFPSTickCount = GetTickCount();

    return TRUE;
}

////////////////////////////////////////////////////////////////
// 解放処理
void CMyExampleTitle::FreeTitle()
{
    FreeCDib(pBG);
    FreeCDib(pLogo);
    FreeCDib(pSprite);
    FreeCDib(pOffscreen);
    DeleteObject(hfont16);
    DeleteObject(hfont32);
    DeleteObject(hfont64);
}

void CMyExampleTitle::FreeCDib(CDib24* ptr)
{
    if(ptr != NULL)
    {
        delete ptr;
        ptr = NULL;
    }
}

////////////////////////////////////////////////////////////////
// オフスクリーン消去
void CMyExampleTitle::EraseOffscreen()
{
    PatBlt(hMemOffscreen, 0, 0, myenv.nWidth, myenv.nHeight, BLACKNESS);
}

////////////////////////////////////////////////////////////////
// オフスクリーン初期化
void CMyExampleTitle::InitOffscreen()
{
    EraseOffscreen();

    // 固定文言表示
    DrawChar(hfont32, 408,  48, L"テンキー8:", RGB(254,254,254), RGB(127,127,254));
    DrawChar(hfont32, 416,  96, L"選択カーソルを上に移動", RGB(254,254,254), RGB(127,127,254));
    DrawChar(hfont32, 408, 192, L"テンキー2:", RGB(254,254,254), RGB(127,127,254));
    DrawChar(hfont32, 416, 240, L"選択カーソルを下に移動", RGB(254,254,254), RGB(127,127,254));
    DrawChar(hfont32, 408, 336, L"テンキー5:", RGB(254,254,254), RGB(127,127,254));
    DrawChar(hfont32, 416, 384, L"決定", RGB(254,254,254), RGB(127,127,254));
    DrawChar(hfont32, 408, 480, L"マウス操作対応", RGB(254,254,254), RGB(127,127,254));
}

////////////////////////////////////////////////////////////////
// フレーム更新
void CMyExampleTitle::UpdateFrame(DWORD dwTickCount, BOOL bDrawSkip)
{
    CheckKeyInput();   // キー入力チェック

    // 処理が間に合わなければ描画はスキップ
    if(bDrawSkip == TRUE)
    {
        return;
    }

    DrawBackground();  // 背景描画
    DrawSprite(dwTickCount);  // スプライト描画
    pOffscreen->DrawBits(hMemOffscreen, 0, 0);  // 仮想ウィンドウに描画
    DrawChar(hfont64, selcurMenu[0].nPosX, selcurMenu[0].nPosY, selcurMenu[0].lpszText, selcurMenu[0].rgbText[selcurMenu[0].nState]);  // メニュー文字描画
    DrawChar(hfont64, selcurMenu[1].nPosX, selcurMenu[1].nPosY, selcurMenu[1].lpszText, selcurMenu[1].rgbText[selcurMenu[1].nState]);  // メニュー文字描画
    InvalidateRect(hwndExample, NULL, FALSE);  // 画面表示

    // FPS表示
    DrawChar(hfont16, 336, 40, szFPS, RGB(254,254,254), RGB(0,0,0));
    wFPS++;
    if(dwTickCount - dwFPSTickCount >= 1000)
    {
        szFPS[4] = L'0' + wFPS / 100;
        szFPS[5] = L'0' + wFPS % 100 / 10;
        szFPS[6] = L'0' + wFPS % 10;

        dwFPSTickCount = dwTickCount - (dwTickCount - dwFPSTickCount - 1000);
        wFPS = 0;
    }
}

////////////////////////////////////////////////////////////////
// キー入力チェック
void CMyExampleTitle::CheckKeyInput()
{
    int i;
    POINT pt;
    BOOL bMouseOver = FALSE;

    // キー入力状態をセット
    myKeyInput.SetMyKeyState();

    ////////////////////////////////////////////
    // メニューカーソルのポジション判定
    ////////////////////////////////////////////
    // マウスカーソルの座標をチェック
    if((myKeyInput.GetMouseCoord((int&)pt.x, (int&)pt.y) == TRUE)  // カーソルが移動している
    && (ScreenToClient(hwndExample, &pt))                          // かつ、ウィンドウ内にカーソルがある
    && (pt.x > 0 && pt.y > 0 && pt.x < myenv.nWidth && pt.y < myenv.nHeight))
    {
        // カーソルON/OFF判定
        for(i = 0; i <= CURSOR_MENU_QUIT; i++)
        {
            if(pt.x < selcurMenu[i].nPosX || pt.x >= selcurMenu[i].nPosX + selcurMenu[i].nWidth
            || pt.y < selcurMenu[i].nPosY || pt.y >= selcurMenu[i].nPosY + selcurMenu[i].nHeight)
            {
                selcurMenu[i].nState = 0;
            }
            else
            {
                bMouseOver = TRUE;
                selcurMenu[i].nState = 1;
                spriteCursor.nState = i;
            }
        }
    }

    // キーボード入力のチェック
    if(bMouseOver == FALSE)
    {
        selcurMenu[spriteCursor.nState].nState = 1;

        // テンキー8押下の場合、"NewGame"を選択
        if(myKeyInput.CheckMyKeyState(VK_NUMPAD8, MYKEYSTATE::PUSH) == TRUE)
        {
            spriteCursor.nState = CURSOR_MENU_START;
            selcurMenu[CURSOR_MENU_START].nState = 1;
            selcurMenu[CURSOR_MENU_QUIT].nState = 0;
        }

        // テンキー2押下の場合、"終了"を選択
        if(myKeyInput.CheckMyKeyState(VK_NUMPAD2, MYKEYSTATE::PUSH) == TRUE)
        {
            spriteCursor.nState = CURSOR_MENU_QUIT;
            selcurMenu[CURSOR_MENU_QUIT].nState = 1;
            selcurMenu[CURSOR_MENU_START].nState = 0;
        }
    }

    ////////////////////////////////////////////
    // 決定キーの押下判定
    ////////////////////////////////////////////
    // テンキー5 or 左クリック
    if(myKeyInput.CheckMyKeyState(VK_NUMPAD5, MYKEYSTATE::PUSH) == TRUE
    || myKeyInput.CheckMyKeyState(VK_LBUTTON, MYKEYSTATE::PUSH) == TRUE)
    {
        taskNext = (MYMAINTASK)selcurMenu[spriteCursor.nState].nValue;
    }

    // カレントキーの添え字を切替
    myKeyInput.SwitchCurrentIndex();
}

////////////////////////////////////////////////////////////////
// 背景描画
void CMyExampleTitle::DrawBackground()
{
    int x, y;
    int nSizeX, nSizeY;
    RECT rcSrc, rcDst;

    // タイル描画
    nSizeX = pBG->GetCDibWidth();
    nSizeY = pBG->GetCDibHeight();

    rcSrc.left = rcSrc.top = 0;
    rcSrc.right = nSizeX;
    rcSrc.bottom = nSizeY;

    for(y = 0; y < GAMESCREEN_HEIGHT; y += nSizeY)
    {
        for(x = 0; x < GAMESCREEN_WIDTH; x += nSizeX)
        {
            // 描画先の座標をセット
            rcDst.left = x;
            rcDst.top = y;
            rcDst.right = rcDst.left + nSizeX;
            rcDst.bottom = rcDst.top + nSizeY;

            // 領域外を切り取り
            if(rcDst.right > GAMESCREEN_WIDTH)
            {
                rcDst.right = GAMESCREEN_WIDTH;
            }

            if(rcDst.bottom > GAMESCREEN_HEIGHT)
            {
                rcDst.bottom = GAMESCREEN_HEIGHT;
            }

            // 描画
            pOffscreen->CopyDibBits(*pBG, rcSrc, rcDst, FALSE);
        }
    }

    // ロゴ描画
    nSizeX = pLogo->GetCDibWidth();
    nSizeY = pLogo->GetCDibHeight();

    rcSrc.left = rcSrc.top = 0;
    rcSrc.right = nSizeX;
    rcSrc.bottom = nSizeY;

    rcDst.left = (GAMESCREEN_WIDTH - nSizeX) / 2;
    rcDst.top = 64;
    rcDst.right = rcDst.left + nSizeX;
    rcDst.bottom = rcDst.top + nSizeY;

    pOffscreen->CopyDibBits(*pLogo, rcSrc, rcDst, FALSE);
}

////////////////////////////////////////////////////////////////
// スプライト描画
void CMyExampleTitle::DrawSprite(DWORD dwTickCount)
{
    RECT rcSrc, rcDst;
    DWORD dwColor;

    // スプライト表示
    if(dwTickCount - spriteCursor.dwLastTickCount >= spriteCursor.dwDelay)
    {
        spriteCursor.nCurrentCell = (spriteCursor.nCurrentCell + 1) % spriteCursor.nMaxCell;
        spriteCursor.dwLastTickCount = dwTickCount;
    }

    rcSrc = spriteCursor.rcImage;
    rcSrc.left = spriteCursor.nCurrentCell * MYCURSOR_SIZE;
    rcSrc.right = rcSrc.left + MYCURSOR_SIZE;

    rcDst.left = spriteCursor.nPosX;
    rcDst.right = rcDst.left + MYCURSOR_SIZE;
    rcDst.top = GET_MYCURSOR_POS_Y(spriteCursor.nState);
    rcDst.bottom = rcDst.top + MYCURSOR_SIZE;

    pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);

    // メニュー表示
    rcDst.left = selcurMenu[0].nPosX;
    rcDst.top  = selcurMenu[0].nPosY;
    rcDst.right = rcDst.left + selcurMenu[0].nWidth;
    rcDst.bottom = rcDst.top + selcurMenu[0].nHeight;
    pOffscreen->DrawRectangle(rcDst, selcurMenu[0].rgbBack[selcurMenu[0].nState], selcurMenu[0].byAlpha);

    rcDst.left = selcurMenu[1].nPosX;
    rcDst.top  = selcurMenu[1].nPosY;
    rcDst.right = rcDst.left + selcurMenu[1].nWidth;
    rcDst.bottom = rcDst.top + selcurMenu[1].nHeight;
    pOffscreen->DrawRectangle(rcDst, selcurMenu[1].rgbBack[selcurMenu[1].nState], selcurMenu[1].byAlpha);
}

////////////////////////////////////////////////////////////////
// 文字列描画
void CMyExampleTitle::DrawChar(HFONT& hfont, int x, int y, LPCWSTR lpszText, COLORREF rgbText)
{
    HFONT hFontOld;

    SetBkMode(hMemOffscreen, TRANSPARENT);
    hFontOld = (HFONT)SelectObject(hMemOffscreen, hfont);
    SetTextColor(hMemOffscreen, rgbText);
    TextOutW(hMemOffscreen, x, y, lpszText, wcslen(lpszText));
    SelectObject(hMemOffscreen, hFontOld);
}

void CMyExampleTitle::DrawChar(HFONT& hfont, int x, int y, LPCWSTR lpszText, COLORREF rgbText, COLORREF rgbBorder)
{
    HFONT hFontOld;

    SetBkMode(hMemOffscreen, TRANSPARENT);
    hFontOld = (HFONT)SelectObject(hMemOffscreen, hfont);

    SetTextColor(hMemOffscreen, rgbBorder);
    TextOutW(hMemOffscreen, x, y - 1, lpszText, wcslen(lpszText));
    TextOutW(hMemOffscreen, x + 1, y, lpszText, wcslen(lpszText));
    TextOutW(hMemOffscreen, x, y + 1, lpszText, wcslen(lpszText));
    TextOutW(hMemOffscreen, x - 1, y, lpszText, wcslen(lpszText));

    SetTextColor(hMemOffscreen, rgbText);
    TextOutW(hMemOffscreen, x, y, lpszText, wcslen(lpszText));

    SelectObject(hMemOffscreen, hFontOld);
}

