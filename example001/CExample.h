#include <windows.h>
#include "CDib.h"
#include "CMyInput.h"
#include "CExampleGame.h"


////////////////////////////////////////////////////////////////
// 定数
////////////////////////////////////////////////////////////////
#define GAMESCREEN_WIDTH  400
#define GAMESCREEN_HEIGHT 600
#define GAMESCREEN_FRATE   30

// 処理タスク
enum MYMAINTASK
{
    // 起動時ダイアログ
    TASK_STARTUP,  // 起動
    TASK_INIT,     // 初期化

    // タイトルメニュー
    TASK_TITLEINIT,  // タイトル初期化
    TASK_TITLEMENU,  // タイトルメニュー画面
    TASK_TITLEEXIT,  // タイトル終了

    // ゲームメイン
    TASK_GAMEINIT,  // ゲーム初期化
    TASK_GAMEWAIT,  // ゲーム開始待ち
    TASK_GAMEMAIN,  // ゲームメインルーチン
    TASK_GAMEOVER,  // ゲームオーバー
    TASK_GAMEEXIT,  // ゲーム終了

    // アプリ状態
    TASK_SUSPEND,  // 一時停止
    TASK_QUIT      // 終了
};

////////////////////////////////////////////////////////////////
// 構造体定義
////////////////////////////////////////////////////////////////
// 動作環境
typedef struct tag_myexample_environment
{
    int nWidth;  // ウィンドウ幅
    int nHeight; // ウィンドウ高さ
    int nDepth;  // ビット深度
    BOOL bFullScreen;  // フルスクリーンON/OFF
    int nKindBGM;  // BGM種別
    BOOL bSE;      // 効果音ON/OFF
    WORD wInput;   // ユーザー入力デバイス
} MYENVIRONMENT;

// 選択肢
typedef struct tag_select_cursor
{
    int nWidth;   // 幅
    int nHeight;  // 高さ
    int nPosX;  // 表示座標(X)
    int nPosY;  // 表示座標(Y)
    int nValue;  // 固有値
    int nState;  // カーソルON/OFF(0:OFF, 1:ON)
    BYTE byAlpha;  // アルファブレンド
    COLORREF rgbText[2];  // 文字色(ON/OFF)
    COLORREF rgbBack[2];  // 背景色(ON/OFF)
    LPCWSTR lpszText; // 表示文字
} SELECTCURSOR;

////////////////////////////////////////////////////////////////
// アプリ固有クラス
////////////////////////////////////////////////////////////////
// 基本クラス
class CMyExampleBase : public CCommonErrorDialog
{
protected:
    // ウィンドウ情報
    static HWND hwndExample;
    static HINSTANCE hExampleInst;

    // アプリ情報
    static MYENVIRONMENT myenv;

    // 入力デバイス
    static CMyKeyInput myKeyInput;

public:
    // 描画領域
    static HDC hMemOffscreen;
    static HBITMAP hbmOffscreen;

    BOOL DispErrorMsg(LPCWSTR lpszCaption, LPCWSTR lpszMessage);  // エラーダイアログ表示
    MYENVIRONMENT GetMyEnvironment(){ return myenv; }
};

////////////////////////////////////////////////////////////////
// タイトル画面クラス
class CMyExampleTitle : public CMyExampleBase
{
private:
    WORD wFPS;
    WCHAR szFPS[8];
    DWORD dwFPSTickCount;

    // タイトルメニュー
    SPRITE spriteCursor;  // カーソル
    SELECTCURSOR selcurMenu[2];  // 選択メニュー
    MYMAINTASK taskNext;  // 遷移先タスク

    // 画像
    CDib24* pBG;         // 背景
    CDib24* pLogo;       // タイトルロゴ
    CDib24* pSprite;     // スプライト
    CDib24* pOffscreen;  // オフスクリーン

    // フォント
    HFONT hfont16;
    HFONT hfont32;
    HFONT hfont64;

    // 入力デバイス
    //CMyKeyInput myKeyInput;

public:
    CMyExampleTitle();
    ~CMyExampleTitle(){ FreeTitle(); }

    MYMAINTASK GetNextTask(){ return taskNext; }

    // 初期化
    BOOL InitTitle();
    void FreeTitle();
    void FreeCDib(CDib24* ptr);
    void EraseOffscreen();
    void InitOffscreen();

    // フレーム
    void UpdateFrame(DWORD dwTickCount, BOOL bDrawSkip);
    void CheckKeyInput();
    void DrawBackground();
    void DrawSprite(DWORD dwTickCount);
    void DrawChar(HFONT& hfont, int x, int y, LPCWSTR lpszText, COLORREF rgbText);
    void DrawChar(HFONT& hfont, int x, int y, LPCWSTR lpszText, COLORREF rgbText, COLORREF rgbBorder);
};

////////////////////////////////////////////////////////////////
// ゲーム画面クラス
class CMyExampleGame : public CMyExampleBase
{
private:
    WORD wFPS;
    WCHAR szFPS[8];
    DWORD dwFPSTickCount;

    DWORD dwScore;
    WCHAR szGameStatus[10];
    DWORD dwWaveCount;
    DWORD dwWaveInterval;
    DWORD dwItemInterval;
    DWORD dwBulletInterval;

    // スプライト
    MYPLAYERCHARACTER spritePlayChar;  // 操作キャラ
    CMyListBase<MYBULLET> listBullet;  // 自弾
    CMyListBase<MYENEMY>  listEnemy;   // 敵キャラ

    // タスク遷移
    MYMAINTASK taskNext;  // 遷移先タスク
    SPRITE spriteCursor;  // カーソル
    SELECTCURSOR selcurMenu[2];  // 選択メニュー
    DWORD dwGameLastTickCount;

    // 画像
    CDib24* pBG;         // 背景
    CDib24* pSprite;     // スプライト
    CDib24* pOffscreen;  // オフスクリーン

    // フォント
    HFONT hfont16;
    HFONT hfont24;
    HFONT hfont32;
    HFONT hfont64;

    // 入力デバイス
    //CMyKeyInput myKeyInput;

public:
    CMyExampleGame();
    ~CMyExampleGame(){ FreeGame(); }

    // タスク
    MYMAINTASK GetNextTask(){ return taskNext; }
    DWORD GetGameLastTickCount(){ return dwGameLastTickCount; }

    // 初期化
    BOOL InitGame();
    void FreeGame();
    void FreeCDib(CDib24* ptr);
    void EraseOffscreen();
    void InitOffscreen();
    void ResetGameData();

    // フレーム
    void UpdateFrame_Init(DWORD dwTickCount, BOOL bDrawSkip);
    void UpdateFrame_Main(DWORD dwTickCount, BOOL bDrawSkip);
    void UpdateFrame_Exit(DWORD dwTickCount, BOOL bDrawSkip);
    void CheckKeyInput();
    void CheckKeyInput_Exit();
    void DrawBackground();
    void DrawSprite(DWORD dwTickCount);
    void DrawSprite_Exit();
    void DrawChar(HFONT& hfont, int x, int y, LPCWSTR lpszText, COLORREF rgbText, COLORREF rgbBorder);
    void SetScoreStr();

    // ゲームデータ更新
    void CreateBullet(DWORD dwTickCount);
    void UpdateBullet(DWORD dwTickCount);
    void CreateEnemy(DWORD dwTickCount);
    void UpdateEnemy(DWORD dwTickCount);
};

////////////////////////////////////////////////////////////////
// メインクラス
class CMyExample : public CMyExampleBase
{
private:
    DWORD dwLastTickCount;
    DWORD dwElapsed;
    BOOL  bDrawSkip;

    // アプリ状態
    int nWinMode;
    BOOL bIsActive;
    static MYMAINTASK nMainTask;

    // 画面
    CMyExampleTitle* pMyTitle;
    CMyExampleGame*  pMyGame;

public:
    CMyExample();
    ~CMyExample();

    // ウィンドウ
    void SetExampleWindowValue(HINSTANCE hInst, int nShowCmd){ hExampleInst = hInst; nWinMode = nShowCmd; }
    BOOL CreateMyExampleWindow();  // アプリ用ウィンドウ生成

    // 処理タスク
    void Scheduler();
    void FlipScreen(HDC hDC);

    // ダイアログ用コールバック関数
    static BOOL CALLBACK StartupDialogFunc(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK SetEnvDialogFunc(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam);
};

