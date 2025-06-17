#include <windows.h>
#include <fstream>
#include <iostream>
#include "CDib.h"

using namespace std;


////////////////////////////////////////////////////////////////
// エラーダイアログ表示
BOOL DispErrorDialog(HWND hwnd, LPCWSTR lpszCaption, LPCWSTR lpszMessage)
{
    MessageBoxW(hwnd, lpszMessage, lpszCaption, MB_OK);
    return FALSE;
}

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

    return TRUE;
}

////////////////////////////////////////////////////////////////
// BMPファイルの読み込み
BOOL CDib24::ReadBMPFile(LPCSTR lpszFilename)
{
    // ファイルオープン
    ifstream fin(lpszFilename, ios::in | ios::binary);
    if(fin.is_open() == FALSE)
    {
        return DispErrorDialog(hwnd, L"ファイル読込", L"ファイルオープンに失敗");
    }

    // BITMAPFILEHEADER構造体の読み込み
    BITMAPFILEHEADER bmfh;
    fin.read((char *)&bmfh, sizeof(BITMAPFILEHEADER));
    if(fin.gcount() != sizeof(BITMAPFILEHEADER))
    {
        return DispErrorDialog(hwnd, L"ファイル読込", L"BITMAPFILEHEADER構造体の読み込みに失敗");
    }

    // ファイルがBMPであるか判定
    if(bmfh.bfType != 0x4d42)
    {
        return DispErrorDialog(hwnd, L"ファイル読込", L"BMP形式ではないファイルを指定");
    }

    // BITMAPINFO構造体のメモリ割り当て
    if(AllocBMInfoMem() == FALSE)
    {
        return DispErrorDialog(hwnd, L"ファイル読込", L"BITMAPINFO構造体のメモリ割り当てに失敗");
    }

    // BITMAPINFOHEADER構造体の読み込み
    fin.read((char *)lpbmi, sizeof(BITMAPINFOHEADER));
    if(fin.gcount() != sizeof(BITMAPINFOHEADER))
    {
        return DispErrorDialog(hwnd, L"ファイル読込", L"BITMAPINFOHEADER構造体の読み込みに失敗");
    }

    // イメージが24bitカラーであるか判定
    if(lpbmi->bmiHeader.biBitCount != 24)
    {
        return DispErrorDialog(hwnd, L"ファイル読込", L"24bitカラーではないBMPファイルを指定");
    }

    // イメージビットのメモリ割り当て
    UINT uImageSize = bmfh.bfSize - bmfh.bfOffBits;
    if(AllocBitsMem(uImageSize) == FALSE)
    {
        return DispErrorDialog(hwnd, L"ファイル読込", L"イメージビットのメモリ割り当てに失敗");
    }

    // イメージビットの読み込み
    fin.read((char *)lpBits, uImageSize);
    if(fin.gcount() != uImageSize)
    {
        return DispErrorDialog(hwnd, L"ファイル読込", L"イメージビットの読み込みに失敗");
    }

    fin.close();

    return TRUE;
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

