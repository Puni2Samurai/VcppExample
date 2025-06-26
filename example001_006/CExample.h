#include "CDib.h"
#include "CMyInput.h"

#define MYEXAMPLE_REFLASH_RATE 16

////////////////////////////////////////////////////////////////
// スプライト定義
////////////////////////////////////////////////////////////////
// スプライトの状態
enum
{
    SPRITE_STATE_STAY,  // 立ち
    SPRITE_STATE_WALK,  // 歩き

    SPRITE_STATE_INIT,
    SPRITE_STATE_GLAD,

    SPRITE_STATE_MAX
};

// スプライトのセル
#define SPRITE_DELAY 100        // スプライトのセル更新周期(ミリ秒)
#define SPRITE_STAY_CELL_LEN 1  // スプライト立ち状態のセル数
#define SPRITE_WALK_CELL_LEN 8  // スプライト歩き状態のセル数

// スプライト構造体
typedef struct tag_sprite
{
    RECT rcImage;  // イメージ内座標
    int nPosX;  // 表示座標(X)
    int nPosY;  // 表示座標(Y)
    int nMoveX;  // 移動量(X)
    int nMoveY;  // 移動量(Y)
    int nState;  // 状態(0:立ち, 1:歩き)
    int nCurrentCell;  // 表示セル
    int nMaxCell;  // 最大セル数
    DWORD dwDelay;  // 描画更新カウント
    DWORD dwLastTickCount;  // 経過時間カウント
} SPRITE;

////////////////////////////////////////////////////////////////
// アプリ固有クラス
////////////////////////////////////////////////////////////////
class CMyExample : public CCommon
{
protected:
    // ウィンドウ情報
    static HWND hwnd;
    static HINSTANCE hThisInst;
    int nWinMode;
    int nWindowWidth;
    int nWindowHeight;

    DWORD dwLastTickCount;

    // DIB
    CDib24 *pDibSprite;
    CDib24 *pDibBackground;
    CDib24 *pDibOffScreen;

    // スプライト
    SPRITE sprite;
    SPRITE spriteCPU;
    BYTE byAlphaBlend;
    int nAlphaBlendInc;

    // キー入力
    CMyKeyboard mykey;
    CMyMouse mymouse;

public:
    CMyExample() : pDibSprite(NULL), pDibBackground(NULL), pDibOffScreen(NULL){};
    ~CMyExample(){ FreeExample(); }

    BOOL DispErrorMsg(LPCWSTR lpszCaption, LPCWSTR lpszMessage);

    BOOL CreateExampleWindow(HINSTANCE hAppInst, int nShowCmd, int nWidth, int nHeight);
    BOOL InitExample();
    void FreeExample();

    void DrawBackground();
    void DrawSprite(DWORD dwTickCount);
    void DrawOtherSprite(DWORD dwTickCount);
    void CheckKeyInput();
    void UpdateFrame();
    void UpdateScreen(HDC hDC);

    void ChangeSpriteState();
    void SetSpriteCoord(int nPosX, int nPosY);
    void MoveSpriteCoord(SPRITE& sp, int nMoveX, int nMoveY);
    void MoveOtherSpriteCoord();
    void SetOtherSpriteMove();
    BOOL CollisionDetect();
};

