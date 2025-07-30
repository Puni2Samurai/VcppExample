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
#define MYPLAYBUDDY_SIZE 32
#define GET_MYPLAYCHAR_POS_Y (GAMESCREEN_HEIGHT - MYPLAYCHAR_SIZE - 32)
#define MYPLAYCHAR_WEAPON_LV_MAX     10
#define MYPLAYCHAR_WEAPON_RED_INDEX   0
#define MYPLAYCHAR_WEAPON_GREEN_INDEX 1
#define MYPLAYCHAR_WEAPON_BLUE_INDEX  2
#define MYPLAYCHAR_BUDDY_M_INDEX 0
#define MYPLAYCHAR_BUDDY_S_INDEX 1
#define MYPLAYCHAR_BUDDY_B_INDEX 2
#define MYPLAYCHAR_LIFE 256

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
int nImageCell_pc[MYPLAYERCHARACTER_STATE::MAX][4] = {
    // x, y, セル数, 遅延時間
    { 0,                   1, 1,  500 },  // 振り向き
    { MYPLAYCHAR_SIZE,     1, 1,   60 },  // 立ち
    { MYPLAYCHAR_SIZE,     1, 2,   60 },  // 攻撃
    { MYPLAYCHAR_SIZE * 3, 1, 2,   60 },  // 移動
    { MYPLAYCHAR_SIZE * 5, 0, 1, 1000 }   // 負け
};

// 操作キャラと味方キャラの相対座標
int nBuddyPosX[3] = { 0, -16, 48 };
int nBuddyPosY[3] = { 0,  32, 32 };

// アイテム
#define MYITEM_WAVE_INTERVAL 20000
#define GETITEM_GUN_R   0x00000001
#define GETITEM_GUN_G   0x00000002
#define GETITEM_GUN_B   0x00000004
#define GETITEM_BUDDY1  0x00000010
#define GETITEM_BUDDY2  0x00000020
#define GETITEM_BUDDY3  0x00000040
#define GETITEM_RECOVER 0x00000100
#define GETITEM_IDX_MAX          7

// アイコン
enum MYICON_INDEX
{
    MYICON_INDEX_LIFEICON,
    MYICON_INDEX_LIFEBAR,
    MYICON_INDEX_ENEMYBAR,
    MYICON_INDEX_GUN_R,
    MYICON_INDEX_GUN_G,
    MYICON_INDEX_GUN_B,
    MYICON_INDEX_GUN_OFF,
    MYICON_INDEX_GUN_ON,
};

RECT rcImageCell_icon[] = {
    { 328, 288, 344, 304 },
    { 344, 288, 348, 304 },
    { 348, 288, 352, 296 },
    { 320,  96, 352, 128 },
    { 320, 128, 352, 160 },
    { 320, 160, 352, 192 },
    { 352,  96, 384, 128 },
    { 352, 128, 384, 160 },
};

// 敵キャラ
#define MYENEMY_WAVE_INTERVAL 5000  // 暫定
#define MYENEMY_SIZE 32
#define MYBOSS_SIZE  64

// ゲームオーバー状態
#define GAMEOVER_CONTINUE 0
#define GAMEOVER_EXIT     1
#define GET_MYCURSOR_POS_Y(Y)  (228+Y*80)

////////////////////////////////////////////////////////////////
extern LONG GetMyRandVal(LONG lMin, LONG lMax);


////////////////////////////////////////////////////////////////
// 操作キャラの初期化
////////////////////////////////////////////////////////////////
// コンストラクタ
MYPLAYERCHARACTER::tag_playercharacter()
{
    // 操作キャラ情報
    nWeaponType = nWeaponTypeOld = MYPLAYCHAR_WEAPON_GREEN_INDEX;
    nWeaponLevel[0] = nWeaponLevel[1] = nWeaponLevel[2] = 1;
    nLifeMax = MYPLAYCHAR_LIFE;
    nLife = nLifeTmp = 0;

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

    nLife = nLifeTmp = nLifeMax;
    nIsAttack = 1;
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
        }
    }
    // 武器持替状態
    else if(nWeaponType != nWeaponTypeOld)
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
        (nImageCell_pc[sp.nState][1] == 0) ? 0 : nWeaponType * MYPLAYCHAR_SIZE,
        nImageCell_pc[sp.nState][0] + MYPLAYCHAR_SIZE,
        (nImageCell_pc[sp.nState][1] == 0) ? MYPLAYCHAR_SIZE : (nWeaponType + 1) * MYPLAYCHAR_SIZE
    };
    sp.dwDelay = nImageCell_pc[sp.nState][3];
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
    hfont12 = NULL;
    hfont16 = NULL;
    hfont32 = NULL;
    hfont64 = NULL;
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
    hfont12 = CreateFontW(
        12, 0, 0, 0,
        FW_DONTCARE,
        FALSE, FALSE, FALSE,
        SHIFTJIS_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH,
        L"ＭＳ ゴシック"
    );

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

    // 選択カーソル
    spriteCursor.nState = GAMEOVER_CONTINUE;
    spriteCursor.rcImage.left = 0;
    spriteCursor.rcImage.top = 64;
    spriteCursor.rcImage.right = MYPLAYCHAR_SIZE;
    spriteCursor.rcImage.bottom = 64 + MYPLAYCHAR_SIZE;
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
    DeleteObject(hfont12);
    DeleteObject(hfont16);
    DeleteObject(hfont32);
    DeleteObject(hfont64);

    listBullet.ClearElement();
    listEnemy.ClearElement();
    listItem.ClearElement();
    listBuddy.ClearElement();
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
    wItemIndex = 0;
    dwWaveInterval = dwItemInterval = dwBulletInterval = dwTickCount;

    // スプライト初期化
    spritePlayChar.InitData(dwTickCount);
    listBullet.ClearElement();
    listEnemy.ClearElement();
    listItem.ClearElement();
    listBuddy.ClearElement();
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
    spritePlayChar.nWeaponTypeOld = spritePlayChar.nWeaponType;
    spritePlayChar.nMoveX = 0;
    spritePlayChar.nLifeTmp = spritePlayChar.nLife;

    CheckKeyInput();  // キー入力

    // スプライト情報更新
    CreateBullet(dwTickCount);  // 弾生成
    UpdateBullet(dwTickCount);  // 弾移動
    CreateEnemy(dwTickCount);   // 敵生成
    UpdateEnemy(dwTickCount);   // 敵移動
    CreateGameItem(dwTickCount);// アイテム生成
    UpdateGameItem(dwTickCount);// アイテム更新
    UpdateBuddy(dwTickCount);   // 味方キャラ情報更新
    if((spritePlayChar.sp.nState != MYPLAYERCHARACTER_STATE::TURN)
    || (dwTickCount - spritePlayChar.sp.dwLastTickCount >= spritePlayChar.sp.dwDelay))
    {
        spritePlayChar.ChangeSpriteState();  // 操作キャラ情報更新
    }

    // 処理が間に合わなければ描画はスキップ
    if(bDrawSkip == TRUE)
    {
        return;
    }

    DrawBackground();  // 背景描画
    DrawSprite(dwTickCount);  // スプライト描画
    pOffscreen->DrawBits(hMemOffscreen, 0, 0);  // 仮想ウィンドウに描画
    InvalidateRect(hwndExample, NULL, FALSE);  // 画面表示
    DrawGameString(dwTickCount);  // 情報文字列表示

    // 体力0時は画面暗転から時間経過でゲームオーバー状態へ遷移
    if((spritePlayChar.sp.nState == MYPLAYERCHARACTER_STATE::LOSE)
    && (dwTickCount - spritePlayChar.sp.dwLastTickCount >= spritePlayChar.sp.dwDelay))
    {
        taskNext = MYMAINTASK::TASK_GAMEOVER;
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
    DrawSprite_Exit();
    pOffscreen->DrawBits(hMemOffscreen, 0, 0);  // 仮想ウィンドウに描画
    InvalidateRect(hwndExample, NULL, FALSE);  // 画面表示
    DrawGameString(dwTickCount);  // 情報文字列表示

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
// スプライト描画
void CMyExampleGame::DrawSprite(DWORD dwTickCount)
{
    int i;
    int nWidth, nHeight, nCorrect;
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
        pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE, ptrEnemy->data.sp.byAlpha);

        // 残り体力表示
        if((ptrEnemy->data.bDispLife == TRUE) && (ptrEnemy->data.sp.nPosY >= 8))
        {
            rcSrc = { 348, 288, 352, 296 };
            rcDst.top = ptrEnemy->data.sp.nPosY - 8;
            rcDst.bottom = rcDst.top + 8;
            for(i = 0; i < 10; i++)
            {
                rcDst.left = ptrEnemy->data.sp.nPosX + (nWidth - 40) / 2 + i * 4;
                rcDst.right = rcDst.left + 4;
                pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);
            }

            nCorrect = ptrEnemy->data.nLife * 40 / ptrEnemy->data.nLifeMax;
            if(nCorrect > 0)
            {
                rcDst.left = ptrEnemy->data.sp.nPosX + (nWidth - 40) / 2;
                rcDst.top = ptrEnemy->data.sp.nPosY - 6;
                rcDst.right = rcDst.left + nCorrect;
                rcDst.bottom = rcDst.top + 4;
                pOffscreen->DrawRectangle(rcDst, RGB(255,64,64), 255);
            }
        }

        ptrEnemy = ptrEnemy->prev;
    }

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

    ////////////////////////////////////////////////////////////
    // アイテム描画
    ////////////////////////////////////////////////////////////
    int nHeightTop, nHeightMiddle, nHeightBottom;
    int nWidthExt;
    CMyList<CMyGameItem> *ptrItem;
    ptrItem = listItem.GetHeadPtr();
    while(ptrItem)
    {
        // 非活性のアイテムは描画をスキップ
        if(ptrItem->data.bActive == FALSE)
        {
            ptrItem = ptrItem->next;
            continue;
        }

        nHeightTop = ptrItem->data.rcUpSide.bottom - ptrItem->data.rcUpSide.top;
        nHeightBottom = ptrItem->data.rcLowSide.bottom - ptrItem->data.rcLowSide.top;
        nHeightMiddle = ptrItem->data.rcLowSide.top - ptrItem->data.rcUpSide.bottom;
        nWidthExt = ptrItem->data.rcLowSide.right - ptrItem->data.rcLowSide.left;

        // 画面外のアイテムは描画をスキップ
        if(ptrItem->data.sp.nPosY + nHeightTop + nHeightMiddle + nHeightBottom <= 0)
        {
            ptrItem = ptrItem->next;
            continue;
        }

        ////////////////////////////////////////////////////////
        // アイテム下段を描画
        ////////////////////////////////////////////////////////
        // イメージ内座標をセット
        rcSrc = ptrItem->data.rcLowSide;

        // 画面外の領域は切り取る
        if(ptrItem->data.sp.nPosY + nHeightTop + nHeightMiddle < 0)
        {
            rcSrc.top = rcSrc.top - (ptrItem->data.sp.nPosY + nHeightTop + nHeightMiddle);
        }

        // 描画位置をセット
        rcDst.left = ptrItem->data.sp.nPosX;
        rcDst.top = (ptrItem->data.sp.nPosY + nHeightTop + nHeightMiddle < 0) ? 0 : ptrItem->data.sp.nPosY + nHeightTop + nHeightMiddle;
        rcDst.right = rcDst.left + nWidthExt;
        rcDst.bottom = rcDst.top + nHeightBottom;

        // 描画
        pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE, ptrItem->data.sp.byAlpha);

        ////////////////////////////////////////////////////////
        // アイテム上段を描画
        ////////////////////////////////////////////////////////
        // イメージ内座標をセット
        rcSrc = ptrItem->data.rcUpSide;

        // 画面内の領域あり
        if(ptrItem->data.sp.nPosY + nHeightTop > 0)
        {
            // 画面外の領域は切り取る
            if(ptrItem->data.sp.nPosY < 0)
            {
                rcSrc.top = rcSrc.top - ptrItem->data.sp.nPosY;
            }

            // 描画位置をセット
            rcDst.left = ptrItem->data.sp.nPosX;
            rcDst.top = ptrItem->data.sp.nPosY < 0 ? 0 : ptrItem->data.sp.nPosY;
            rcDst.right = rcDst.left + nWidthExt;
            rcDst.bottom = rcDst.top + nHeightTop;

            // 描画
            pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE, ptrItem->data.sp.byAlpha);
        }

        ////////////////////////////////////////////////////////
        // アイテム内容を描画
        ////////////////////////////////////////////////////////
        // イメージ内座標をセット
        rcSrc = ptrItem->data.sp.rcImage;

        // 画面内の領域あり
        nWidth = rcSrc.right - rcSrc.left;
        nHeight = rcSrc.bottom - rcSrc.top;
        nCorrect = (nHeightMiddle - nHeight) / 2;
        if(ptrItem->data.sp.nPosY + nHeight + nHeightTop + nCorrect > 0)
        {
            // 画面外の領域は切り取る
            if(ptrItem->data.sp.nPosY + nHeightTop + nCorrect < 0)
            {
                rcSrc.top = rcSrc.top - (ptrItem->data.sp.nPosY + nHeightTop + nCorrect);
            }

            // 描画位置をセット
            rcDst.left = ptrItem->data.sp.nPosX + ((nWidthExt - nWidth) / 2);
            rcDst.top = (ptrItem->data.sp.nPosY + nHeightTop + nCorrect < 0) ? 0 : ptrItem->data.sp.nPosY + nHeightTop + nCorrect;
            rcDst.right = rcDst.left + nWidth;
            rcDst.bottom = rcDst.top + nHeight;

            // 描画
            pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);
        }

        ////////////////////////////////////////////////////////
        // アイテム耐久を描画
        ////////////////////////////////////////////////////////
        // 画面内の領域あり
        nHeight = (int)(ptrItem->data.nLife * nHeightMiddle / ptrItem->data.nLifeMax);
        nCorrect = nHeightMiddle - nHeight;
        if((nHeight > 0) && (ptrItem->data.sp.nPosY + nHeightTop + nHeightMiddle > 0))
        {
            // 画面外の領域は切り取る
            if(ptrItem->data.sp.nPosY + nHeightTop + nCorrect < 0)
            {
                nHeight = nHeight + (ptrItem->data.sp.nPosY + nHeightTop + nCorrect);
            }

            // 描画位置をセット
            rcDst.left = ptrItem->data.sp.nPosX;
            rcDst.top = (ptrItem->data.sp.nPosY + nHeightTop + nCorrect < 0) ? 0 : ptrItem->data.sp.nPosY + nHeightTop + nCorrect;
            rcDst.right = rcDst.left + nWidthExt;
            rcDst.bottom = rcDst.top + nHeight;

            // 描画
            BYTE byRed, byGreen, byBlue;
            byRed   = (ptrItem->data.sp.nCurrentCell == MYPLAYCHAR_WEAPON_RED_INDEX)   ? 255 : 64;
            byGreen = (ptrItem->data.sp.nCurrentCell == MYPLAYCHAR_WEAPON_GREEN_INDEX) ? 255 : 64;
            byBlue  = (ptrItem->data.sp.nCurrentCell == MYPLAYCHAR_WEAPON_BLUE_INDEX)  ? 255 : 64;
            pOffscreen->DrawRectangle(rcDst, RGB(byRed,byGreen,byBlue), 128);
        }

        ////////////////////////////////////////////////////////
        // アイテム中段を描画
        ////////////////////////////////////////////////////////
        // イメージ内座標をセット
        rcSrc.left = ptrItem->data.rcUpSide.left;
        rcSrc.top = ptrItem->data.rcUpSide.bottom;
        rcSrc.right = rcSrc.left + nWidthExt;
        rcSrc.bottom = rcSrc.top + nHeightMiddle;

        // アイテム未開放、かつ、画面内の領域あり
        if((ptrItem->data.sp.nState != MYPLAYERCHARACTER_STATE::LOSE)
        && (ptrItem->data.sp.nPosY + nHeightTop + nHeightMiddle > 0))
        {
            // 画面外の領域は切り取る
            if(ptrItem->data.sp.nPosY + nHeightTop < 0)
            {
                rcSrc.top = rcSrc.top - (ptrItem->data.sp.nPosY + nHeightTop);
            }

            // 描画位置をセット
            rcDst.left = ptrItem->data.sp.nPosX;
            rcDst.top = (ptrItem->data.sp.nPosY + nHeightTop < 0) ? 0 : ptrItem->data.sp.nPosY + nHeightTop;
            rcDst.right = rcDst.left + nWidthExt;
            rcDst.bottom = rcDst.top + nHeightMiddle;

            // 描画
            pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE, ptrItem->data.sp.byAlpha);
        }

        ptrItem = ptrItem->next;
    }

    ////////////////////////////////////////////////////////////
    // 操作キャラ/味方キャラ描画
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

    ////////////////////////////////////////////
    // 先頭の味方キャラ
    ////////////////////////////////////////////
    CMyList<SPRITE> *ptrBuddy;
    int nImageCell_buddy[MYPLAYERCHARACTER_STATE::MAX] = { 0, 0, 32, 32, 96 };

    ptrBuddy = listBuddy.GetHeadPtr();
    if(ptrBuddy != NULL)
    {
        // イメージ内座標をセット
        rcSrc = ptrBuddy->data.rcImage;
        rcSrc.left = rcSrc.left + ptrBuddy->data.nCurrentCell * MYPLAYBUDDY_SIZE + nImageCell_buddy[ptrBuddy->data.nState];
        rcSrc.right = rcSrc.left + MYPLAYBUDDY_SIZE;

        // 描画位置をセット
        rcDst.left = ptrBuddy->data.nPosX;
        rcDst.right = rcDst.left + MYPLAYBUDDY_SIZE;
        rcDst.top = ptrBuddy->data.nPosY;
        rcDst.bottom = rcDst.top + MYPLAYBUDDY_SIZE;

        // 描画
        pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE, ptrBuddy->data.byAlpha);
    }

    ////////////////////////////////////////////
    // 操作キャラ
    ////////////////////////////////////////////
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

    ////////////////////////////////////////////
    // 後方の味方キャラ
    ////////////////////////////////////////////
    ptrBuddy = listBuddy.GetHeadPtr();
    while(ptrBuddy)
    {
        if(ptrBuddy != listBuddy.GetHeadPtr())
        {
            // イメージ内座標をセット
            rcSrc = ptrBuddy->data.rcImage;
            rcSrc.left = rcSrc.left + ptrBuddy->data.nCurrentCell * MYPLAYBUDDY_SIZE + nImageCell_buddy[ptrBuddy->data.nState];
            rcSrc.right = rcSrc.left + MYPLAYBUDDY_SIZE;

            // 描画位置をセット
            rcDst.left = ptrBuddy->data.nPosX;
            rcDst.right = rcDst.left + MYPLAYBUDDY_SIZE;
            rcDst.top = ptrBuddy->data.nPosY;
            rcDst.bottom = rcDst.top + MYPLAYBUDDY_SIZE;

            // 描画
            pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE, ptrBuddy->data.byAlpha);
        }

        ptrBuddy = ptrBuddy->next;
    }

    ////////////////////////////////////////////////////////////
    // アイコン描画
    ////////////////////////////////////////////////////////////
    // ライフアイコン
    rcSrc = rcImageCell_icon[MYICON_INDEX_LIFEICON];
    rcDst = { 12, GAMESCREEN_HEIGHT - 24, 28, GAMESCREEN_HEIGHT - 8 };
    pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);

    // ライフバー
    rcSrc = rcImageCell_icon[MYICON_INDEX_LIFEBAR];
    for(i = 0; i < 64; i++)
    {
        rcDst = { 32 + i * 4, GAMESCREEN_HEIGHT - 24, 36 + i * 4, GAMESCREEN_HEIGHT - 8 };
        pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);
    }

    // 残りライフ
    rcDst = { 32, GAMESCREEN_HEIGHT - 20, 32 + spritePlayChar.nLife, GAMESCREEN_HEIGHT - 12 };
    pOffscreen->DrawRectangle(rcDst, RGB(255,128,192), 255);

    // ウェポン
    for(i = 0; i < 3; i++)
    {
        rcSrc = rcImageCell_icon[MYICON_INDEX_GUN_R + i];
        rcDst = { 301 + i * 32 + i, GAMESCREEN_HEIGHT - 40, 333 + i * 32 + i, GAMESCREEN_HEIGHT - 8 };
        pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);
        rcSrc = (spritePlayChar.nWeaponType == i) ? rcImageCell_icon[MYICON_INDEX_GUN_ON] : rcImageCell_icon[MYICON_INDEX_GUN_OFF];
        pOffscreen->CopyDibBits(*pSprite, rcSrc, rcDst, TRUE);
    }

    ////////////////////////////////////////////////////////////
    // エフェクト描画
    ////////////////////////////////////////////////////////////
    // ダメージ
    if(spritePlayChar.nLife < spritePlayChar.nLifeTmp)
    {
        rcDst = { 32, GAMESCREEN_HEIGHT - 24, 288, GAMESCREEN_HEIGHT - 20 };
        pOffscreen->DrawRectangle(rcDst, RGB(255,32,32), 192);
        rcDst = { 32, GAMESCREEN_HEIGHT - 12, 288, GAMESCREEN_HEIGHT -  8 };
        pOffscreen->DrawRectangle(rcDst, RGB(255,32,32), 192);
    }

    // ゲームオーバー状態の場合は暗転処理
    if(spritePlayChar.sp.nState == MYPLAYERCHARACTER_STATE::LOSE)
    {
        int nDarkHeight;
        nDarkHeight = dwTickCount - spritePlayChar.sp.dwLastTickCount;
        nDarkHeight = (nDarkHeight > spritePlayChar.sp.dwDelay) ? spritePlayChar.sp.dwDelay : nDarkHeight;
        nDarkHeight = nDarkHeight * GAMESCREEN_HEIGHT / spritePlayChar.sp.dwDelay;

        rcDst = { 0, 0, GAMESCREEN_WIDTH, nDarkHeight };
        pOffscreen->DrawRectangle(rcDst, RGB(0,0,0), 192);
    }
}

// スプライト描画(ゲームオーバー状態)
void CMyExampleGame::DrawSprite_Exit()
{
    RECT rcSrc, rcDst;
    DWORD dwColor;
    RECT rcImageCell_cursor[2] = {
        { 0, MYPLAYCHAR_SIZE, MYPLAYCHAR_SIZE, MYPLAYCHAR_SIZE * 2 },
        { MYPLAYCHAR_SIZE * 5, 0, MYPLAYCHAR_SIZE * 6, MYPLAYCHAR_SIZE } 
    };

    // スプライト表示
    rcSrc = rcImageCell_cursor[spriteCursor.nState];

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
// 文字情報表示
void CMyExampleGame::DrawGameString(DWORD dwTickCount)
{
    int i;
    BYTE byCharColor;
    byCharColor = (spritePlayChar.nLife > 0) ? 254 : 64;

    // スコア表示
    szGameStatus[0] = L'0' + dwScore / 100000000;
    szGameStatus[1] = L'0' + dwScore % 100000000 / 10000000;
    szGameStatus[2] = L'0' + dwScore % 10000000 / 1000000;
    szGameStatus[3] = L'0' + dwScore % 1000000 / 100000;
    szGameStatus[4] = L'0' + dwScore % 100000 / 10000;
    szGameStatus[5] = L'0' + dwScore % 10000 / 1000;
    szGameStatus[6] = L'0' + dwScore % 1000 / 100;
    szGameStatus[7] = L'0' + dwScore % 100 / 10;
    szGameStatus[8] = L'0' + dwScore % 10;
    szGameStatus[9] = 0;
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

    // 武器Lv表示
    for(i = 0; i < 3; i++)
    {
        if(spritePlayChar.nWeaponLevel[i] < 10)
        {
            wcscpy(szGameStatus, L"Lv");
            szGameStatus[2] = L'0' + spritePlayChar.nWeaponLevel[i] % 10;
            szGameStatus[3] = 0;
        }
        else
        {
            wcscpy(szGameStatus, L"MAX");
            szGameStatus[3] = 0;
        }

        DrawChar(
            hfont12,
            315 + i * 32 + i, GAMESCREEN_HEIGHT - 18,
            szGameStatus,
            RGB(byCharColor,byCharColor,byCharColor), RGB(0,0,0), FALSE
        );
    }
}

////////////////////////////////////////////////////////////////
// 文字列描画
void CMyExampleGame::DrawChar(HFONT& hfont, int x, int y, LPCWSTR lpszText, COLORREF rgbText, COLORREF rgbBorder, BOOL bBorder)
{
    HFONT hFontOld;

    SetBkMode(hMemOffscreen, TRANSPARENT);
    hFontOld = (HFONT)SelectObject(hMemOffscreen, hfont);

    if(bBorder == TRUE)
    {
        SetTextColor(hMemOffscreen, rgbBorder);
        TextOutW(hMemOffscreen, x, y - 1, lpszText, wcslen(lpszText));
        TextOutW(hMemOffscreen, x + 1, y, lpszText, wcslen(lpszText));
        TextOutW(hMemOffscreen, x, y + 1, lpszText, wcslen(lpszText));
        TextOutW(hMemOffscreen, x - 1, y, lpszText, wcslen(lpszText));
    }

    SetTextColor(hMemOffscreen, rgbText);
    TextOutW(hMemOffscreen, x, y, lpszText, wcslen(lpszText));

    SelectObject(hMemOffscreen, hFontOld);
}

////////////////////////////////////////////////////////////////
// 味方状態更新
void CMyExampleGame::UpdateBuddy(DWORD dwTickCount)
{
    int nIndex;
    BOOL bDelete;
    CMyList<SPRITE> *ptr;

    nIndex = 0;
    ptr = listBuddy.GetHeadPtr();
    while(ptr)
    {
        bDelete = FALSE;

        // 一定時間経過
        if(dwTickCount - ptr->data.dwLastTickCount >= ptr->data.dwDelay)
        {
            if(ptr->data.nState == MYPLAYERCHARACTER_STATE::LOSE)
            {
                bDelete = TRUE;
            }
            else
            {
                ptr->data.nCurrentCell = (ptr->data.nCurrentCell + 1) % ptr->data.nMaxCell;
                ptr->data.dwLastTickCount = dwTickCount;
            }
        }

        // 生存キャラの情報更新
        if(ptr->data.nState != MYPLAYERCHARACTER_STATE::LOSE)
        {
            // 操作キャラの状態に応じて味方キャラの状態を変更
            if((spritePlayChar.sp.nState == MYPLAYERCHARACTER_STATE::STAND)
            && (ptr->data.nState != MYPLAYERCHARACTER_STATE::STAND))
            {
                ptr->data.nState = MYPLAYERCHARACTER_STATE::STAND;
                ptr->data.nCurrentCell = 0;
                ptr->data.dwLastTickCount = dwTickCount;
                ptr->data.nMaxCell = nImageCell_pc[MYPLAYERCHARACTER_STATE::STAND][2];
                ptr->data.dwDelay = nImageCell_pc[MYPLAYERCHARACTER_STATE::STAND][3];
            }
            else if((spritePlayChar.sp.nState == MYPLAYERCHARACTER_STATE::ATTACK)
            && (ptr->data.nState != MYPLAYERCHARACTER_STATE::MOVE))
            {
                ptr->data.nState = MYPLAYERCHARACTER_STATE::MOVE;
                ptr->data.nCurrentCell = 0;
                ptr->data.dwLastTickCount = dwTickCount;
                ptr->data.nMaxCell = nImageCell_pc[MYPLAYERCHARACTER_STATE::MOVE][2];
                ptr->data.dwDelay = nImageCell_pc[MYPLAYERCHARACTER_STATE::MOVE][3];
            }
            else if((spritePlayChar.sp.nState == MYPLAYERCHARACTER_STATE::MOVE)
            && (ptr->data.nState != MYPLAYERCHARACTER_STATE::MOVE))
            {
                ptr->data.nState = MYPLAYERCHARACTER_STATE::MOVE;
                ptr->data.nCurrentCell = 0;
                ptr->data.dwLastTickCount = dwTickCount;
                ptr->data.nMaxCell = nImageCell_pc[MYPLAYERCHARACTER_STATE::MOVE][2];
                ptr->data.dwDelay = nImageCell_pc[MYPLAYERCHARACTER_STATE::MOVE][3];
            }

            // 座標更新
            ptr->data.nPosX = spritePlayChar.sp.nPosX + nBuddyPosX[nIndex];
            if(ptr->data.nPosX < 0)
            {
                ptr->data.nPosX = 0;
            }

            if(ptr->data.nPosX > GAMESCREEN_WIDTH - MYPLAYBUDDY_SIZE)
            {
                ptr->data.nPosX = GAMESCREEN_WIDTH - MYPLAYBUDDY_SIZE;
            }
        }

        // 次のポインタを取得
        nIndex++;
        if(bDelete == TRUE)
        {
            ptr = listBuddy.DeleteElement(ptr);
        }
        else
        {
            ptr = ptr->next;
        }
    }
}

////////////////////////////////////////////////////////////////
// 弾スプライト生成
void CMyExampleGame::CreateBullet(DWORD dwTickCount)
{
    // 攻撃状態/攻撃可能状態でなければ処理対象外
    if((spritePlayChar.nIsAttack == 0)
    || (spritePlayChar.sp.nState != MYPLAYERCHARACTER_STATE::ATTACK && spritePlayChar.sp.nState != MYPLAYERCHARACTER_STATE::MOVE))
    {
        return;
    }

    // 時間経過が攻撃間隔未満であれば処理対象外
    int nBulletInterval[MYPLAYCHAR_WEAPON_LV_MAX][3] = {
        {  800, 300, 300 },
        {  800, 270, 390 },
        {  800, 240, 480 },
        {  800, 210, 570 },
        {  800, 180, 660 },
        {  800, 150, 630 },
        {  800, 120, 600 },
        {  800,  90, 570 },
        {  800,  60, 540 },
        {  800,  30, 510 }
    };
    WORD nLv = spritePlayChar.nWeaponLevel[spritePlayChar.nWeaponType] - 1;

    if(dwTickCount - dwBulletInterval < nBulletInterval[nLv][spritePlayChar.nWeaponType])
    {
        return;
    }

    // 武器種別ごとの弾生成
    int i, nCreateNum;
    CMyList<SPRITE> *pBuddy;
    switch(spritePlayChar.nWeaponType)
    {
        // 赤(Penetrate)
        case MYPLAYCHAR_WEAPON_RED_INDEX:
            int nAddNum;
            nAddNum = spritePlayChar.nWeaponLevel[0] * 2 % 4;

            nCreateNum = spritePlayChar.nWeaponLevel[0] * 2 / 4;
            nCreateNum += ((nAddNum >= 1) ? 1 : 0);
            for(i = 0; i < nCreateNum; i++)
            {
                CreateBulletR(dwTickCount, spritePlayChar.sp.nPosX + 52, GET_MYPLAYCHAR_POS_Y - 12);
                pBuddy = listBuddy.GetHeadPtr();
                while(pBuddy)
                {
                    CreateBulletR(dwTickCount, pBuddy->data.nPosX + 14, pBuddy->data.nPosY - 12);
                    pBuddy = pBuddy->next;
                }
            }

            nCreateNum = spritePlayChar.nWeaponLevel[0] * 2 / 4;
            nCreateNum += ((nAddNum >= 2) ? 1 : 0);
            for(i = 0; i < nCreateNum; i++)
            {
                CreateBulletR(dwTickCount, spritePlayChar.sp.nPosX + 52, GET_MYPLAYCHAR_POS_Y - 8);
                pBuddy = listBuddy.GetHeadPtr();
                while(pBuddy)
                {
                    CreateBulletR(dwTickCount, pBuddy->data.nPosX + 14, pBuddy->data.nPosY - 8);
                    pBuddy = pBuddy->next;
                }
            }

            nCreateNum = spritePlayChar.nWeaponLevel[0] * 2 / 4;
            nCreateNum += ((nAddNum >= 3) ? 1 : 0);
            for(i = 0; i < nCreateNum; i++)
            {
                CreateBulletR(dwTickCount, spritePlayChar.sp.nPosX + 52, GET_MYPLAYCHAR_POS_Y - 4);
                pBuddy = listBuddy.GetHeadPtr();
                while(pBuddy)
                {
                    CreateBulletR(dwTickCount, pBuddy->data.nPosX + 14, pBuddy->data.nPosY - 4);
                    pBuddy = pBuddy->next;
                }
            }

            nCreateNum = spritePlayChar.nWeaponLevel[0] * 2 / 4;
            for(i = 0; i < nCreateNum; i++)
            {
                CreateBulletR(dwTickCount, spritePlayChar.sp.nPosX + 52, GET_MYPLAYCHAR_POS_Y - 0);
                pBuddy = listBuddy.GetHeadPtr();
                while(pBuddy)
                {
                    CreateBulletR(dwTickCount, pBuddy->data.nPosX + 14, pBuddy->data.nPosY - 0);
                    pBuddy = pBuddy->next;
                }
            }
            break;

        // 緑(Rapid Shot)
        case MYPLAYCHAR_WEAPON_GREEN_INDEX:
            CreateBulletG(dwTickCount, spritePlayChar.sp.nPosX + 52, GET_MYPLAYCHAR_POS_Y - 4);
            pBuddy = listBuddy.GetHeadPtr();
            while(pBuddy)
            {
                CreateBulletG(dwTickCount, pBuddy->data.nPosX + 14, pBuddy->data.nPosY - 4);
                pBuddy = pBuddy->next;
            }
            break;

        // 青(1-5 Way Shot)
        case MYPLAYCHAR_WEAPON_BLUE_INDEX:
            int nIndex, nCorrectX, nCorrectY;

            nCreateNum = (spritePlayChar.nWeaponLevel[MYPLAYCHAR_WEAPON_BLUE_INDEX] + 4) / 5;
            for(i = 0; i < nCreateNum; i++)
            {
                nIndex = (spritePlayChar.nWeaponLevel[MYPLAYCHAR_WEAPON_BLUE_INDEX] - 1) % 5 + 1;
                if(i == 0 && nCreateNum > 1)
                {
                    nIndex = 5;
                }

                if(nIndex & 1)
                {
                    nCorrectX = nCorrectY = 0;
                    if(i == 0)
                    {
                        nCorrectY = -4;
                    }
                    CreateBulletB(dwTickCount, spritePlayChar.sp.nPosX + 52 + nCorrectX, GET_MYPLAYCHAR_POS_Y + nCorrectY, 0, -4);

                    pBuddy = listBuddy.GetHeadPtr();
                    while(pBuddy)
                    {
                        CreateBulletB(dwTickCount, pBuddy->data.nPosX + 14 + nCorrectX, pBuddy->data.nPosY + nCorrectY, 0, -4);
                        pBuddy = pBuddy->next;
                    }
                }

                if(nIndex & 2)
                {
                    nCorrectX = nCorrectY = 0;
                    if(i == 0)
                    {
                        nCorrectX = -1;
                        nCorrectY = -3;
                    }
                    CreateBulletB(dwTickCount, spritePlayChar.sp.nPosX + 52 + nCorrectX, GET_MYPLAYCHAR_POS_Y + nCorrectY, -1, -3);
                    CreateBulletB(dwTickCount, spritePlayChar.sp.nPosX + 52 - nCorrectX, GET_MYPLAYCHAR_POS_Y + nCorrectY,  1, -3);

                    pBuddy = listBuddy.GetHeadPtr();
                    while(pBuddy)
                    {
                        CreateBulletB(dwTickCount, pBuddy->data.nPosX + 14 + nCorrectX, pBuddy->data.nPosY + nCorrectY, -1, -3);
                        CreateBulletB(dwTickCount, pBuddy->data.nPosX + 14 - nCorrectX, pBuddy->data.nPosY + nCorrectY,  1, -3);
                        pBuddy = pBuddy->next;
                    }
                }

                if(nIndex & 4)
                {
                    nCorrectX = nCorrectY = 0;
                    if(i == 0)
                    {
                        nCorrectX = -1;
                        nCorrectY = -3;
                    }
                    CreateBulletB(dwTickCount, spritePlayChar.sp.nPosX + 52 + nCorrectX, GET_MYPLAYCHAR_POS_Y + nCorrectY, -1, -3);
                    CreateBulletB(dwTickCount, spritePlayChar.sp.nPosX + 52 - nCorrectX, GET_MYPLAYCHAR_POS_Y + nCorrectY,  1, -3);

                    if(i == 0)
                    {
                        nCorrectX = -2;
                        nCorrectY = -2;
                    }
                    CreateBulletB(dwTickCount, spritePlayChar.sp.nPosX + 52 + nCorrectX, GET_MYPLAYCHAR_POS_Y + nCorrectY, -2, -2);
                    CreateBulletB(dwTickCount, spritePlayChar.sp.nPosX + 52 - nCorrectX, GET_MYPLAYCHAR_POS_Y + nCorrectY,  2, -2);

                    pBuddy = listBuddy.GetHeadPtr();
                    while(pBuddy)
                    {
                        if(i == 0)
                        {
                            nCorrectX = -1;
                            nCorrectY = -3;
                        }
                        CreateBulletB(dwTickCount, pBuddy->data.nPosX + 14 + nCorrectX, pBuddy->data.nPosY + nCorrectY, -1, -3);
                        CreateBulletB(dwTickCount, pBuddy->data.nPosX + 14 - nCorrectX, pBuddy->data.nPosY + nCorrectY,  1, -3);

                        if(i == 0)
                        {
                            nCorrectX = -2;
                            nCorrectY = -2;
                        }
                        CreateBulletB(dwTickCount, pBuddy->data.nPosX + 14 + nCorrectX, pBuddy->data.nPosY + nCorrectY, -2, -2);
                        CreateBulletB(dwTickCount, pBuddy->data.nPosX + 14 - nCorrectX, pBuddy->data.nPosY + nCorrectY,  2, -2);
                        pBuddy = pBuddy->next;
                    }
                }
            } 
            break;
    }

    dwBulletInterval = dwTickCount;
}

// 弾スプライト生成(R)
void CMyExampleGame::CreateBulletR(DWORD dwTickCount, int x, int y)
{
    CMyList<MYBULLET> *ptr;

    // X座標が壁(300-316)に接する場合は弾生成なし
    if((x + 3 >= 300) && (x < 317))
    {
        return;
    }

    // 弾の初期値をセット
    ptr = new CMyList<MYBULLET>;
    if(ptr == NULL)
    {
        return;
    }

    ptr->data.nMoveX = 0;
    ptr->data.nMoveY = -8;
    ptr->data.sp.byAlpha = 255;
    ptr->data.sp.dwDelay = 16;
    ptr->data.sp.dwLastTickCount = dwTickCount;
    ptr->data.sp.nCurrentCell = 0;
    ptr->data.sp.nMaxCell = 1;
    ptr->data.sp.nPosX = x;
    ptr->data.sp.nPosY = y;
    ptr->data.sp.nState = MYPLAYCHAR_WEAPON_RED_INDEX;
    ptr->data.sp.rcImage = { 320, 64, 324, 68 };

    listBullet.AddTail(ptr);
}

// 弾スプライト生成(G)
void CMyExampleGame::CreateBulletG(DWORD dwTickCount, int x, int y)
{
    CMyList<MYBULLET> *ptr;

    // X座標が壁(300-316)に接する場合は弾生成なし
    if((x + 3 >= 300) && (x < 317))
    {
        return;
    }

    // 弾の初期値をセット
    ptr = new CMyList<MYBULLET>;
    if(ptr == NULL)
    {
        return;
    }

    ptr->data.nMoveX = 0;
    ptr->data.nMoveY = -4;
    ptr->data.sp.byAlpha = 255;
    ptr->data.sp.dwDelay = 16;
    ptr->data.sp.dwLastTickCount = dwTickCount;
    ptr->data.sp.nCurrentCell = 0;
    ptr->data.sp.nMaxCell = 1;
    ptr->data.sp.nPosX = x;
    ptr->data.sp.nPosY = y;
    ptr->data.sp.nState = MYPLAYCHAR_WEAPON_GREEN_INDEX;
    ptr->data.sp.rcImage = { 324, 64, 328, 68 };

    listBullet.AddTail(ptr);
}

// 弾スプライト生成(B)
void CMyExampleGame::CreateBulletB(DWORD dwTickCount, int x, int y, int nMoveX, int nMoveY)
{
    CMyList<MYBULLET> *ptr;

    // X座標が壁(300-316)に接する場合は弾生成なし
    if((x + 3 >= 300) && (x < 317))
    {
        return;
    }

    // 弾の初期値をセット
    ptr = new CMyList<MYBULLET>;
    if(ptr == NULL)
    {
        return;
    }

    ptr->data.nMoveX = nMoveX;
    ptr->data.nMoveY = nMoveY;
    ptr->data.sp.byAlpha = 255;
    ptr->data.sp.dwDelay = 16;
    ptr->data.sp.dwLastTickCount = dwTickCount;
    ptr->data.sp.nCurrentCell = 0;
    ptr->data.sp.nMaxCell = 1;
    ptr->data.sp.nPosX = x;
    ptr->data.sp.nPosY = y;
    ptr->data.sp.nState = MYPLAYCHAR_WEAPON_BLUE_INDEX;
    ptr->data.sp.rcImage = { 328, 64, 332, 68 };

    listBullet.AddTail(ptr);
}

////////////////////////////////////////////////////////////////
// 弾スプライト更新
void CMyExampleGame::UpdateBullet(DWORD dwTickCount)
{
    BOOL bDelete;
    int nWidth, nHeight, nHeightCorrect;
    CMyList<MYBULLET> *ptr;
    CMyList<MYENEMY> *pEnemy;
    CMyList<CMyGameItem> *pItem;

    // 弾の移動
    ptr = listBullet.GetHeadPtr();
    while(ptr)
    {
        bDelete = FALSE;

        // 時間経過で弾移動
        if(dwTickCount - ptr->data.sp.dwLastTickCount > ptr->data.sp.dwDelay)
        {
            // 壁との衝突判定
            if(ptr->data.nMoveX > 0)
            {
                if((ptr->data.sp.nPosX + ptr->data.nMoveX + 4 > GAMESCREEN_WIDTH)
                || (ptr->data.sp.nPosX + ptr->data.nMoveX + 4 > 300 && ptr->data.sp.nPosX + ptr->data.nMoveX + 4 < 316))
                {
                    ptr->data.nMoveX = -(ptr->data.nMoveX);
                }
            }
            else if(ptr->data.nMoveX < 0)
            {
                if((ptr->data.sp.nPosX + ptr->data.nMoveX < 0)
                || (ptr->data.sp.nPosX + ptr->data.nMoveX < 316 && ptr->data.sp.nPosX + ptr->data.nMoveX > 300))
                {
                    ptr->data.nMoveX = -(ptr->data.nMoveX);
                }
            }

            ptr->data.sp.nPosX += ptr->data.nMoveX;
            ptr->data.sp.nPosY += ptr->data.nMoveY;
            ptr->data.sp.dwLastTickCount = dwTickCount;
        }

        // 画面外の弾は削除
        if(ptr->data.sp.nPosY < 0)
        {
            bDelete = TRUE;
        }

        // 弾と敵の衝突判定
        pEnemy = listEnemy.GetHeadPtr();
        while(pEnemy)
        {
            // 衝突判定
            nWidth = pEnemy->data.sp.rcImage.right - pEnemy->data.sp.rcImage.left;
            nHeight = pEnemy->data.sp.rcImage.bottom - pEnemy->data.sp.rcImage.top;
            if((pEnemy->data.sp.nState != MYPLAYERCHARACTER_STATE::LOSE)
            && (ptr->data.sp.nPosX < pEnemy->data.sp.nPosX + nWidth)  && (ptr->data.sp.nPosX + 4 > pEnemy->data.sp.nPosX)
            && (ptr->data.sp.nPosY < pEnemy->data.sp.nPosY + nHeight) && (ptr->data.sp.nPosY + 4 > pEnemy->data.sp.nPosY))
            {
                // 同一属性ならダメージ増
                pEnemy->data.nLife -= (ptr->data.sp.nState == pEnemy->data.nWeaponType) ? 2 : 1;

                // 撃破にともなう処理
                if(pEnemy->data.nLife <= 0)
                {
                    dwScore += pEnemy->data.dwScore;
                    if(pEnemy->data.bDispLife == TRUE)
                    {
                        pEnemy->data.sp.rcImage.left += MYBOSS_SIZE;
                        pEnemy->data.sp.rcImage.right += MYBOSS_SIZE;
                    }
                    else
                    {
                        if(pEnemy->data.sp.rcImage.left < 128)
                        {
                            pEnemy->data.sp.rcImage.left = 96;
                            pEnemy->data.sp.rcImage.right = 128;
                        }
                        else
                        {
                            pEnemy->data.sp.rcImage.left = 224;
                            pEnemy->data.sp.rcImage.right = 256;
                        }
                    }
                    pEnemy->data.nLife = 0;
                    pEnemy->data.bDispLife = FALSE;
                    pEnemy->data.nAttack = 0;
                    pEnemy->data.nMoveX = 0;
                    pEnemy->data.nMoveY = 0;
                    pEnemy->data.sp.byAlpha = 128;
                    pEnemy->data.sp.dwDelay = 300;
                    pEnemy->data.sp.dwLastTickCount = dwTickCount;
                    pEnemy->data.sp.nState = MYPLAYERCHARACTER_STATE::LOSE;
                }

                // 衝突した弾は削除
                bDelete = TRUE;
            }

            pEnemy = pEnemy->next;
        }

        // 弾とアイテムの衝突判定
        pItem = listItem.GetHeadPtr();
        while(pItem)
        {
            // 衝突判定
            nWidth = pItem->data.rcLowSide.right - pItem->data.rcLowSide.left;
            nHeight = pItem->data.rcLowSide.bottom - pItem->data.rcLowSide.top;
            nHeightCorrect = pItem->data.rcLowSide.top - pItem->data.rcUpSide.top;
            if((pItem->data.bActive == TRUE)
            && (pItem->data.sp.nState != MYPLAYERCHARACTER_STATE::LOSE)
            && (ptr->data.sp.nPosX < pItem->data.sp.nPosX + nWidth)
            && (ptr->data.sp.nPosX + 4 > pItem->data.sp.nPosX)
            && (ptr->data.sp.nPosY < pItem->data.sp.nPosY + nHeight + nHeightCorrect)
            && (ptr->data.sp.nPosY + 4 > pItem->data.sp.nPosY + nHeightCorrect))
            {
                // 同一属性ならダメージ増
                pItem->data.nLife -= (ptr->data.sp.nState == pItem->data.sp.nCurrentCell) ? 2 : 1;

                // 開放にともなう処理
                if(pItem->data.nLife <= 0)
                {
                    pItem->data.nLife = 0;
                    pItem->data.nMoveY = 0;
                    pItem->data.sp.byAlpha = 128;
                    pItem->data.sp.dwDelay = 300;
                    pItem->data.sp.dwLastTickCount = dwTickCount;
                    pItem->data.sp.nState = MYPLAYERCHARACTER_STATE::LOSE;
                }

                // 衝突した弾は削除
                bDelete = TRUE;
            }

            pItem = pItem->next;
        }

        // 次ポインタを取得
        if(bDelete == TRUE)
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

    int x, y, nEnemyType, nEnemyAttr;
    DWORD dwCnt, dwCreateNum, dwEnemyRank, dwBossCnt;

    dwEnemyRank = dwWaveCount / 40;
    dwBossCnt = dwWaveCount % 100;
    dwCreateNum = (dwWaveCount / 2 < 100) ? dwWaveCount / 2 : 100;
    for(dwCnt = 0; dwCnt < dwCreateNum; dwCnt++)
    {
        // BOSS出現判定
        if((dwBossCnt % 10 == 0) && (dwCnt == dwCreateNum - 1))
        {
            // BOSS出現(1体)
            if(dwCnt < 50)
            {
                y = 0 - MYBOSS_SIZE - (dwCnt / 10 * 4);
                x = (300 - MYBOSS_SIZE) / 2;
                nEnemyAttr = GetMyRandVal(0, 2);
                CreateEnemyData(dwTickCount, x, y, 0, nEnemyAttr, dwEnemyRank, TRUE);
            }
            // BOSS出現(2体)
            else if(dwCnt < 80)
            {
                y = 0 - MYBOSS_SIZE - (dwCnt / 10 * 4);
                x = (300 / 2 - MYBOSS_SIZE) / 2;
                nEnemyAttr = GetMyRandVal(0, 2);
                CreateEnemyData(dwTickCount, x, y, 0, nEnemyAttr, dwEnemyRank, TRUE);

                x = x + 300 / 2;
                nEnemyAttr = GetMyRandVal(0, 2);
                CreateEnemyData(dwTickCount, x, y, 0, nEnemyAttr, dwEnemyRank, TRUE);
            }
            // BOSS出現(3体)
            else if(dwCnt < 100)
            {
                y = 0 - MYBOSS_SIZE - (dwCnt / 10 * 4);
                x = (300 / 3 - MYBOSS_SIZE) / 2;
                nEnemyAttr = GetMyRandVal(0, 2);
                CreateEnemyData(dwTickCount, x, y, 0, nEnemyAttr, dwEnemyRank, TRUE);

                x = x + 300 / 3;
                nEnemyAttr = GetMyRandVal(0, 2);
                CreateEnemyData(dwTickCount, x, y, 0, nEnemyAttr, dwEnemyRank, TRUE);

                x = x + 300 / 3;
                nEnemyAttr = GetMyRandVal(0, 2);
                CreateEnemyData(dwTickCount, x, y, 0, nEnemyAttr, dwEnemyRank, TRUE);
            }
        }

        // 通常
        x = GetMyRandVal(24, 268);
        y = 0 - MYENEMY_SIZE - (dwCnt / 10 * 4);
        nEnemyType = GetMyRandVal(0, 1);
        nEnemyAttr = GetMyRandVal(0, 2);
        CreateEnemyData(dwTickCount, x, y, nEnemyType, nEnemyAttr, dwEnemyRank, FALSE);
    }

    dwWaveInterval = dwTickCount;
}

void CMyExampleGame::CreateEnemyData(DWORD dwTickCount, int x, int y, int nType, int nAttr, DWORD dwRank, BOOL bBoss)
{
    CMyList<MYENEMY> *ptr;
    ptr = new CMyList<MYENEMY>;
    if(ptr == NULL)
    {
        return;
    }

    ptr->data.sp.byAlpha = 255;
    ptr->data.sp.dwDelay = 120;
    ptr->data.sp.dwLastTickCount = dwTickCount;
    ptr->data.sp.nCurrentCell = 0;
    ptr->data.sp.nMaxCell = 1;
    ptr->data.sp.nPosX = x;
    ptr->data.sp.nPosY = y;
    ptr->data.sp.nState = MYPLAYERCHARACTER_STATE::STAND;
    ptr->data.nMoveX = 0;
    ptr->data.nMoveY = 4;
    ptr->data.nWeaponType = nAttr;

    if(bBoss == TRUE)
    {
        ptr->data.sp.rcImage = { nAttr * MYBOSS_SIZE, 192, (nAttr + 1) * MYBOSS_SIZE, 256 };
        ptr->data.bDispLife = TRUE;
        ptr->data.dwScore = (dwRank + 1) * 100;
        ptr->data.nAttack = 5;
        ptr->data.nLife = ptr->data.nLifeMax = ((dwRank + 1) * 10 < 250) ? (dwRank + 1) * 10 : 250;
    }
    else
    {
        ptr->data.sp.rcImage = { nType * 128 + nAttr * MYENEMY_SIZE, 288, nType * 128 + (nAttr + 1) * MYENEMY_SIZE, 320 };
        ptr->data.bDispLife = FALSE;
        ptr->data.dwScore = dwRank + 1;
        ptr->data.nAttack = 1;
        ptr->data.nLife = ptr->data.nLifeMax = (dwRank + 2 < 12) ? dwRank + 2 : 12;
    }

    listEnemy.AddTail(ptr);
}

////////////////////////////////////////////////////////////////
// 敵スプライト更新
void CMyExampleGame::UpdateEnemy(DWORD dwTickCount)
{
    int nHeight;
    BOOL bDelete;
    CMyList<MYENEMY> *ptr;
    CMyList<SPRITE> *pBuddy;

    // 弾の移動
    ptr = listEnemy.GetTailPtr();
    while(ptr)
    {
        bDelete = FALSE;

        // 時間経過で敵移動
        if(dwTickCount - ptr->data.sp.dwLastTickCount > ptr->data.sp.dwDelay)
        {
            // 撃破された敵は削除
            if(ptr->data.sp.nState == MYPLAYERCHARACTER_STATE::LOSE)
            {
                bDelete = TRUE;
            }
            // 生存している敵は移動
            {
                ptr->data.sp.nPosX += ptr->data.nMoveX;
                ptr->data.sp.nPosY += ptr->data.nMoveY;
                ptr->data.sp.dwLastTickCount = dwTickCount;

                // 操作キャラに触れるとダメージ
                nHeight = ptr->data.sp.rcImage.bottom - ptr->data.sp.rcImage.top;
                if(ptr->data.sp.nPosY + nHeight > GET_MYPLAYCHAR_POS_Y)
                {
                    // 武器持ち替え中は無敵
                    if(spritePlayChar.sp.nState != MYPLAYERCHARACTER_STATE::TURN)
                    {
                        spritePlayChar.nLife -= ptr->data.nAttack;
                    }

                    if(spritePlayChar.nLife <= 0)
                    {
                        spritePlayChar.nLife = 0;

                        // 味方がいれば身代わり
                        pBuddy = listBuddy.GetTailPtr();
                        while(pBuddy)
                        {
                            if(pBuddy->data.nState == MYPLAYERCHARACTER_STATE::LOSE)
                            {
                                pBuddy = pBuddy->prev;
                            }
                            else
                            {
                                spritePlayChar.nLife = spritePlayChar.nLifeMax;
                                pBuddy->data.nState = MYPLAYERCHARACTER_STATE::LOSE;
                                pBuddy->data.nCurrentCell = 0;
                                pBuddy->data.dwLastTickCount = dwTickCount;
                                pBuddy->data.byAlpha = 128;
                                pBuddy->data.nMaxCell = nImageCell_pc[MYPLAYERCHARACTER_STATE::LOSE][2];
                                pBuddy->data.dwDelay = nImageCell_pc[MYPLAYERCHARACTER_STATE::LOSE][3];
                                break;
                            }
                        }
                    }

                    // 操作キャラに触れたらノックバック
                    if(ptr->data.sp.nPosY + nHeight > GET_MYPLAYCHAR_POS_Y + ptr->data.nMoveY)
                    {
                        ptr->data.sp.nPosY = GET_MYPLAYCHAR_POS_Y - nHeight;
                    }
                }
            }
        }

        // 前のポインタを取得
        if(bDelete == TRUE)
        {
            ptr = listEnemy.DeleteElement(ptr, FALSE);
        }
        else
        {
            ptr = ptr->prev;
        }
    }
}

////////////////////////////////////////////////////////////////
// アイテム生成
void CMyExampleGame::CreateGameItem(DWORD dwTickCount)
{
    // 操作不能の場合は処理対象外
    if(spritePlayChar.nLife <= 0)
    {
        return;
    }

    // 時間経過がアイテム出現間隔未満であれば処理対象外
    if(dwTickCount - dwItemInterval < MYITEM_WAVE_INTERVAL)
    {
        return;
    }

    int i, nRandVal;
    CMyList<CMyGameItem> *ptr;
    CMyList<SPRITE> *ptrBuddy;
    int nItemIDRotate[GETITEM_IDX_MAX] = {
        GETITEM_GUN_R, GETITEM_GUN_G, GETITEM_GUN_B,
        GETITEM_BUDDY1, GETITEM_BUDDY2, GETITEM_BUDDY3,
        GETITEM_RECOVER
    };
    DWORD dwCreateFlag = nItemIDRotate[wItemIndex];

    // 生成済みチェック
    ptr = listItem.GetHeadPtr();
    while(ptr)
    {
        if(ptr->data.dwItemID != dwCreateFlag)
        {
            ptr = ptr->next;
            continue;
        }

        // 生成済みアイテムの場合はデータを使いまわす
        switch(dwCreateFlag)
        {
            // 武器データのリセット
            case GETITEM_GUN_R:
            case GETITEM_GUN_G:
            case GETITEM_GUN_B:
                ptr->data.SetInitDataWeapon(this, dwCreateFlag, dwTickCount, ptr->data.sp.nCurrentCell);
                break;

            // 味方データのリセット
            case GETITEM_BUDDY1:
                ptr->data.SetInitDataBuddy(this, dwCreateFlag, dwTickCount, 0);
                break;
            case GETITEM_BUDDY2:
                ptr->data.SetInitDataBuddy(this, dwCreateFlag, dwTickCount, 1);
                break;
            case GETITEM_BUDDY3:
                ptr->data.SetInitDataBuddy(this, dwCreateFlag, dwTickCount, 2);
                break;

            // 回復データのリセット
            case GETITEM_RECOVER:
                ptr->data.SetInitDataOther(this, dwCreateFlag, dwTickCount);
                break;

            default:
                break;
        }
        ptr->data.JudgeDisplay();
        dwItemInterval = dwTickCount;
        wItemIndex = (wItemIndex + 1) % GETITEM_IDX_MAX;
        return;
    }
    
    // 武器アイテム生成
    DWORD a_dwWeaponID[3] = { GETITEM_GUN_R, GETITEM_GUN_G, GETITEM_GUN_B };
    for(i = 0; i < 3; i++)
    {
        if(dwCreateFlag & a_dwWeaponID[i])
        {
            ptr = new CMyList<CMyGameItem>;
            if(ptr != NULL)
            {
                ptr->data.SetInitDataWeapon(this, a_dwWeaponID[i], dwTickCount, i);
                ptr->data.ResetLife();
                ptr->data.JudgeDisplay();
                listItem.AddTail(ptr);
                dwItemInterval = dwTickCount;
            }
        }
    }

    // 味方生成
    DWORD a_dwBuddyID[3] = { GETITEM_BUDDY1, GETITEM_BUDDY2, GETITEM_BUDDY3 };
    for(i = 0; i < 3; i++)
    {
        if(dwCreateFlag & a_dwBuddyID[i])
        {
            // 味方キャラが存在するかチェック
            ptrBuddy = listBuddy.GetHeadPtr();
            while(ptrBuddy)
            {
                if(ptrBuddy->data.dwSpriteID == a_dwBuddyID[i])
                {
                    break;
                }
    
                ptrBuddy = ptrBuddy->next;
            }
    
            // 存在しなければ生成
            if(ptrBuddy == NULL)
            {
                ptr = new CMyList<CMyGameItem>;
                if(ptr != NULL)
                {
                    ptr->data.SetInitDataBuddy(this, a_dwBuddyID[i], dwTickCount, i);
                    ptr->data.sp.nCurrentCell = GetMyRandVal(0, 2);
                    ptr->data.ResetLife();
                    ptr->data.JudgeDisplay();
                    listItem.AddTail(ptr);
                    dwItemInterval = dwTickCount;
                }
            }
        }
    }

    // 回復生成
    if(dwCreateFlag & GETITEM_RECOVER)
    {
        ptr = new CMyList<CMyGameItem>;
        if(ptr != NULL)
        {
            ptr->data.SetInitDataOther(this, GETITEM_RECOVER, dwTickCount);
            ptr->data.sp.nCurrentCell = GetMyRandVal(0, 2);
            ptr->data.ResetLife();
            ptr->data.JudgeDisplay();
            listItem.AddTail(ptr);
            dwItemInterval = dwTickCount;
        }
    }

    wItemIndex = (wItemIndex + 1) % GETITEM_IDX_MAX;
}

////////////////////////////////////////////////////////////////
// アイテム更新
void CMyExampleGame::UpdateGameItem(DWORD dwTickCount)
{
    int nHeightLow, nHeightAll;
    BOOL bDelete;
    CMyList<CMyGameItem> *ptr;

    // アイテムの移動
    ptr = listItem.GetHeadPtr();
    while(ptr)
    {
        bDelete = FALSE;

        // 時間経過でアイテム移動
        if(dwTickCount - ptr->data.sp.dwLastTickCount > ptr->data.sp.dwDelay)
        {
            // 開放されたアイテムは削除
            if(ptr->data.sp.nState == MYPLAYERCHARACTER_STATE::LOSE)
            {
                bDelete = TRUE;

                // アイテム効果発動
                if(ptr->data.pfunc != NULL)
                {
                    (this->*ptr->data.pfunc)(&(ptr->data), dwTickCount);
                }
            }
            // 開放されていないアイテムは移動
            else if(ptr->data.bActive == TRUE)
            {
                ptr->data.sp.nPosY += ptr->data.nMoveY;
                ptr->data.sp.dwLastTickCount = dwTickCount;

                // 操作キャラに触れたら非表示/非活性
                nHeightAll = ptr->data.rcLowSide.bottom - ptr->data.rcUpSide.top;
                nHeightLow = ptr->data.rcLowSide.bottom - ptr->data.rcLowSide.top;
                if(ptr->data.sp.nPosY + nHeightAll > GET_MYPLAYCHAR_POS_Y + nHeightLow)
                {
                    ptr->data.bActive = FALSE;
                }
            }
        }

        // 次のポインタを取得
        if(bDelete == TRUE)
        {
            ptr = listItem.DeleteElement(ptr);
        }
        else
        {
            ptr = ptr->next;
        }
    }
}

////////////////////////////////////////////////////////////////
// アイテム獲得(武器)
void CMyExampleGame::ChangeWeapon(CMyGameItem *ptrItem, DWORD dwTickCount)
{
    WORD wType = (WORD)(ptrItem->sp.nCurrentCell);

    // 異種なら持ち替え
    if(spritePlayChar.nWeaponType != wType)
    {
        spritePlayChar.nWeaponType = wType;
    }
    // 同種ならレベルアップ
    else
    {
        spritePlayChar.nWeaponLevel[wType]++;

        // レベル上限(10)に到達すれば同種の敵を撃破
        if(spritePlayChar.nWeaponLevel[wType] >= 10)
        {
            CMyList<MYENEMY> *ptr;
            ptr = listEnemy.GetHeadPtr();
            while(ptr)
            {
                if(ptr->data.nWeaponType == wType)
                {
                    dwScore += ptr->data.dwScore;
                    if(ptr->data.bDispLife == TRUE)
                    {
                        ptr->data.sp.rcImage.left += MYBOSS_SIZE;
                        ptr->data.sp.rcImage.right += MYBOSS_SIZE;
                    }
                    else
                    {
                        if(ptr->data.sp.rcImage.left < 128)
                        {
                            ptr->data.sp.rcImage.left = 96;
                            ptr->data.sp.rcImage.right = 128;
                        }
                        else
                        {
                            ptr->data.sp.rcImage.left = 224;
                            ptr->data.sp.rcImage.right = 256;
                        }
                    }
                    ptr->data.nLife = 0;
                    ptr->data.bDispLife = FALSE;
                    ptr->data.nAttack = 0;
                    ptr->data.nMoveX = 0;
                    ptr->data.nMoveY = 0;
                    ptr->data.sp.byAlpha = 128;
                    ptr->data.sp.dwDelay = 300;
                    ptr->data.sp.dwLastTickCount = dwTickCount;
                    ptr->data.sp.nState = MYPLAYERCHARACTER_STATE::LOSE;
                }

                ptr = ptr->next;
            }

            spritePlayChar.nWeaponLevel[wType] = 10;
        }
    }
}

////////////////////////////////////////////////////////////////
// アイテム獲得(味方)
void CMyExampleGame::AddBuddy(CMyGameItem *ptrItem, DWORD dwTickCount)
{
    int nNum;
    CMyList<SPRITE> *ptrBuddy;

    // 既に味方がいるなら無効
    nNum = listBuddy.GetNumOfElement();
    if(nNum >= 3)
    {
        return;
    }

    // 重複する味方がいるなら無効
    ptrBuddy = listBuddy.GetHeadPtr();
    while(ptrBuddy)
    {
        if(ptrBuddy->data.dwSpriteID == ptrItem->dwItemID)
        {
            return;
        }

        ptrBuddy = ptrBuddy->next;
    }

    ptrBuddy = new CMyList<SPRITE>;
    if(ptrBuddy == NULL)
    {
        return;
    }

    // 初期値セット
    int nIndex = ptrItem->sp.dwSpriteID;

    ptrBuddy->data.dwSpriteID = ptrItem->dwItemID;
    ptrBuddy->data.rcImage = { nIndex * 128, 256, nIndex * 128 + MYPLAYBUDDY_SIZE, 288 };
    ptrBuddy->data.nPosX = spritePlayChar.sp.nPosX + nBuddyPosX[nNum];
    ptrBuddy->data.nPosY = spritePlayChar.sp.nPosY + nBuddyPosY[nNum];
    ptrBuddy->data.nState = MYPLAYERCHARACTER_STATE::STAND;
    ptrBuddy->data.byAlpha = 255;
    ptrBuddy->data.nCurrentCell = 0;
    ptrBuddy->data.nMaxCell = nImageCell_pc[MYPLAYERCHARACTER_STATE::STAND][2];
    ptrBuddy->data.dwDelay = nImageCell_pc[MYPLAYERCHARACTER_STATE::STAND][3];
    ptrBuddy->data.dwLastTickCount = spritePlayChar.sp.dwLastTickCount;

    if(ptrBuddy->data.nPosX < 0)
    {
        ptrBuddy->data.nPosX = 0;
    }

    if(ptrBuddy->data.nPosX > GAMESCREEN_WIDTH - MYPLAYBUDDY_SIZE)
    {
        ptrBuddy->data.nPosX = GAMESCREEN_WIDTH - MYPLAYBUDDY_SIZE;
    }

    listBuddy.AddTail(ptrBuddy);

    // 回復
    spritePlayChar.nLife = spritePlayChar.nLifeMax;
}


////////////////////////////////////////////////////////////////
// ゲームアイテムクラス
////////////////////////////////////////////////////////////////
// 武器アイテム初期値セット
void CMyGameItem::SetInitDataWeapon(CMyExampleGame *pgame, DWORD dwID, DWORD dwTickCount, int nState)
{
    dwItemID = dwID;
    bActive = FALSE;
    nRate = (pgame->spritePlayChar.nWeaponLevel[nState] < 10) ? 100 : 50;
    nCountMax = nCount = 3;
    nLifeMax = 72 * pgame->spritePlayChar.nWeaponLevel[nState];
    nMoveY = 1;
    rcUpSide =  { 256, 288, 328, 296 };
    rcLowSide = { 256, 368, 328, 384 };
    sp.rcImage = { nState * 64, 320, (nState + 1) * 64, 384 };
    sp.nPosX = 322;
    sp.nPosY = -96;
    sp.nState = MYPLAYERCHARACTER_STATE::MOVE;
    sp.byAlpha = 255;
    sp.nCurrentCell = nState;
    sp.nMaxCell = 1;
    sp.dwDelay = 16;
    sp.dwLastTickCount = dwTickCount;
    pfunc = CMyExampleGame::ChangeWeapon;
}

////////////////////////////////////////////////////////////////
// 味方アイテム初期値セット
void CMyGameItem::SetInitDataBuddy(CMyExampleGame *pgame, DWORD dwID, DWORD dwTickCount, int nState)
{
    dwItemID = dwID;
    bActive = FALSE;
    nRate = 100;
    nCountMax = nCount = 1;
    nLifeMax = 72;
    nMoveY = 1;
    rcUpSide =  { 256, 288, 328, 296 };
    rcLowSide = { 256, 368, 328, 384 };
    sp.rcImage = { nState * 128 + 96, 256, (nState + 1) * 128, 288 };
    sp.dwSpriteID = nState;
    sp.nPosX = 322;
    sp.nPosY = -96;
    sp.nState = MYPLAYERCHARACTER_STATE::MOVE;
    sp.byAlpha = 255;
    sp.nMaxCell = 1;
    sp.dwDelay = 16;
    sp.dwLastTickCount = dwTickCount;
    pfunc = CMyExampleGame::AddBuddy;
}

////////////////////////////////////////////////////////////////
// 回復アイテム初期値セット
void CMyGameItem::SetInitDataOther(CMyExampleGame *pgame, DWORD dwID, DWORD dwTickCount)
{
    dwItemID = dwID;
    bActive = FALSE;
    nRate = 100 - (int)(pgame->spritePlayChar.nLife * 100 / MYPLAYCHAR_LIFE);
    nCountMax = nCount = 4;
    nLifeMax = 72;
    nMoveY = 1;
    rcUpSide =  { 256, 288, 328, 296 };
    rcLowSide = { 256, 368, 328, 384 };
    sp.rcImage = { 192, 320, 256, 384 };
    sp.dwSpriteID = dwID;
    sp.nPosX = 322;
    sp.nPosY = -96;
    sp.nState = MYPLAYERCHARACTER_STATE::MOVE;
    sp.byAlpha = 255;
    sp.nMaxCell = 1;
    sp.dwDelay = 16;
    sp.dwLastTickCount = dwTickCount;
    pfunc = CMyExampleGame::RecoverLife;
}

////////////////////////////////////////////////////////////////
// アイテム出現判定
void CMyGameItem::JudgeDisplay()
{
    int nRandVal = GetMyRandVal(0, 99);
    nCount--;

    // 確率判定or天井カウントに到達
    if((nRandVal < nRate) || (nCount <= 0))
    {
        nCount = nCountMax;
        bActive = TRUE;
    }
}

