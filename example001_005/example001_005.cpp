#include <windows.h>
#include "CExample.h"

LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);

HWND CMyExample::hwnd;
HINSTANCE CMyExample::hThisInst;
CMyExample myex;

////////////////////////////////////////////////////////////////
// エントリー関数
////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
    // アプリ初期化
    if(myex.CreateExampleWindow(hThisInst, nWinMode, 640, 480) == FALSE)
    {
        return 0;
    }

    if(myex.InitExample() == FALSE)
    {
        return 0;
    }

    // メッセージループ
    MSG msg;

    while(TRUE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if(!GetMessage(&msg, NULL, 0, 0))
            {
                return msg.wParam;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // ここにアプリ固有の処理を記述
            myex.UpdateFrame();
        }
    }

    return msg.wParam;
}


////////////////////////////////////////////////////////////////
// ウィンドウコールバック関数
////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    switch(message)
    {
        // 再描画
        case WM_PAINT:
            hDC = BeginPaint(hwnd, &ps);
            myex.UpdateScreen(hDC);
            EndPaint(hwnd, &ps);
            break;

        // マウスボタン押下
        case WM_LBUTTONDOWN:
            myex.SetSpriteCoord(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_RBUTTONDOWN:
            myex.ChangeSpriteState();
            break;

        // キーボード押下
        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_NUMPAD2:
                    myex.MoveSpriteCoord(0, 8);
                    break;

                case VK_NUMPAD4:
                    myex.MoveSpriteCoord(-8, 0);
                    break;

                case VK_NUMPAD5:
                    myex.ChangeSpriteState();
                    break;

                case VK_NUMPAD6:
                    myex.MoveSpriteCoord(8, 0);
                    break;

                case VK_NUMPAD8:
                    myex.MoveSpriteCoord(0, -8);
                    break;

                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
            }
            break;

        // プログラム終了
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        // その他メッセージ処理
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

