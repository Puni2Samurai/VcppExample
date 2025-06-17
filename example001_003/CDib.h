////////////////////////////////////////////////////////////////
// DIB(24bit)クラス
////////////////////////////////////////////////////////////////
class CDib24
{
protected:
    HWND hwnd;
    LPBITMAPINFO lpbmi;
    LPBYTE lpBits;       // イメージビット

public:
    CDib24(HWND& hThisInst) : lpBits(NULL), lpbmi(NULL){ hwnd = hThisInst; }
    ~CDib24(){ FreeCDib(); }

    // メモリ
    void FreeCDib();
    BOOL AllocBMInfoMem();
    BOOL AllocBitsMem(UINT uImageSize);

    // ビットマップ情報
    BOOL ReadBMPFile(LPCSTR lpszFilename);

    // 描画
    void DrawBits(HDC hDC, int nCoordX, int nCoordY);
};

