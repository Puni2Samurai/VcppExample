#include <windows.h>
#include <stdlib.h>    // rand
#include <time.h>      // time
#include "CExample.h"


////////////////////////////////////////////////////////////////
// BMPファイル
#define BMP_GAME_BG  L"bmp\\game_bg.bmp"
#define BMP_GAME_SP  L"bmp\\game_sp.bmp"

// 背景画像
#define BG_TILE_SIZE   20
#define BG_TILE_ELEM_X 20
#define BG_TILE_ELEM_Y 30

// 操作キャラ
#define MYPLAYCHAR_SIZE  64
#define GET_MYPLAYCHAR_POS_Y (GAMESCREEN_HEIGHT - MYPLAYCHAR_SIZE - 32)
#define MYPLAYCHAR_WEAPON_LV_MAX     10
#define MYPLAYCHAR_WEAPON_RED_INDEX   0
#define MYPLAYCHAR_WEAPON_GREEN_INDEX 1
#define MYPLAYCHAR_WEAPON_BLUE_INDEX  2
#define MYPLAYCHAR_LIFE 64

// 操作キャラの状態
enum MYPLAYERCHARACTER_STATE
{
    TURN,    // 振り向き
    STAND,   // 立ち
    ATTACK,  // 攻撃
    MOVE,    // 移動
    LOSE,    // 負け
    MAX,
};

// 操作キャラのイメージ内座標とセル数
int nImageCell_pc[MYPLAYERCHARACTER_STATE::MAX][3] = {
    // x, y, セル数
    { 0,                   0, 1 },  // 振り向き
    { MYPLAYCHAR_SIZE,     0, 1 },  // 立ち
    { MYPLAYCHAR_SIZE,     0, 2 },  // 攻撃
    { MYPLAYCHAR_SIZE * 3, 0, 2 },  // 移動
    { MYPLAYCHAR_SIZE * 5, 0, 1 }   // 負け
};

// アイテム
#define GETITEM_GUN_R   0x00000001
#define GETITEM_GUN_G   0x00000002
#define GETITEM_GUN_B   0x00000004
#define GETITEM_BUDDY1  0x00000010
#define GETITEM_BUDDY2  0x00000020
#define GETITEM_BUDDY3  0x00000040
#define GETITEM_RECOVER 0x00000100

// アイテムのイメージ内座標

// 敵キャラ
#define MYENEMY_WAVE_INTERVAL 5000  // 暫定

// ゲームオーバー状態
#define GAMEOVER_CONTINUE 0
#define GAMEOVER_EXIT     1
#define GET_MYCURSOR_POS_Y(Y)  (228+Y*80)
int nImageCell_cursor[2] = { 0, MYPLAYCHAR_SIZE * 5 };


////////////////////////////////////////////////////////////////
// 乱数取得
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
// 操作キャラの初期化
////////////////////////////////////////////////////////////////
// コンストラクタ
MYPLAYERCHARACTER::tag_playercharacter()
{
    // 操作キャラ情報
    nWeaponType = MYPLAYCHAR_WEAPON_GREEN_INDEX;
    nWeaponLevel[0] = nWeaponLevel[1] = nWeaponLevel[2] = 1;
    nLifeMax = MYPLAYCHAR_LIFE;

    // スプライト表示情報
    sp.nState = MYPLAYERCHARACTER_STATE::TURN;
    SetSpriteRect();
    sp.nPosX = (GAMESCREEN_WIDTH - MYPLAYCHAR_SIZE) / 2;
    sp.nPosY = GET_MYPLAYCHAR_POS_Y;
    sp.dwDelay = 60;
    sp.byAlpha = 255;
    sp.dwLastTickCount = GetTickCount();
}

////////////////////////////////////////////////////////////////
// データ初期化
void MYPLAYERCHARACTER::InitData(DWORD dwTickCount)
{
    if(dwTickCount == 0)
    {
        dwTickCount = GetTickCount();
    }

    nLife = nLifeMax;
    nIsAttack = 1;
    dwGetItemType = 0;
    nMoveX = 0;

    ChangeSpriteState();
    sp.dwDelay = 60;
    sp.dwLastTickCount = dwTickCount;
}

////////////////////////////////////////////////////////////////
// スプライトの移動
void MYPLAYERCHARACTER::OnMoveX(int x)
{
    int nOldPosX;
    nOldPosX = sp.nPosX;

    // 移動可能な状態判定
    if(sp.nState == MYPLAYERCHARACTER_STATE::MOVE
    || sp.nState == MYPLAYERCHARACTER_STATE::STAND
    || sp.nState == MYPLAYERCHARACTER_STATE::ATTACK)
    {
        // 画面外への移動は禁止
        sp.nPosX += x;
        if(sp.nPosX < 0)
        {
            sp.nPosX = 0;
        }
        else if(sp.nPosX + MYPLAYCHAR_SIZE > GAMESCREEN_WIDTH)
        {
            sp.nPosX = GAMESCREEN_WIDTH - MYPLAYCHAR_SIZE;
        }
    }

    nMoveX = nOldPosX - sp.nPosX;
}

////////////////////////////////////////////////////////////////
// スプライトの状態変更
void MYPLAYERCHARACTER::ChangeSpriteState()
{
    // ゲームオーバー状態
    if(nLife <= 0)
    {
        if(sp.nState != MYPLAYERCHARACTER_STATE::LOSE)
        {
            sp.nState = MYPLAYERCHARACTER_STATE::LOSE;
            nIsAttack = 0;
            SetSpriteRect();
            sp.dwDelay = 1000;
        }
    }
    // 武器持替状態
    else
    if((dwGetItemType & GETITEM_GUN_R && nWeaponType != MYPLAYCHAR_WEAPON_RED_INDEX)
    || (dwGetItemType & GETITEM_GUN_G && nWeaponType != MYPLAYCHAR_WEAPON_GREEN_INDEX)
    || (dwGetItemType & GETITEM_GUN_B && nWeaponType != MYPLAYCHAR_WEAPON_BLUE_INDEX))
    {
        if(sp.nState != MYPLAYERCHARACTER_STATE::TURN)
        {
            sp.nState = MYPLAYERCHARACTER_STATE::TURN;
            SetSpriteRect();
        }
    }
    // 移動状態
    else if(nMoveX != 0)
    {
        if(sp.nState != MYPLAYERCHARACTER_STATE::MOVE)
        {
            sp.nState = MYPLAYERCHARACTER_STATE::MOVE;
            SetSpriteRect();
        }
    }
    // 攻撃状態
    else if(nIsAttack == 1)
    {
        if(sp.nState != MYPLAYERCHARACTER_STATE::ATTACK)
        {
            sp.nState = MYPLAYERCHARACTER_STATE::ATTACK;
            SetSpriteRect();
        }
    }
    // 立ち状態
    else
    {
        if(sp.nState != MYPLAYERCHARACTER_STATE::STAND)
        {
            sp.nState = MYPLAYERCHARACTER_STATE::STAND;
            SetSpriteRect();
        }
    }
}

////////////////////////////////////////////////////////////////
// スプライトのイメージ内座標をセット
void MYPLAYERCHARACTER::SetSpriteRect()
{
    sp.nCurrentCell = 0;
    sp.nMaxCell = nImageCell_pc[sp.nState][2];
    sp.rcImage = {
        nImageCell_pc[sp.nState][0],
        nImageCell_pc[sp.nState][1],
        nImageCell_pc[sp.nState][0] + MYPLAYCHAR_SIZE,
        nImageCell_pc[sp.nState][1] + MYPLAYCHAR_SIZE
    };
}

////////////////////////////////////////////////////////////////
// タイトル画面クラス
////////////////////////////////////////////////////////////////
// コンストラクタ
CMyExampleGame::CMyExampleGame()
{
    srand((UINT)time(NULL));
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

    hfont64 = CreateFontW(
        64, 0, 0, 0,
        FW_DONTCARE,
        FALSE, FALSE, FALSE,
        SHIFTJIS_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH,
        L"ＭＳ ゴシック"
    );

    // 選択カーソル
    spriteCursor.nState = GAMEOVER_CONTINUE;
    spriteCursor.rcImage.left = 0;
    spriteCursor.rcImage.top = 0;
    spriteCursor.rcImage.right = MYPLAYCHAR_SIZE;
    spriteCursor.rcImage.bottom = MYPLAYCHAR_SIZE;
    spriteCursor.nPosX = 24;
    spriteCursor.nPosY = GET_MYCURSOR_POS_Y(spriteCursor.nState);
    spriteCursor.nCurrentCell = 0;
    spriteCursor.nMaxCell = 1;
    spriteCursor.dwDelay = 1000;
    spriteCursor.dwLastTickCount = GetTickCount();

    // 選択メニュー
    selcurMenu[0].nWidth = 256;
    selcurMenu[0].nHeight = 64;
    selcurMenu[0].nPosX = spriteCursor.nPosX + MYPLAYCHAR_SIZE;
    selcurMenu[0].nPosY = GET_MYCURSOR_POS_Y(0);
    selcurMenu[0].nValue = MYMAINTASK::TASK_GAMEWAIT;
    selcurMenu[0].nState = 1;
    selcurMenu[0].byAlpha = 128;
    selcurMenu[0].rgbText[0] = RGB(112, 112, 127);
    selcurMenu[0].rgbText[1] = RGB(240, 240, 254);
    selcurMenu[0].rgbBack[0] = RGB(192, 192, 255);
    selcurMenu[0].rgbBack[1] = RGB(192, 192, 255);
    selcurMenu[0].lpszText = L"Continue";

    selcurMenu[1].nWidth = 256;
    selcurMenu[1].nHeight = 64;
    selcurMenu[1].nPosX = spriteCursor.nPosX + MYPLAYCHAR_SIZE;
    selcurMenu[1].nPosY = GET_MYCURSOR_POS_Y(1);
    selcurMenu[1].nValue = MYMAINTASK::TASK_GAMEEXIT;
    selcurMenu[1].nState = 0;
    selcurMenu[1].byAlpha = 128;
    selcurMenu[1].rgbText[0] = RGB(112, 112, 127);
    selcurMenu[1].rgbText[1] = RGB(240, 240, 254);
    selcurMenu[1].rgbBack[0] = RGB(192, 192, 255);
    selcurMenu[1].rgbBack[1] = RGB(192, 192, 255);
    selcurMenu[1].lpszText = L"GAMEOVER";

    //
    wFPS = 0;
    memset(szFPS, 0, sizeof(szFPS));
    wcscpy(szFPS, L"FPS:");

    dwGameLastTickCount = dwFPSTickCount = GetTickCount();

    return TRUE;
}

////////////////////////////////////////////////////////////////
// 解放処理
void CMyExampleGame::FreeGame()
{
    FreeCDib(pBG);
    FreeCDib(pSprite);
    FreeCDib(pOffscreen);
    DeleteObject(hfont16);
    DeleteObject(hfont24);
    DeleteObject(hfont32);
    DeleteObject(hfont64);

    listBullet.ClearElement();
    listEnemy.ClearElement();
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
    DrawChar(hfont32, 408,   8, L"現在、作成中です...", RGB(254,254,254), RGB(127,254,127));
    DrawChar(hfont32, 408,  48, L"テンキー4/左クリック", RGB(254,254,254), RGB(127,254,127));
    DrawChar(hfont32, 424,  96, L"左に移動", RGB(254,254,254), RGB(127,254,127));
    DrawChar(hfont32, 408, 192, L"テンキー6/右クリック", RGB(254,254,254), RGB(127,254,127));
    DrawChar(hfont32, 424, 240, L"右に移動", RGB(254,254,254), RGB(127,254,127));
    DrawChar(hfont32, 408, 336, L"テンキー5/中クリック", RGB(254,254,254), RGB(127,254,127));
    DrawChar(hfont32, 424, 384, L"射撃ON/OFF切り替え", RGB(254,254,254), RGB(127,254,127));
    DrawChar(hfont32, 408, 480, L"画面は開発中のものです", RGB(254,254,254), RGB(127,254,127));
}

////////////////////////////////////////////////////////////////
// データ初期化
void CMyExampleGame::ResetGameData()
{
    DWORD dwTickCount = GetTickCount();

    // ゲーム画面初期化
    taskNext = MYMAINTASK::TASK_GAMEMAIN;
    dwScore = 0;
    dwWaveCount = 0;
    dwWaveInterval = dwItemInterval = dwBulletInterval = dwTickCount;

    // スプライト初期化
    spritePlayChar.InitData(dwTickCount);  // 操作キャラ
    listBullet.ClearElement();
    listEnemy.ClearElement();
}

////////////////////////////////////////////////////////////////
// フレーム更新
void CMyExampleGame::UpdateFrame_Init(DWORD dwTickCount, BOOL bDrawSkip)
{
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

void CMyExampleGame::UpdateFrame_Main(DWORD dwTickCount, BOOL bDrawSkip)
{
    spritePlayChar.dwGetItemType = 0;
    spritePlayChar.nMoveX = 0;

    CheckKeyInput();  // キー入力

    // スプライト情報更新
    CreateBullet(dwTickCount);  // 弾生成
    UpdateBullet(dwTickCount);  // 弾移動
    CreateEnemy(dwTickCount);   // 敵生成
    UpdateEnemy(dwTickCount);   // 敵移動
    spritePlayChar.ChangeSpriteState();  // 操作キャラ情報更新

    // 処理が間に合わなければ描画はスキップ
    if(bDrawSkip == TRUE)
    {
        return;
    }

    DrawBackground();  // 背景描画
    DrawSprite(dwTickCount);  // スプライト描画

    // ゲームオーバー状態の場合は暗転処理
    if(spritePlayChar.sp.nState == MYPLAYERCHARACTER_STATE::LOSE)
    {
        int nDarkHeight;
        nDarkHeight = dwTickCount - spritePlayChar.sp.dwLastTickCount;
        nDarkHeight = (nDarkHeight > spritePlayChar.sp.dwDelay) ? spritePlayChar.sp.dwDelay : nDarkHeight;
        nDarkHeight = nDarkHeight * GAMESCREEN_HEIGHT / spritePlayChar.sp.dwDelay;

        RECT rcDark = { 0, 0, GAMESCREEN_WIDTH, nDarkHeight };
        pOffscreen->DrawRectangle(rcDark, RGB(0,0,0), 192);

        if(dwTickCount - spritePlayChar.sp.dwLastTickCount >= spritePlayChar.sp.dwDelay)
        {
            taskNext = MYMAINTASK::TASK_GAMEOVER;
        }
    }

    pOffscreen->DrawBits(hMemOffscreen, 0, 0);  // 仮想ウィンドウに描画
    InvalidateRect(hwndExample, NULL, FALSE);  // 画面表示

    // 文字描画
    BYTE byCharColor;
    byCharColor = (spritePlayChar.nLife > 0) ? 254 : 64;

    // スコア表示
    SetScoreStr();
    DrawChar(hfont16, 320, 16, szGameStatus, RGB(byCharColor,byCharColor,byCharColor), RGB(0,0,0));

    // FPS表示
    DrawChar(hfont16, 336, 40, szFPS, RGB(byCharColor,byCharColor,byCharColor), RGB(0,0,0));
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

void CMyExampleGame::UpdateFrame_Exit(DWORD dwTickCount, BOOL bDrawSkip)
{
    dwGameLastTickCount = dwTickCount;

    CheckKeyInput_Exit();  // キー入力

    // "Continue"選択時はスプライト表示情報をリセット
    if(taskNext == MYMAINTASK::TASK_GAMEWAIT)
    {
        listBullet.ClearElement();
        listEnemy.ClearElement();

        spritePlayChar.sp.nState = MYPLAYERCHARACTER_STATE::TURN;
        spritePlayChar.SetSpriteRect();
        spritePlayChar.sp.dwDelay = 60;
        spritePlayChar.sp.dwLastTickCount = dwTickCount;
    }

    // 処理が間に合わなければ描画はスキップ
    if(bDrawSkip == TRUE)
    {
        return;
    }

    DrawBackground();  // 背景描画
    DrawSprite(dwTickCount);  // スプライト描画
    RECT rcDark = { 0, 0, GAMESCREEN_WIDTH, GAMESCREEN_HEIGHT };
    pOffscreen->DrawRectangle(rcDark, RGB(0,0,0), 192);  // 暗転
    DrawSprite_Exit();
    pOffscreen->DrawBits(hMemOffscreen, 0, 0);  // 仮想ウィンドウに描画
    InvalidateRect(hwndExample, NULL, FALSE);  // 画面表示

    // メニュー文字描画
    DrawChar(
        hfont64,
        selcurMenu[0].nPosX, selcurMenu[0].nPosY, 
        selcurMenu[0].lpszText,
        selcurMenu[0].rgbText[selcurMenu[0].nState], RGB(0,0,0)
    );  // "Continue"描画
    DrawChar(
        hfont64,
        selcurMenu[1].nPosX, selcurMenu[1].nPosY,
        selcurMenu[1].lpszText,
        selcurMenu[1].rgbText[selcurMenu[1].nState], RGB(0,0,0)
    );  // "GAMEOVER"描画

    // スコア表示
    SetScoreStr();
    DrawChar(hfont16, 320, 16, szGameStatus, RGB(64,64,64), RGB(0,0,0));
}

////////////////////////////////////////////////////////////////
// キー入力チェック
void CMyExampleGame::CheckKeyInput()
{
    int nMoveX = 0;

    // キー状態セット
    myKeyInput.SetMyKeyState();

    // テンキー4 or 左クリック
    if(myKeyInput.CheckMyKeyState(VK_NUMPAD4, MYKEYSTATE::DOWN)
    || myKeyInput.CheckMyKeyState(VK_LBUTTON, MYKEYSTATE::DOWN))
    {
        nMoveX += -4;
    }

    // テンキー6 or 右クリック
    if(myKeyInput.CheckMyKeyState(VK_NUMPAD6, MYKEYSTATE::DOWN)
    || myKeyInput.CheckMyKeyState(VK_RBUTTON, MYKEYSTATE::DOWN))
    {
        nMoveX += 4;
    }

    // テンキー5 or 中クリック
    if(myKeyInput.CheckMyKeyState(VK_NUMPAD5, MYKEYSTATE::PUSH) == TRUE
    || myKeyInput.CheckMyKeyState(VK_MBUTTON, MYKEYSTATE::PUSH) == TRUE)
    {
        spritePlayChar.nIsAttack = 1 - spritePlayChar.nIsAttack;
    }

    spritePlayChar.OnMoveX(nMoveX);

    // カレントキーの添え字を切替
    myKeyInput.SwitchCurrentIndex();
}

// キー入力チェック(ゲームオーバー状態)
void CMyExampleGame::CheckKeyInput_Exit()
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
        for(i = 0; i <= GAMEOVER_EXIT; i++)
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

        // テンキー8押下の場合、"Continue"を選択
        if(myKeyInput.CheckMyKeyState(VK_NUMPAD8, MYKEYSTATE::PUSH) == TRUE)
        {
            spriteCursor.nState = GAMEOVER_CONTINUE;
            selcurMenu[GAMEOVER_CONTINUE].nState = 1;
            selcurMenu[GAMEOVER_EXIT].nState = 0;
        }

        // テンキー2押下の場合、"終了"を選択
        if(myKeyInput.CheckMyKeyState(VK_NUMPAD2, MYKEYSTATE::PUSH) == TRUE)
        {
            spriteCursor.nState = GAMEOVER_EXIT;
            selcurMenu[GAMEOVER_EXIT].nState = 1;
            selcurMenu[GAMEOVER_CONTINUE].nState = 0;
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
// スコアの文字列をセット
void CMyExampleGame::SetScoreStr()
{
    szGameStatus[0] = L'0' + dwScore / 100000000;
    szGameStatus[1] = L'0' + dwScore % 100000000 / 10000000;
    szGameStatus[2] = L'0' + dwScore % 10000000 / 1000000;
    szGameStatus[3] = L'0' + dwScore % 1000000 / 100000;
    szGameStatus[4] = L'0' + dwScore % 100000 / 100000;
    szGameStatus[5] = L'0' + dwScore % 10000 / 1000;
    szGameStatus[6] = L'0' + dwScore % 1000 / 100;
    szGameStatus[7] = L'0' + dwScore % 100 / 10;
    szGameStatus[8] = L'0' + dwScore % 10;
    szGameStatus[9] = 0;
}

////////////////////////////////////////////////////////////////
// スプライト描画
void CMyExampleGame::DrawSprite(DWORD dwTickCount)
{
    int nWidth, nHeight;
    RECT rcSrc, rcDst;
    DWORD dwColor;

    ////////////////////////////////////////////////////////////
    // 敵キャラ描画
    ////////////////////////////////////////////////////////////
    CMyList<MYENEMY> *ptrEnemy;
    ptrEnemy = listEnemy.GetTailPtr();
    while(ptrEnemy)
    {
        // イメージ内座標をセット
        rcSrc = ptrEnemy->data.sp.rcImage;

        // 画面外の領域は切り取る
        nWidth = rcSrc.right - rcSrc.left;
        nHeight = rcSrc.bottom - rcSrc.top;
        if(ptrEnemy->data.sp.nPosY + nHeight <= 0)
        {
            ptrEnemy = ptrEnemy->prev;
            continue;
        }
        else if(ptrEnemy->data.sp.nPosY < 0)
        {
            rcSrc.top = rcSrc.top - ptrEnemy->data.sp.nPosY;
        }

        // 描画位置をセット
        rcDst.left = ptrEnemy->data.sp.nPosX;
        rcDst.top = ptrEnemy->data.sp.nPosY < 0 ? 0 : ptrEnemy->data.sp.nPosY;
        rcDst.right = rcDst.left + nWidth;
        rcDst.bottom = rcDst.top + nHeight;

        // 描画
        pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);

        ptrEnemy = ptrEnemy->prev;
    }

    ////////////////////////////////////////////////////////////
    // 自キャラ描画
    ////////////////////////////////////////////////////////////
    // アニメーション
    if(dwTickCount - spritePlayChar.sp.dwLastTickCount >= spritePlayChar.sp.dwDelay)
    {
        if(spritePlayChar.sp.nState != MYPLAYERCHARACTER_STATE::LOSE)
        {
            spritePlayChar.sp.nCurrentCell = (spritePlayChar.sp.nCurrentCell + 1) % spritePlayChar.sp.nMaxCell;
            spritePlayChar.sp.dwLastTickCount = dwTickCount;
        }
    }

    // イメージ内座標をセット
    rcSrc = spritePlayChar.sp.rcImage;
    rcSrc.left = rcSrc.left + spritePlayChar.sp.nCurrentCell * MYPLAYCHAR_SIZE;
    rcSrc.right = rcSrc.left + MYPLAYCHAR_SIZE;

    // 描画位置をセット
    rcDst.left = spritePlayChar.sp.nPosX;
    rcDst.right = rcDst.left + MYPLAYCHAR_SIZE;
    rcDst.top = GET_MYPLAYCHAR_POS_Y;
    rcDst.bottom = rcDst.top + MYPLAYCHAR_SIZE;

    // 描画
    pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);

    ////////////////////////////////////////////////////////////
    // 弾描画
    ////////////////////////////////////////////////////////////
    CMyList<MYBULLET> *ptrBullet;
    ptrBullet = listBullet.GetHeadPtr();
    while(ptrBullet)
    {
        // イメージ内座標をセット
        rcSrc = ptrBullet->data.sp.rcImage;

        // 描画位置をセット
        rcDst.left = ptrBullet->data.sp.nPosX;
        rcDst.top = ptrBullet->data.sp.nPosY;
        rcDst.right = rcDst.left + 4;
        rcDst.bottom = rcDst.top + 4;

        // 描画
        pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);

        ptrBullet = ptrBullet->next; 
    }

}

// スプライト描画(ゲームオーバー状態)
void CMyExampleGame::DrawSprite_Exit()
{
    RECT rcSrc, rcDst;
    DWORD dwColor;

    // スプライト表示
    rcSrc = spriteCursor.rcImage;
    rcSrc.left = nImageCell_cursor[spriteCursor.nState];
    rcSrc.right = rcSrc.left + MYPLAYCHAR_SIZE;

    rcDst.left = spriteCursor.nPosX;
    rcDst.right = rcDst.left + MYPLAYCHAR_SIZE;
    rcDst.top = GET_MYCURSOR_POS_Y(spriteCursor.nState);
    rcDst.bottom = rcDst.top + MYPLAYCHAR_SIZE;

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

////////////////////////////////////////////////////////////////
// 弾スプライト生成
void CMyExampleGame::CreateBullet(DWORD dwTickCount)
{
    // 攻撃状態でなければ処理対象外
    if(spritePlayChar.nIsAttack == 0)
    {
        return;
    }

    // 時間経過が攻撃間隔未満であれば処理対象外
    int nBulletInterval[MYPLAYCHAR_WEAPON_LV_MAX][3] = {
        { 1000, 300, 700 },
        { 1000, 270, 700 },
        { 1000, 240, 700 },
        { 1000, 210, 700 },
        { 1000, 180, 700 },
        { 1000, 150, 700 },
        { 1000, 120, 700 },
        { 1000,  90, 700 },
        { 1000,  60, 700 },
        { 1000,  30, 700 }
    };
    WORD nLv = spritePlayChar.nWeaponLevel[spritePlayChar.nWeaponType];

    if(dwTickCount - dwBulletInterval < nBulletInterval[nLv][spritePlayChar.nWeaponType])
    {
        return;
    }

    // 武器種別ごとの弾生成
    CMyList<MYBULLET> *ptr;
    ptr = new CMyList<MYBULLET>;
    if(ptr == NULL)
    {
        return;
    }

    switch(spritePlayChar.nWeaponType)
    {
        // 赤
        case MYPLAYCHAR_WEAPON_RED_INDEX:
            break;

        // 緑
        case MYPLAYCHAR_WEAPON_GREEN_INDEX:
            ptr->data.nMoveX = 0;
            ptr->data.nMoveY = -4;
            ptr->data.sp.byAlpha = 255;
            ptr->data.sp.dwDelay = 16;
            ptr->data.sp.dwLastTickCount = dwTickCount;
            ptr->data.sp.nCurrentCell = 0;
            ptr->data.sp.nMaxCell = 1;
            ptr->data.sp.nPosX = spritePlayChar.sp.nPosX + 52;
            ptr->data.sp.nPosY = GET_MYPLAYCHAR_POS_Y - 4;
            ptr->data.sp.nState = 0;
            ptr->data.sp.rcImage = { 324, 64, 328, 68 };

            listBullet.AddTail(ptr);
            break;

        // 青
        case MYPLAYCHAR_WEAPON_BLUE_INDEX:
            break;
    }

    dwBulletInterval = dwTickCount;
}

////////////////////////////////////////////////////////////////
// 弾スプライト更新
void CMyExampleGame::UpdateBullet(DWORD dwTickCount)
{
    CMyList<MYBULLET> *ptr;

    // 弾の移動
    ptr = listBullet.GetHeadPtr();
    while(ptr)
    {
        // 時間経過で弾移動
        if(dwTickCount - ptr->data.sp.dwLastTickCount > ptr->data.sp.dwDelay)
        {
            ptr->data.sp.nPosX += ptr->data.nMoveX;
            ptr->data.sp.nPosY += ptr->data.nMoveY;
            ptr->data.sp.dwLastTickCount = dwTickCount;
        }

        // 画面外の弾は削除
        if(ptr->data.sp.nPosY < 0)
        {
            ptr = listBullet.DeleteElement(ptr);
        }
        else
        {
            ptr = ptr->next;
        }
    }
}

////////////////////////////////////////////////////////////////
// 敵スプライト生成
void CMyExampleGame::CreateEnemy(DWORD dwTickCount)
{
    // 操作不能の場合は処理対象外
    if(spritePlayChar.nLife <= 0)
    {
        return;
    }

    // 時間経過が攻撃間隔未満であれば処理対象外
    if(dwTickCount - dwWaveInterval < MYENEMY_WAVE_INTERVAL)
    {
        return;
    }

    // ウェーブカウントに応じた敵データ生成
    dwWaveCount++;

    CMyList<MYENEMY> *ptr;
    ptr = new CMyList<MYENEMY>;
    if(ptr == NULL)
    {
        return;
    }

    // 暫定処理
    ptr->data.sp.byAlpha = 255;
    ptr->data.sp.dwDelay = 120;
    ptr->data.sp.dwLastTickCount = dwTickCount;
    ptr->data.sp.nCurrentCell = 0;
    ptr->data.sp.nMaxCell = 1;
    ptr->data.sp.nPosX = GetMyRandVal(32, 236);
    ptr->data.sp.nPosY = -32;
    ptr->data.sp.nState = MYPLAYERCHARACTER_STATE::STAND;
    ptr->data.sp.rcImage = { 32, 288, 64, 320 };
    ptr->data.bDispLife = FALSE;
    ptr->data.dwScore = 1;
    ptr->data.nAttack = 1;
    ptr->data.nLife = 1;
    ptr->data.nLifeMax = 1;
    ptr->data.nMoveX = 0;
    ptr->data.nMoveY = 4;
    ptr->data.nWeaponType = MYPLAYCHAR_WEAPON_GREEN_INDEX;

    listEnemy.AddTail(ptr);

    dwWaveInterval = dwTickCount;
}

////////////////////////////////////////////////////////////////
// 敵スプライト更新
void CMyExampleGame::UpdateEnemy(DWORD dwTickCount)
{
    int nHeight;
    CMyList<MYENEMY> *ptr;

    // 弾の移動
    ptr = listEnemy.GetTailPtr();
    while(ptr)
    {
        // 時間経過で敵移動
        if(dwTickCount - ptr->data.sp.dwLastTickCount > ptr->data.sp.dwDelay)
        {
            ptr->data.sp.nPosX += ptr->data.nMoveX;
            ptr->data.sp.nPosY += ptr->data.nMoveY;
            ptr->data.sp.dwLastTickCount = dwTickCount;

            // 操作キャラに触れるとダメージ
            nHeight = ptr->data.sp.rcImage.bottom - ptr->data.sp.rcImage.top;
            if(ptr->data.sp.nPosY + nHeight > GET_MYPLAYCHAR_POS_Y)
            {
                ptr->data.sp.nPosY -= ptr->data.nMoveY;
                spritePlayChar.nLife -= ptr->data.nAttack;
                if(spritePlayChar.nLife < 0)
                {
                    spritePlayChar.nLife = 0;
                }
            }
        }

        ptr = ptr->prev;
    }
}

