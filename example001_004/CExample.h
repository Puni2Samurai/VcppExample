#include "CDib.h"

////////////////////////////////////////////////////////////////
// アプリ固有クラス
////////////////////////////////////////////////////////////////
class CMyExample : public CCommon
{
protected:
    // ウィンドウ情報
    static HWND hwnd;
    static HINSTANCE hThisInst;
    int nWinMode;
    int nWindowWidth;
    int nWindowHeight;

    // DIB
    CDib24 *pDibSprite;
    CDib24 *pDibBackground;
    CDib24 *pDibOffScreen;

public:
    CMyExample() : pDibSprite(NULL), pDibBackground(NULL){}

    BOOL DispErrorMsg(LPCWSTR lpszCaption, LPCWSTR lpszMessage);

    BOOL CreateExampleWindow(HINSTANCE hAppInst, int nShowCmd, int nWidth, int nHeight);
    BOOL InitExample();
    void DrawOffScreen();
    void UpdateScreen(HDC hDC);
};

