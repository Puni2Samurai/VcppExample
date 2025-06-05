#include <windows.h>
#include "CExample.h"

LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);

CMyExample myex;


////////////////////////////////////////////////////////////////
// エントリー関数
////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
    // アプリ初期化
    if(myex.CreateExampleWindow(hThisInst, nWinMode) == FALSE)
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

        // キーボード押下
        case WM_KEYDOWN:
            switch(wParam)
            {
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

