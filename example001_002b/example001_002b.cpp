#include <windows.h>
#include "example001_002b.h"

LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);

////////////////////////////////////////////////////////////////
// エントリー関数
////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
    HWND hwnd;
    HMENU hMenu;
    MSG msg;
    WNDCLASSEXA wcl;
    const char szWinName[] = "ExampleWin";
    const char szWinTitle[] = "Window Title: Example";

    // ウィンドウクラス定義
    wcl.hInstance = hThisInst;
    wcl.lpszClassName = szWinName;
    wcl.lpfnWndProc = WindowFunc;
    wcl.style = 0;
    wcl.cbSize = sizeof(WNDCLASSEXA);
    wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcl.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcl.lpszMenuName = "MENU_EXAMPLE001_002";
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if(!RegisterClassExA(&wcl))
    {
        return 0;
    }

    // ウィンドウ生成
    hwnd = CreateWindowA(
        szWinName,
        szWinTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        640, 480,
        HWND_DESKTOP,
        NULL,
        hThisInst,
        NULL
    );

    ShowWindow(hwnd, nWinMode);
    UpdateWindow(hwnd);

    // メッセージループ
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
    switch(message)
    {
        // メニュー項目選択
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDM_EXIT:
                    MessageBoxA(hwnd, "Quit this app.", "EXIT", MB_OK);
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

