#include "CMyInput.h"

////////////////////////////////////////////////////////////////
// キーボード入力クラス
////////////////////////////////////////////////////////////////
// コンストラクタ
CMyKeyboard::CMyKeyboard()
{
    nCurrentIndex = 0;
    memset(mk,0, sizeof(mk));
}

////////////////////////////////////////////////////////////////
// 現在のキー入力状態をセット
void CMyKeyboard::SetMyKeyState()
{
    mk[nCurrentIndex].nLeft    = GetAsyncKeyState(VK_NUMPAD4);
    mk[nCurrentIndex].nRight   = GetAsyncKeyState(VK_NUMPAD6);
    mk[nCurrentIndex].nUp      = GetAsyncKeyState(VK_NUMPAD8);
    mk[nCurrentIndex].nDown    = GetAsyncKeyState(VK_NUMPAD2);
    mk[nCurrentIndex].nButton1 = GetAsyncKeyState(VK_Z);
    mk[nCurrentIndex].nButton2 = GetAsyncKeyState(VK_X);
}

////////////////////////////////////////////////////////////////
// キー入力状態チェック
BOOL CMyKeyboard::CheckMyKeyState(int nVKey, int nMode)
{
    SHORT nCurKey = 0, nOldKey = 0;

    // 指定のキー状態を取得
    switch(nVKey)
    {
        case VK_NUMPAD4:
            nCurKey = mk[nCurrentIndex].nLeft;
            nOldKey = mk[1 - nCurrentIndex].nLeft;
            break;

        case VK_NUMPAD6:
            nCurKey = mk[nCurrentIndex].nRight;
            nOldKey = mk[1 - nCurrentIndex].nRight;
            break;

        case VK_NUMPAD8:
            nCurKey = mk[nCurrentIndex].nUp;
            nOldKey = mk[1 - nCurrentIndex].nUp;
            break;

        case VK_NUMPAD2:
            nCurKey = mk[nCurrentIndex].nDown;
            nOldKey = mk[1 - nCurrentIndex].nDown;
            break;

        case VK_Z:
            nCurKey = mk[nCurrentIndex].nButton1;
            nOldKey = mk[1 - nCurrentIndex].nButton1;
            break;

        case VK_X:
            nCurKey = mk[nCurrentIndex].nButton2;
            nOldKey = mk[1 - nCurrentIndex].nButton2;
            break;

        default:
            return FALSE;
    }

    // 押下状態チェック
    switch(nMode)
    {
        // 現在の押下状態をチェック
        case MYKEYSTATE_DOWN:
            return (nCurKey & 0x8000);

        // 押下状態の継続をチェック
        case MYKEYSTATE_HOLD:
            return ((nCurKey & 0x8000) && (nOldKey & 0x8000));

        // 押下状態解除をチェック
        case MYKEYSTATE_BREAK:
            return (!(nCurKey & 0x8000) && (nOldKey & 0x8000));

        // 押下状態への遷移をチェック
        case MYKEYSTATE_PRESS:
            return ((nCurKey & 0x8000) && !(nOldKey & 0x8000));
    }

    return FALSE;
}


////////////////////////////////////////////////////////////////
// マウス入力クラス
////////////////////////////////////////////////////////////////
// コンストラクタ
CMyMouse::CMyMouse()
{
    nCurrentIndex = 0;
    memset(ms, 0, sizeof(ms));
}

////////////////////////////////////////////////////////////////
// 現在のマウス状態をセット
void CMyMouse::SetMouseState()
{
    // カーソル座標をセット
    GetCursorPos(&ms[nCurrentIndex].ptCursor);

    // ボタンの状態をセット
    ms[nCurrentIndex].nLeft  = GetAsyncKeyState(VK_LBUTTON);
    ms[nCurrentIndex].nRight = GetAsyncKeyState(VK_RBUTTON);
}

////////////////////////////////////////////////////////////////
// マウスカーソルの座標を取得
void CMyMouse::GetMouseCoord(int& nCoordX, int& nCoordY)
{
    nCoordX = ms[nCurrentIndex].ptCursor.x;
    nCoordY = ms[nCurrentIndex].ptCursor.y;
}

////////////////////////////////////////////////////////////////
// ボタン押下状態チェック
BOOL CMyMouse::CheckButtonState(int nVKey, int nMode)
{
    SHORT nCurKey = 0, nOldKey = 0;

    // 指定のキー状態を取得
    switch(nVKey)
    {
        case VK_LBUTTON:
            nCurKey = ms[nCurrentIndex].nLeft;
            nOldKey = ms[1 - nCurrentIndex].nLeft;
            break;

        case VK_RBUTTON:
            nCurKey = ms[nCurrentIndex].nRight;
            nOldKey = ms[1 - nCurrentIndex].nRight;
            break;
    }

    // 押下状態チェック
    switch(nMode)
    {
        // 現在の押下状態をチェック
        case MYKEYSTATE_DOWN:
            return (nCurKey & 0x8000);

        // 押下状態の継続をチェック
        case MYKEYSTATE_HOLD:
            return ((nCurKey & 0x8000) && (nOldKey & 0x8000));

        // 押下状態からの解放をチェック
        case MYKEYSTATE_BREAK:
            return (!(nCurKey & 0x8000) && (nOldKey & 0x8000));

        // 押下状態への遷移をチェック
        case MYKEYSTATE_PRESS:
            return ((nCurKey & 0x8000) && !(nOldKey & 0x8000));
    }

    return FALSE;
}
