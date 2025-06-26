#include "CCommon.h"

////////////////////////////////////////////////////////////////
// DIB(24bit)クラス
////////////////////////////////////////////////////////////////
class CDib24
{
protected:
    CCommon *pcommon;
    LPBITMAPINFO lpbmi;
    LPBYTE lpBits;       // イメージビット
    DWORD  dwTransparentColor;  // 透明色

public:
    CDib24(CCommon *p) : lpBits(NULL), lpbmi(NULL), pcommon(p){ dwTransparentColor = 0; }
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
    void DrawBits(HDC hDC, int nCoordX, int nCoordY);
};

