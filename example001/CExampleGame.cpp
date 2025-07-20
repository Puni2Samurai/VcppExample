#include <windows.h>
#include "CExample.h"


////////////////////////////////////////////////////////////////
#define BMP_GAME_BG  L"bmp\\game_bg.bmp"
#define BMP_GAME_SP  L"bmp\\game_sp.bmp"

#define BG_TILE_SIZE   20
#define BG_TILE_ELEM_X 20
#define BG_TILE_ELEM_Y 30

#define MYPLAYCHAR_SIZE  64
#define GET_MYPLAYCHAR_POS_Y (GAMESCREEN_HEIGHT - MYPLAYCHAR_SIZE - 32)


////////////////////////////////////////////////////////////////
// タイトル画面クラス
////////////////////////////////////////////////////////////////
// コンストラクタ
CMyExampleGame::CMyExampleGame()
{
    taskNext = MYMAINTASK::TASK_GAMEWAIT;

    // 画像
    pBG = NULL;         // 背景
    pSprite = NULL;     // スプライト
    pOffscreen = NULL;  // オフスクリーン

    // フォント
    hfont16 = NULL;
    hfont24 = NULL;
    hfont32 = NULL;
}

////////////////////////////////////////////////////////////////
// 初期化
BOOL CMyExampleGame::InitGame()
{
    // 背景画像読み込み
    pBG = new CDib24(this);
    if(pBG == NULL)
    {
        return FALSE;
    }

    if(pBG->ReadBMPFile(BMP_GAME_BG) == FALSE)
    {
        return FALSE;
    }

    // スプライト画像読み込み
    pSprite = new CDib24(this);
    if(pSprite == NULL)
    {
        return FALSE;
    }

    if(pSprite->ReadBMPFile(BMP_GAME_SP) == FALSE)
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

    hfont24 = CreateFontW(
        24, 0, 0, 0,
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

    ////////////////////////////////////////////
    // スプライト
    ////////////////////////////////////////////
    // 操作キャラ
    spritePlayChar.rcImage = { 0, 0, MYPLAYCHAR_SIZE, MYPLAYCHAR_SIZE };
    spritePlayChar.nPosX = (GAMESCREEN_WIDTH - MYPLAYCHAR_SIZE) / 2;
    spritePlayChar.nPosY = GET_MYPLAYCHAR_POS_Y;
    spritePlayChar.nCurrentCell = 0;
    spritePlayChar.nMaxCell = 1;
    spritePlayChar.dwDelay = 60;
    spritePlayChar.byAlpha = 255;
    spritePlayChar.dwLastTickCount = GetTickCount();

    dwGameLastTickCount = GetTickCount();

    return TRUE;
}

////////////////////////////////////////////////////////////////
// 解放処理
void CMyExampleGame::FreeGame()
{
    FreeCDib(pBG);
    FreeCDib(pSprite);
    DeleteObject(hfont16);
    DeleteObject(hfont24);
    DeleteObject(hfont32);
}

void CMyExampleGame::FreeCDib(CDib24* ptr)
{
    if(ptr != NULL)
    {
        delete ptr;
        ptr = NULL;
    }
}

////////////////////////////////////////////////////////////////
// オフスクリーン消去
void CMyExampleGame::EraseOffscreen()
{
    PatBlt(hMemOffscreen, 0, 0, myenv.nWidth, myenv.nHeight, BLACKNESS);
}

////////////////////////////////////////////////////////////////
// オフスクリーン初期化
void CMyExampleGame::InitOffscreen()
{
    EraseOffscreen();

    // 固定文言表示
    DrawChar(hfont32, 408,  48, L"現在、作成中です...", RGB(254,254,254), RGB(127,254,127));
}

////////////////////////////////////////////////////////////////
// データ初期化
void CMyExampleGame::ResetGameData()
{
    DWORD dwTickCount = GetTickCount();

    taskNext = MYMAINTASK::TASK_GAMEMAIN;

    // 自キャラ初期化
    spritePlayChar.rcImage = { 64, 0, 128, 64 };
    spritePlayChar.dwDelay = 60;
    spritePlayChar.dwLastTickCount = dwTickCount;
    spritePlayChar.nCurrentCell = 0;
    spritePlayChar.nMaxCell = 2;
}

////////////////////////////////////////////////////////////////
// フレーム更新
void CMyExampleGame::UpdateFrame(DWORD dwTickCount, BOOL bDrawSkip)
{
    // キー入力

    // 処理が間に合わなければ描画はスキップ
    if(bDrawSkip == TRUE)
    {
        return;
    }

    DrawBackground();  // 背景描画
    DrawSprite(dwTickCount);  // スプライト描画
    pOffscreen->DrawBits(hMemOffscreen, 0, 0);  // 仮想ウィンドウに描画
    InvalidateRect(hwndExample, NULL, FALSE);  // 画面表示
}

////////////////////////////////////////////////////////////////
// 背景描画
void CMyExampleGame::DrawBackground()
{
    int x, y;
    int nSizeX, nSizeY;
    RECT rcSrc, rcDst;

    // 背景のタイル
    int nTilePattern[BG_TILE_ELEM_Y][BG_TILE_ELEM_X] = {
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },

        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },

        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },

        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },

        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },

        { 0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 2,  7, 9, 9, 9, 9 },
        { 3, 4, 4, 4, 4,  4, 4, 4, 4, 4,  4, 4, 4, 4, 5,  8, 9, 9, 9, 9 },
        { 6, 6, 6, 6, 6,  6, 6, 6, 6, 6,  6, 6, 6, 6, 6,  9, 9, 9, 9, 9 },
        { 6, 6, 6, 6, 6,  6, 6, 6, 6, 6,  6, 6, 6, 6, 6,  9, 9, 9, 9, 9 },
        { 6, 6, 6, 6, 6,  6, 6, 6, 6, 6,  6, 6, 6, 6, 6,  9, 9, 9, 9, 9 }
    };

    // タイル描画
    nSizeX = BG_TILE_SIZE;
    nSizeY = BG_TILE_SIZE;

    for(y = 0; y < BG_TILE_ELEM_Y; y++)
    {
        for(x = 0; x < BG_TILE_ELEM_X; x++)
        {
            if(nTilePattern[y][x] < 0)
            {
                continue;
            }

            // 描画元の座標をセット
            rcSrc.left = nTilePattern[y][x] * nSizeX;
            rcSrc.top = 0;
            rcSrc.right = rcSrc.left + nSizeX;
            rcSrc.bottom = nSizeY;

            // 描画先の座標をセット
            rcDst.left = x * nSizeX;
            rcDst.top = y * nSizeY;
            rcDst.right = rcDst.left + nSizeX;
            rcDst.bottom = rcDst.top + nSizeY;

            // 描画
            pOffscreen->CopyDibBits(*pBG, rcSrc, rcDst, FALSE);
        }
    }
}

////////////////////////////////////////////////////////////////
// スプライト描画
void CMyExampleGame::DrawSprite(DWORD dwTickCount)
{
    RECT rcSrc, rcDst;
    DWORD dwColor;

    // 自キャラ描画
    if(dwTickCount - spritePlayChar.dwLastTickCount >= spritePlayChar.dwDelay)
    {
        spritePlayChar.nCurrentCell = (spritePlayChar.nCurrentCell + 1) % spritePlayChar.nMaxCell;
        spritePlayChar.dwLastTickCount = dwTickCount;
    }

    rcSrc = spritePlayChar.rcImage;
    rcSrc.left = rcSrc.left + spritePlayChar.nCurrentCell * MYPLAYCHAR_SIZE;
    rcSrc.right = rcSrc.left + MYPLAYCHAR_SIZE;

    rcDst.left = spritePlayChar.nPosX;
    rcDst.right = rcDst.left + MYPLAYCHAR_SIZE;
    rcDst.top = GET_MYPLAYCHAR_POS_Y;
    rcDst.bottom = rcDst.top + MYPLAYCHAR_SIZE;

    pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);
}

////////////////////////////////////////////////////////////////
// 文字列描画
void CMyExampleGame::DrawChar(HFONT& hfont, int x, int y, LPCWSTR lpszText, COLORREF rgbText, COLORREF rgbBorder)
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

