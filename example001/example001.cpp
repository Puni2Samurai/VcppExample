#include <windows.h>
#include "CExample.h"

LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);

////////////////////////////////////////////////////////////////
HWND          CMyExampleBase::hwndExample;
HINSTANCE     CMyExampleBase::hExampleInst;
HDC           CMyExampleBase::hMemOffscreen;
HBITMAP       CMyExampleBase::hbmOffscreen;
MYENVIRONMENT CMyExampleBase::myenv;
MYMAINTASK    CMyExample::nMainTask;

CMyExample myex;
HGDIOBJ hOld = NULL;


////////////////////////////////////////////////////////////////
// エントリー関数
////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
    myex.SetExampleWindowValue(hThisInst, nWinMode);

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
            myex.Scheduler();
        }
    }

    return msg.wParam;
}


////////////////////////////////////////////////////////////////
// ウィンドウ関数
////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    switch(message)
    {
        // ウィンドウ生成時
        case WM_CREATE:
            int nWidth, nHeight;
            nWidth  = myex.GetMyEnvironment().nWidth;
            nHeight = myex.GetMyEnvironment().nHeight;

            hDC = GetDC(hwnd);
            myex.hbmOffscreen = CreateCompatibleBitmap(hDC, nWidth, nHeight);
            myex.hMemOffscreen = CreateCompatibleDC(hDC);
            hOld = SelectObject(myex.hMemOffscreen, myex.hbmOffscreen);
            PatBlt(myex.hMemOffscreen, 0, 0, nWidth, nHeight, BLACKNESS);
            ReleaseDC(hwnd, hDC);
            break;

        // 再描画
        case WM_PAINT:
            hDC = BeginPaint(hwnd, &ps);
            myex.FlipScreen(hDC);
            EndPaint(hwnd, &ps);
            break;

        // キーボード押下
        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;
            }
            break;

        // プログラム終了
        case WM_DESTROY:
            SelectObject(myex.hMemOffscreen, hOld);
            DeleteDC(myex.hMemOffscreen);
            DeleteObject(myex.hbmOffscreen);
            PostQuitMessage(0);
            break;

        // その他メッセージ処理
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

