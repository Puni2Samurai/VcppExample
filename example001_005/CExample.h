#include "CDib.h"

////////////////////////////////////////////////////////////////
// スプライト定義
////////////////////////////////////////////////////////////////
// スプライトの状態
enum
{
    SPRITE_STATE_STAY,  // 立ち
    SPRITE_STATE_WALK,  // 歩き

    SPRITE_STATE_MAX
};

// スプライトのセル
#define SPRITE_DELAY 125        // スプライトのセル更新周期(ミリ秒)
#define SPRITE_STAY_CELL_LEN 1  // スプライト立ち状態のセル数
#define SPRITE_WALK_CELL_LEN 8  // スプライト歩き状態のセル数

// スプライト構造体
typedef struct tag_sprite
{
    int nPosX;  // 表示座標(X)
    int nPosY;  // 表示座標(Y)
    int nState;  // 状態(0:立ち, 1:歩き)
    int nCurrentCell;  // 標示セル
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

    // DIB
    CDib24 *pDibSprite;
    CDib24 *pDibBackground;
    CDib24 *pDibOffScreen;

    // スプライト
    SPRITE sprite;

public:
    CMyExample() : pDibSprite(NULL), pDibBackground(NULL){}

    BOOL DispErrorMsg(LPCWSTR lpszCaption, LPCWSTR lpszMessage);

    BOOL CreateExampleWindow(HINSTANCE hAppInst, int nShowCmd, int nWidth, int nHeight);
    BOOL InitExample();

    void DrawBackground();
    void DrawSprite(DWORD& dwTickCount);
    void UpdateFrame();
    void UpdateScreen(HDC hDC);

    void ChangeSpriteState();
    void SetSpriteCoord(int nPosX, int nPosY);
    void MoveSpriteCoord(int nMoveX, int nMoveY);
};

