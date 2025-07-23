#include <windows.h>


////////////////////////////////////////////////////////////////
// アプリ共通クラス
////////////////////////////////////////////////////////////////
// エラーダイアログ
class CCommonErrorDialog
{
public:
    virtual ~CCommonErrorDialog() = default;
    virtual BOOL DispErrorMsg(LPCWSTR lpszCaption, LPCWSTR lpszMessage) = 0;
};

////////////////////////////////////////////////////////////////
// テンプレートクラス
////////////////////////////////////////////////////////////////
// リスト
template <class T>
class CMyList
{
public:
    T data;
    CMyList<T> *next;  // 次の要素
    CMyList<T> *prev;  // 前の要素
};

template <class T>
class CMyListBase
{
    int nElement;      // 要素数
    CMyList<T> *head;  // 先頭の要素
    CMyList<T> *tail;  // 末尾の要素

public:
    CMyListBase() : nElement(0), head(NULL), tail(NULL){}
    ~CMyListBase(){ ClearElement(); }

    // 要素の追加/削除
    void AddHead(CMyList<T> *ptr);  // 先頭に要素を追加
    void AddTail(CMyList<T> *ptr);  // 末尾に要素を追加
    CMyList<T> *DeleteElement(CMyList<T> *ptr);  // 指定の要素を削除
    void ClearElement(){ while(head){ DeleteElement(head); }}  // すべての要素を削除

    // 要素の取得
    CMyList<T> *GetHeadPtr(){ return head; }   // 先頭の要素を取得
    CMyList<T> *GetTailPtr(){ return tail; }   // 末尾の要素を取得
    int GetNumOfElement(){ return nElement; }  // 要素数を取得
};

