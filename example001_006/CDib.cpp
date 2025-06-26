#include <windows.h>
#include <fstream>
#include <iostream>
#include "CDib.h"

using namespace std;


////////////////////////////////////////////////////////////////
// DIB基本クラス
////////////////////////////////////////////////////////////////
// メモリ解放
void CDib24::FreeCDib()
{
    if(lpbmi != NULL)
    {
        delete lpbmi;
        lpbmi = NULL;
    }

    if(lpBits != NULL)
    {
        delete [] lpBits;
        lpBits = NULL;
    }
}

////////////////////////////////////////////////////////////////
// BITMAPINFO構造体のメモリ割り当て
BOOL CDib24::AllocBMInfoMem()
{
    // 既に割り当て済みの場合は解放
    if(lpbmi != NULL)
    {
        delete lpbmi;
        lpbmi = NULL;
    }

    // メモリ割り当て
    lpbmi = (LPBITMAPINFO) new BITMAPINFOHEADER;
    if(lpbmi == NULL)
    {
        return FALSE;
    }

    memset(lpbmi, 0, sizeof(BITMAPINFOHEADER));

    return TRUE;
}

////////////////////////////////////////////////////////////////
// イメージビットのメモリ割り当て
BOOL CDib24::AllocBitsMem(UINT uImageSize)
{
    // 既に割り当て済みの場合は解放
    if(lpBits != NULL)
    {
        delete lpBits;
        lpBits = NULL;
    }

    // メモリ割り当て
    lpBits = new BYTE[uImageSize];
    if(lpBits == NULL)
    {
        return FALSE;
    }

    memset(lpBits, 0, uImageSize);

    return TRUE;
}

////////////////////////////////////////////////////////////////
// BMPファイルの読み込み
BOOL CDib24::ReadBMPFile(LPCWSTR lpszFilename)
{
    // ファイルオープン
    ifstream fin(lpszFilename, ios::in | ios::binary);
    if(fin.is_open() == FALSE)
    {
        return pcommon->DispErrorMsg(L"ファイル読込", L"ファイルオープンに失敗");
    }

    // BITMAPFILEHEADER構造体の読み込み
    BITMAPFILEHEADER bmfh;
    fin.read((char *)&bmfh, sizeof(BITMAPFILEHEADER));
    if(fin.gcount() != sizeof(BITMAPFILEHEADER))
    {
        return pcommon->DispErrorMsg(L"ファイル読込", L"BITMAPFILEHEADER構造体の読み込みに失敗");
    }

    // ファイルがBMPであるか判定
    if(bmfh.bfType != 0x4d42)
    {
        return pcommon->DispErrorMsg(L"ファイル読込", L"BMP形式ではないファイルを指定");
    }

    // BITMAPINFO構造体のメモリ割り当て
    if(AllocBMInfoMem() == FALSE)
    {
        return pcommon->DispErrorMsg(L"ファイル読込", L"BITMAPINFO構造体のメモリ割り当てに失敗");
    }

    // BITMAPINFOHEADER構造体の読み込み
    fin.read((char *)lpbmi, sizeof(BITMAPINFOHEADER));
    if(fin.gcount() != sizeof(BITMAPINFOHEADER))
    {
        return pcommon->DispErrorMsg(L"ファイル読込", L"BITMAPINFOHEADER構造体の読み込みに失敗");
    }

    // イメージが24bitカラーであるか判定
    if(lpbmi->bmiHeader.biBitCount != 24)
    {
        return pcommon->DispErrorMsg(L"ファイル読込", L"24bitカラーではないBMPファイルを指定");
    }

    // イメージビットのメモリ割り当て
    UINT uImageSize = bmfh.bfSize - bmfh.bfOffBits;
    if(AllocBitsMem(uImageSize) == FALSE)
    {
        return pcommon->DispErrorMsg(L"ファイル読込", L"イメージビットのメモリ割り当てに失敗");
    }

    // イメージビットの読み込み
    fin.read((char *)lpBits, uImageSize);
    if(fin.gcount() != uImageSize)
    {
        return pcommon->DispErrorMsg(L"ファイル読込", L"イメージビットの読み込みに失敗");
    }

    fin.close();

    return TRUE;
}

////////////////////////////////////////////////////////////////
// DIB生成
BOOL CDib24::CreateDibObject(LONG lWidth, LONG lHeight)
{
    // BITMAPINFO構造体のメモリ割り当て
    if(AllocBMInfoMem() == FALSE)
    {
        return pcommon->DispErrorMsg(L"DIB生成", L"BITMAPINFO構造体のメモリ割り当てに失敗");
    }

    // BITMAPINFO構造体の初期化
    if(lpbmi != NULL)
    {
        lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        lpbmi->bmiHeader.biWidth = lWidth;
        lpbmi->bmiHeader.biHeight = lHeight;
        lpbmi->bmiHeader.biPlanes = 1;
        lpbmi->bmiHeader.biBitCount = 24;
        lpbmi->bmiHeader.biCompression = BI_RGB;
    }

    // イメージビットのメモリ割り当て
    UINT uImageSize = ((lWidth * 3 + 3) & ~3) * lHeight;
    if(AllocBitsMem(uImageSize) == FALSE)
    {
        return pcommon->DispErrorMsg(L"DIB生成", L"イメージビットのメモリ割り当てに失敗");
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////
// イメージの幅を取得
int CDib24::GetCDibWidth()
{
    if(lpbmi != NULL)
    {
        return (int)(lpbmi->bmiHeader.biWidth);
    }
    else
    {
        return 0;
    }
}

////////////////////////////////////////////////////////////////
// イメージの高さを取得
int CDib24::GetCDibHeight()
{
    if(lpbmi != NULL)
    {
        return (int)(lpbmi->bmiHeader.biHeight);
    }
    else
    {
        return 0;
    }
}

////////////////////////////////////////////////////////////////
// 指定座標のカラー値を透明色としてセット
void CDib24::SetTransparentColor(int nCoordX, int nCoordY)
{
    BYTE byRed, byGreen, byBlue;
    int nByteX = nCoordX * 3;
    int nByteY = GetCDibHeight() - nCoordY - 1;
    int nScanLine = (GetCDibWidth() * 3 + 3) & ~3;

    byRed   = lpBits[nByteX + 2 + nScanLine * nByteY];
    byGreen = lpBits[nByteX + 1 + nScanLine * nByteY];
    byBlue  = lpBits[nByteX     + nScanLine * nByteY];

    dwTransparentColor = byRed | (byGreen << 8) | (byBlue << 16);
}

////////////////////////////////////////////////////////////////
// イメージ全体のコピー
void CDib24::CopyDibBits(const CDib24& dibSrc)
{
    // コピー元とコピー先のサイズを比較して、小さい方をコピーサイズとする
    int nCopyWidth, nCopyHeight;

    if(GetCDibWidth() < (int)dibSrc.lpbmi->bmiHeader.biWidth)
    {
        nCopyWidth = GetCDibWidth() * 3;
    }
    else
    {
        nCopyWidth = (int)dibSrc.lpbmi->bmiHeader.biWidth * 3;
    }

    if(GetCDibHeight() < (int)dibSrc.lpbmi->bmiHeader.biHeight)
    {
        nCopyHeight = GetCDibHeight();
    }
    else
    {
        nCopyHeight = (int)dibSrc.lpbmi->bmiHeader.biHeight;
    }

    // イメージビットのコピー
    int i, j;
    int nSrcLine, nDstLine;
    LPBYTE lpSrcBits, lpDstBits;

    nSrcLine = (dibSrc.lpbmi->bmiHeader.biWidth * 3 + 3) & ~3;
    nDstLine = (GetCDibWidth() * 3 + 3) & ~3;

    lpSrcBits = dibSrc.lpBits;
    lpDstBits = lpBits;

    for(i = 0; i < nCopyHeight; i++)
    {
        for(j = 0; j < nCopyWidth; j += 3)
        {
            lpDstBits[j]     = lpSrcBits[j];
            lpDstBits[j + 1] = lpSrcBits[j + 1];
            lpDstBits[j + 2] = lpSrcBits[j + 2];
        }

        lpSrcBits += nSrcLine;
        lpDstBits += nDstLine;
    }
}

////////////////////////////////////////////////////////////////
// スプライトのコピー
void CDib24::CopyDibBits(const CDib24& dibSrc, RECT rcSrc, RECT rcDst, BOOL bTransparent, BYTE byAlpha)
{
    // コピー元とコピー先のサイズを比較して、小さい方をコピーサイズとする
    int nCopyWidth, nCopyHeight;

    if((rcDst.right - rcDst.left) < (rcSrc.right - rcSrc.left))
    {
        nCopyWidth = (rcDst.right - rcDst.left) * 3;
    }
    else
    {
        nCopyWidth = (rcSrc.right - rcSrc.left) * 3;
    }

    if((rcDst.bottom - rcDst.top) < (rcSrc.bottom - rcSrc.top))
    {
        nCopyHeight = rcDst.bottom - rcDst.top;
    }
    else
    {
        nCopyHeight = rcSrc.bottom - rcSrc.top;
    }

    // 透明色をRGBに分解
    BYTE byRed, byGreen, byBlue;
    byRed   = (BYTE)( dwTransparentColor & 0x000000FF);
    byGreen = (BYTE)((dwTransparentColor & 0x0000FF00) >> 8);
    byBlue  = (BYTE)((dwTransparentColor & 0x00FF0000) >> 16);

    // イメージビットのコピー
    int i, j;
    int nSrcLine, nDstLine;
    int nSrcHeight, nDstHeight;
    int nSrcIndex, nDstIndex;
    LPBYTE lpSrcBits, lpDstBits;
    BYTE bySrcColor, byDstColor, byCopyColor;

    nSrcLine = (dibSrc.lpbmi->bmiHeader.biWidth * 3 + 3) & ~3;
    nDstLine = (GetCDibWidth() * 3 + 3) & ~3;

    nSrcHeight = dibSrc.lpbmi->bmiHeader.biHeight - rcSrc.top - 1;
    nDstHeight = GetCDibHeight() - rcDst.top - 1;

    lpSrcBits = dibSrc.lpBits;
    lpDstBits = lpBits;

    for(i = 0; i < nCopyHeight; i++)
    {
        for(j = 0; j < nCopyWidth; j += 3)
        {
            nSrcIndex = (rcSrc.left * 3) + j + (nSrcHeight - i) * nSrcLine;
            nDstIndex = (rcDst.left * 3) + j + (nDstHeight - i) * nDstLine;

            // 透明色を使用しない、または、透明色ではない場合はビットコピー
            if((bTransparent == FALSE) 
            || (byRed != lpSrcBits[nSrcIndex + 2]) || (byGreen != lpSrcBits[nSrcIndex + 1]) || (byBlue != lpSrcBits[nSrcIndex]))
            {
                // 透過なし
                if(byAlpha == 255)
                {
                    lpDstBits[nDstIndex]     = lpSrcBits[nSrcIndex];
                    lpDstBits[nDstIndex + 1] = lpSrcBits[nSrcIndex + 1];
                    lpDstBits[nDstIndex + 2] = lpSrcBits[nSrcIndex + 2];
                }
                // 透過あり
                else
                {
                    bySrcColor = lpSrcBits[nSrcIndex];
                    byDstColor = lpDstBits[nDstIndex];
                    byCopyColor = ((bySrcColor * byAlpha) + (byDstColor * (255 - byAlpha))) / 255;
                    lpDstBits[nDstIndex] = byCopyColor;
                    bySrcColor = lpSrcBits[nSrcIndex + 1];
                    byDstColor = lpDstBits[nDstIndex + 1];
                    byCopyColor = ((bySrcColor * byAlpha) + (byDstColor * (255 - byAlpha))) / 255;
                    lpDstBits[nDstIndex + 1] = byCopyColor;
                    bySrcColor = lpSrcBits[nSrcIndex + 2];
                    byDstColor = lpDstBits[nDstIndex + 2];
                    byCopyColor = ((bySrcColor * byAlpha) + (byDstColor * (255 - byAlpha))) / 255;
                    lpDstBits[nDstIndex + 2] = byCopyColor;
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////
// イメージの描画
void CDib24::DrawBits(HDC hDC, int nCoordX, int nCoordY)
{
    StretchDIBits(
        hDC,                        // 描画先のデバイス
        nCoordX, nCoordY,           // 描画先の座標(X, Y)
        lpbmi->bmiHeader.biWidth,   // 描画サイズ(幅)
        lpbmi->bmiHeader.biHeight,  // 描画サイズ(高さ)
        0, 0,                       // 描画元の座標(X, Y)
        lpbmi->bmiHeader.biWidth,   // 描画元サイズ(幅)
        lpbmi->bmiHeader.biHeight,  // 描画元サイズ(高さ)
        lpBits,                     // イメージビット
        lpbmi,                      // 当イメージのBIMAPINFO構造体
        DIB_RGB_COLORS,             // RGB値イメージビット
        SRCCOPY                     // 描画元の矩形をコピー
    );
}

