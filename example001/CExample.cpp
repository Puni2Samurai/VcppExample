#include <windows.h>
#include "CExample.h"
#include "example001.h"

extern LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);


////////////////////////////////////////////////////////////////
// 基本クラス
////////////////////////////////////////////////////////////////
// エラーダイアログ表示
BOOL CMyExampleBase::DispErrorMsg(LPCWSTR lpszCaption, LPCWSTR lpszMessage)
{
    MessageBoxW(hwndExample, lpszMessage, lpszCaption, MB_OK);
    return FALSE;
}

////////////////////////////////////////////////////////////////
// メインクラス
////////////////////////////////////////////////////////////////
// コンストラクタ
CMyExample::CMyExample()
{
    bDrawSkip = FALSE;

    // ウィンドウ
    hwndExample = NULL;
    hbmOffscreen = NULL;
    hMemOffscreen = NULL;

    // タスク
    nMainTask = MYMAINTASK::TASK_STARTUP;
    bIsActive = FALSE;

    // 動作環境
    myenv.bFullScreen = FALSE;
    myenv.nWidth = 800;
    myenv.nHeight = 600;
    myenv.nDepth = 24;
    myenv.nKindBGM = 0;
    myenv.bSE = FALSE;
    myenv.wInput = 0;

    // 画面
    pMyTitle = NULL;
    pMyGame  = NULL;
}

////////////////////////////////////////////////////////////////
// デストラクタ
CMyExample::~CMyExample()
{
    if(pMyTitle != NULL)
    {
        delete pMyTitle;
        pMyTitle = NULL;
    }

    if(pMyGame != NULL)
    {
        delete pMyGame;
        pMyGame = NULL;
    }
}

////////////////////////////////////////////////////////////////
// ウィンドウ生成
BOOL CMyExample::CreateMyExampleWindow()
{
    WNDCLASSEXA wcl;
    const char szWinName[] = "ExampleWin";
    const char szWinTitle[] = "My Example Game";

    // ウィンドウクラス登録
    wcl.hInstance = hExampleInst;
    wcl.lpszClassName = szWinName;
    wcl.lpfnWndProc = WindowFunc;
    wcl.style = 0;
    wcl.cbSize = sizeof(WNDCLASSEX);
    wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcl.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcl.lpszMenuName = NULL;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if(!RegisterClassExA(&wcl))
    {
        return DispErrorMsg(L"初期化エラー", L"ウィンドウクラスの登録に失敗");
    }

    // ウィンドウ表示位置を算出
    int x, y, nAdjustWidth, nAdjustHeight;
    DWORD dwStyle;
    RECT rcAppWin, rcDisplay;

    x = y = 0;
    nAdjustWidth = myenv.nWidth;
    nAdjustHeight = myenv.nHeight;
    dwStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE | WS_OVERLAPPED;
    rcAppWin.left = 0;
    rcAppWin.top = 0;
    rcAppWin.right = nAdjustWidth;
    rcAppWin.bottom = nAdjustHeight;
    if(AdjustWindowRectEx(&rcAppWin, dwStyle, FALSE, 0) != FALSE)
    {
        nAdjustWidth  = rcAppWin.right - rcAppWin.left;
        nAdjustHeight = rcAppWin.bottom - rcAppWin.top;
        if(SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDisplay, 0) != FALSE)
        {
            x = rcDisplay.left + ((rcDisplay.right - rcDisplay.left) - nAdjustWidth)  / 2;
            y = rcDisplay.top  + ((rcDisplay.bottom - rcDisplay.top) - nAdjustHeight) / 2;
        }
    }

    // ウィンドウ生成
    hwndExample = CreateWindowA(
        szWinName,
        szWinTitle,
        dwStyle,
        x, y,
        nAdjustWidth, nAdjustHeight,
        HWND_DESKTOP,
        NULL,
        hExampleInst,
        NULL
    );

    ShowWindow(hwndExample, nWinMode);
    UpdateWindow(hwndExample);
    SetFocus(hwndExample);

    dwLastTickCount = dwElapsed = GetTickCount();

    return TRUE;
}

////////////////////////////////////////////////////////////////
// 処理タスク
void CMyExample::Scheduler()
{
    dwElapsed = GetTickCount();

    switch(nMainTask)
    {
        ////////////////////////////////////////////////////////
        // 起動時ダイアログ画面
        ////////////////////////////////////////////////////////
        // アプリ起動
        case MYMAINTASK::TASK_STARTUP:
            nMainTask = MYMAINTASK::TASK_INIT;
            DialogBoxW(hExampleInst, MAKEINTRESOURCEW(IDD_MYEXAMPLE_STARTUP), NULL, (DLGPROC)CMyExample::StartupDialogFunc);
            break;

        // 初期化
        case MYMAINTASK::TASK_INIT:
            if(CreateMyExampleWindow() == FALSE)
            {
                nMainTask = MYMAINTASK::TASK_QUIT;
            }
            nMainTask = MYMAINTASK::TASK_TITLEINIT;
            break;

        ////////////////////////////////////////////////////////
        // タイトル画面
        ////////////////////////////////////////////////////////
        // 初期化
        case MYMAINTASK::TASK_TITLEINIT:
            pMyTitle = new CMyExampleTitle;
            if(pMyTitle == NULL)
            {
                nMainTask = MYMAINTASK::TASK_QUIT;
                break;
            }

            if(pMyTitle->InitTitle() == FALSE)
            {
                nMainTask = MYMAINTASK::TASK_QUIT;
                break;
            }

            pMyTitle->InitOffscreen();
            dwLastTickCount = dwElapsed;
            nMainTask = MYMAINTASK::TASK_TITLEMENU;

        // メニュー選択
        case MYMAINTASK::TASK_TITLEMENU:
            if(dwElapsed - dwLastTickCount >= GAMESCREEN_FRATE)
            {
                pMyTitle->UpdateFrame(dwLastTickCount, bDrawSkip);
                dwLastTickCount = dwElapsed - (dwElapsed - dwLastTickCount - GAMESCREEN_FRATE);
            }

            nMainTask = pMyTitle->GetNextTask();
            if(nMainTask == MYMAINTASK::TASK_TITLEMENU)
            {
                break;
            }

        // 終了
        case MYMAINTASK::TASK_TITLEEXIT:
            if(pMyTitle != NULL)
            {
                nMainTask = pMyTitle->GetNextTask();
                delete pMyTitle;
                pMyTitle = NULL;
            }
            break;

        ////////////////////////////////////////////////////////
        // ゲーム画面
        ////////////////////////////////////////////////////////
        // 初期化
        case MYMAINTASK::TASK_GAMEINIT:
            pMyGame = new CMyExampleGame;
            if(pMyGame == NULL)
            {
                nMainTask = MYMAINTASK::TASK_QUIT;
                break;
            }

            if(pMyGame->InitGame() == FALSE)
            {
                nMainTask = MYMAINTASK::TASK_QUIT;
                break;
            }

            pMyGame->InitOffscreen();
            InvalidateRect(hwndExample, NULL, TRUE);
            dwLastTickCount = dwElapsed;
            nMainTask = MYMAINTASK::TASK_GAMEWAIT;

        // 開始待ち
        case MYMAINTASK::TASK_GAMEWAIT:
            // フレーム更新周期
            if(dwElapsed - dwLastTickCount >= GAMESCREEN_FRATE)
            {
                pMyGame->UpdateFrame_Init(dwLastTickCount, bDrawSkip);
                dwLastTickCount = dwElapsed - (dwElapsed - dwLastTickCount - GAMESCREEN_FRATE);
            }

            // 1秒経過で状態遷移
            if(dwElapsed - pMyGame->GetGameLastTickCount() >= 1000)
            {
                pMyGame->ResetGameData();
                nMainTask = pMyGame->GetNextTask();
            }
            break;

        // ゲームプレイ
        case MYMAINTASK::TASK_GAMEMAIN:
            // フレーム更新周期
            if(dwElapsed - dwLastTickCount >= GAMESCREEN_FRATE)
            {
                pMyGame->UpdateFrame_Main(dwLastTickCount, bDrawSkip);
                dwLastTickCount = dwElapsed - (dwElapsed - dwLastTickCount - GAMESCREEN_FRATE);
            }

            nMainTask = pMyGame->GetNextTask();
            break;

        // ゲームオーバー画面
        case MYMAINTASK::TASK_GAMEOVER:
            // フレーム更新周期
            if(dwElapsed - dwLastTickCount >= GAMESCREEN_FRATE)
            {
                pMyGame->UpdateFrame_Exit(dwLastTickCount, bDrawSkip);
                dwLastTickCount = dwElapsed - (dwElapsed - dwLastTickCount - GAMESCREEN_FRATE);
            }

            nMainTask = pMyGame->GetNextTask();
            break;

        // ゲーム画面破棄
        case MYMAINTASK::TASK_GAMEEXIT:
            nMainTask = MYMAINTASK::TASK_TITLEINIT;
            if(pMyGame != NULL)
            {
                delete pMyGame;
                pMyGame = NULL;
            }
            break;

        ////////////////////////////////////////////////////////
        // アプリ状態
        ////////////////////////////////////////////////////////
        // 一時停止
        case MYMAINTASK::TASK_SUSPEND:
            break;

        // アプリ終了
        case MYMAINTASK::TASK_QUIT:
            // ウィンドウ生成後ならWM_DESTROY
            if(hwndExample != NULL)
            {
                DestroyWindow(hwndExample);
            }
            // ウィンドウ生成前ならWM_QUIT
            else
            {
                PostQuitMessage(0);
            }

            break;

        default:
            break;
    }

    // フレーム更新に処理が間に合わない場合は、描画処理スキップフラグを立てる
    if(GetTickCount() - dwElapsed > GAMESCREEN_FRATE)
    {
        bDrawSkip = TRUE;
    }
    else
    {
        bDrawSkip = FALSE;
    }
}

////////////////////////////////////////////////////////////////
// 表示更新
void CMyExample::FlipScreen(HDC hDC)
{
    BitBlt(hDC, 0, 0, myenv.nWidth, myenv.nHeight, hMemOffscreen, 0, 0, SRCCOPY);
}

////////////////////////////////////////////////////////////////
// 起動時ダイアログ
BOOL CALLBACK CMyExample::StartupDialogFunc(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        // ダイアログのボタン押下
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                // 開始
                case IDC_BUTTON_START:
                    EndDialog(hwndDialog, 0);
                    return TRUE;

                // 環境設定
                case IDC_BUTTON_SETENV:
                    DialogBoxW(hExampleInst, MAKEINTRESOURCEW(IDD_MYEXAMPLE_SETENV), hwndDialog, (DLGPROC)CMyExample::SetEnvDialogFunc);
                    return TRUE;

                // アプリ終了
                case IDC_BUTTON_QUIT:
                    EndDialog(hwndDialog, 0);
                    nMainTask = MYMAINTASK::TASK_QUIT;
                    return TRUE;
            }
            break;
    }

    return FALSE;
}

////////////////////////////////////////////////////////////////
// 起動時ダイアログ
BOOL CALLBACK CMyExample::SetEnvDialogFunc(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        // ダイアログオープン時
        case WM_INITDIALOG:
            // 初期値設定(コンボボックス)
            SendDlgItemMessageW(hwndDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)L" 800 *  600  24bpp");
            SendDlgItemMessageW(hwndDialog, IDC_COMBO1, CB_SETCURSEL, 0, 0);

            // 初期値設定(ラジオボタン)
            SendDlgItemMessageW(hwndDialog, IDC_RADIO_SCREEN1, BM_SETCHECK, 1, 0);
            SendDlgItemMessageW(hwndDialog, IDC_RADIO_BGM_OFF, BM_SETCHECK, 1, 0);
            SendDlgItemMessageW(hwndDialog, IDC_RADIO_SE_OFF,  BM_SETCHECK, 1, 0);
            return TRUE;

        // ダイアログのボタン押下
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                // 設定適用
                case IDC_BUTTON_APPLY:
                    // 画面モード
                    if(SendDlgItemMessageW(hwndDialog, IDC_RADIO_SCREEN1, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    {
                        myenv.bFullScreen = FALSE;
                    }

                    // BGM種別
                    if(SendDlgItemMessageW(hwndDialog, IDC_RADIO_BGM_OFF, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    {
                        myenv.nKindBGM = 0;
                    }

                    // 効果音
                    if(SendDlgItemMessageW(hwndDialog, IDC_RADIO_SE_OFF, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    {
                        myenv.bSE = FALSE;
                    }

                    return TRUE;

                // 設定終了
                case IDC_BUTTON_EXIT:
                    EndDialog(hwndDialog, 0);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}

