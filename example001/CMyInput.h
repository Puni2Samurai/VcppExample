#include <windows.h>


////////////////////////////////////////////////////////////////
// キー入力定義
////////////////////////////////////////////////////////////////
// キー入力状態
enum MYKEYSTATE
{
    DOWN,   // 押下状態
    PUSH,   // 押下状態へ遷移
    HOLD,   // 押下状態を継続
    BREAK,  // 押下状態が解除
};


////////////////////////////////////////////////////////////////
// キー入力クラス
class CMyKeyInput
{
    int nCurrentIndex;
    POINT ptCursor[2];
    BYTE mk[2][256];

public:
    CMyKeyInput();

    void SetMyKeyState();
    BOOL CheckMyKeyState(int nVKey, MYKEYSTATE nMode);
    BOOL GetMouseCoord(int& nCurX, int &nCurY);
    void SwitchCurrentIndex(){ nCurrentIndex = 1 - nCurrentIndex; }
};

