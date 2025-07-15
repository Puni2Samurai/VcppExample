#include "CMyInput.h"

////////////////////////////////////////////////////////////////
// キーボード入力クラス
////////////////////////////////////////////////////////////////
// コンストラクタ
CMyKeyInput::CMyKeyInput()
{
    nCurrentIndex = 0;
    memset(mk,0, sizeof(mk));
    memset(ptCursor, 0, sizeof(ptCursor));
}

////////////////////////////////////////////////////////////////
// 現在のキー入力状態をセット
void CMyKeyInput::SetMyKeyState()
{
    // カーソル座標を取得
    GetCursorPos(&ptCursor[nCurrentIndex]);

    // すべての仮想キーの状態を取得
    GetKeyboardState(mk[nCurrentIndex]);
}

////////////////////////////////////////////////////////////////
// キー入力状態チェック
BOOL CMyKeyInput::CheckMyKeyState(int nVKey, MYKEYSTATE nMode)
{
    SHORT nCurKey = 0, nOldKey = 0;

    // 指定のキー状態を取得
    nCurKey = mk[nCurrentIndex][nVKey];
    nOldKey = mk[1 - nCurrentIndex][nVKey];

    // 押下状態チェック
    switch(nMode)
    {
        // 現在の押下状態をチェック
        case MYKEYSTATE::DOWN:
            return (nCurKey & 0x80);

        // 押下状態への遷移をチェック
        case MYKEYSTATE::PUSH:
            return ((nCurKey & 0x80) && !(nOldKey & 0x80));

        // 押下状態の継続をチェック
        case MYKEYSTATE::HOLD:
            return ((nCurKey & 0x80) && (nOldKey & 0x80));

        // 押下状態解除をチェック
        case MYKEYSTATE::BREAK:
            return (!(nCurKey & 0x80) && (nOldKey & 0x80));
    }

    return FALSE;
}

////////////////////////////////////////////////////////////////
// マウスカーソルの座標を取得
BOOL CMyKeyInput::GetMouseCoord(int& nCurX, int& nCurY)
{
    int nOldX, nOldY;

    nCurX = ptCursor[nCurrentIndex].x;
    nCurY = ptCursor[nCurrentIndex].y;
    nOldX = ptCursor[1 - nCurrentIndex].x;
    nOldY = ptCursor[1 - nCurrentIndex].y;

    if((nCurX != nOldX) || (nCurY != nOldY))
    {
        return TRUE;
    }

    return FALSE;
}

