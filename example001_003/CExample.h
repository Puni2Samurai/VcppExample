#include "CDib.h"

////////////////////////////////////////////////////////////////
// アプリ固有クラス
////////////////////////////////////////////////////////////////
class CMyExample
{
protected:
    HWND hwnd;
	HINSTANCE hThisInst;
	int nWinMode;

	CDib24 *pDibSprite;

public:
	BOOL CreateExampleWindow(HINSTANCE hAppInst, int nShowCmd);
	BOOL InitExample();
	void UpdateScreen(HDC hDC);
};

