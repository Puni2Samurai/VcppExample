#include "CCommon.h"

////////////////////////////////////////////////////////////////
// DIB(24bit)クラス
////////////////////////////////////////////////////////////////
class CDib24
{
protected:
    CCommonErrorDialog *perrDialog;
    LPBITMAPINFO lpbmi;
    LPBYTE lpBits;       // イメージビット
    DWORD  dwTransparentColor;  // 透明色

public:
    CDib24(CCommonErrorDialog *p) : lpBits(NULL), lpbmi(NULL), perrDialog(p){ dwTransparentColor = 0; }
    ~CDib24(){ FreeCDib(); }

    // メモリ
    void FreeCDib();
    BOOL AllocBMInfoMem();
    BOOL AllocBitsMem(UINT uImageSize);

    // ビットマップ情報
    BOOL ReadBMPFile(LPCWSTR lpszFilename);
    BOOL CreateDibObject(LONG lWidth, LONG lHeight);
    int GetCDibWidth();
    int GetCDibHeight();

    // 描画
    DWORD GetTransparentColor(){ return dwTransparentColor; }
    void SetTransparentColor(DWORD dwColor){ dwTransparentColor = dwColor; }
    void SetTransparentColor(int nCoordX, int nCoordY);
    void CopyDibBits(const CDib24& dibSrc);
    void CopyDibBits(const CDib24& dibSrc, RECT rcSrc, RECT rcDst, BOOL bTransparent = TRUE, BYTE byAlpha = 255);
    void DrawRectangle(RECT rcDst, COLORREF color, BYTE byAlpha);
    void DrawBits(HDC hDC, int nCoordX, int nCoordY);
};

