#include <windows.h>

////////////////////////////////////////////////////////////////
// キーボード入力定義
////////////////////////////////////////////////////////////////
// 仮想キー
#define VK_Z 0x5A
#define VK_X 0x58

// キー入力状態
enum
{
    MYKEYSTATE_DOWN,   // 押下状態
    MYKEYSTATE_HOLD,   // 押下状態を継続
    MYKEYSTATE_BREAK,  // 押下状態が解除
    MYKEYSTATE_PRESS,  // 押下状態へ遷移
};


////////////////////////////////////////////////////////////////
// キーボード入力定義
////////////////////////////////////////////////////////////////
// キーボード入力構造体
typedef struct tag_mykeyboardstate
{
    SHORT nLeft;    // テンキー4
    SHORT nRight;   // テンキー6
    SHORT nUp;      // テンキー8
    SHORT nDown;    // テンキー2
    SHORT nButton1; // Zキー
    SHORT nButton2; // Xキー
} MYKEYBOARDSTATE;

////////////////////////////////////////////////////////////////
// キーボード入力クラス
class CMyKeyboard
{
    int nCurrentIndex;
    MYKEYBOARDSTATE mk[2];

public:
    CMyKeyboard();

    void SetMyKeyState();
    BOOL CheckMyKeyState(int nVKey, int nMode);
    void SwitchCurrentIndex(){ nCurrentIndex = 1 - nCurrentIndex; }
};


////////////////////////////////////////////////////////////////
// マウス入力定義
////////////////////////////////////////////////////////////////
// マウス入力構造体
typedef struct tag_mymousestate
{
    POINT ptCursor;
    SHORT nLeft;
    SHORT nRight;
} MYMOUSESTATE;

////////////////////////////////////////////////////////////////
// マウス入力クラス
class CMyMouse
{
    int nCurrentIndex;
    MYMOUSESTATE ms[2];

public:
    CMyMouse();

    void SetMouseState();
    void GetMouseCoord(int& nCoordX, int& nCoordY);
    BOOL CheckButtonState(int nVKey, int nMode);
    void SwitchCurrentIndex(){ nCurrentIndex = 1 - nCurrentIndex; }
};
