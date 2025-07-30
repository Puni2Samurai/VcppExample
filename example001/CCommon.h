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
    void ClearElement(){ while(head){ DeleteElement(head); }}  // すべての要素を削除

    // 要素の取得
    CMyList<T> *GetHeadPtr(){ return head; }   // 先頭の要素を取得
    CMyList<T> *GetTailPtr(){ return tail; }   // 末尾の要素を取得
    int GetNumOfElement(){ return nElement; }  // 要素数を取得

    // 先頭に要素を追加
    void AddHead(CMyList<T> *ptr)
    {
        // 要素が存在する場合
        if(head != NULL)
        {
            head->prev = ptr;
            ptr->next = head;
        }
        // 要素が存在しない場合
        else
        {
            tail = ptr;
            ptr->next = NULL;
        }

        head = ptr;
        head->prev = NULL;
        nElement++;
    }

    // 末尾に要素を追加
    void AddTail(CMyList<T> *ptr)
    {
        // 要素が存在する場合
        if(tail != NULL)
        {
            tail->next = ptr;
            ptr->prev = tail;
        }
        // 要素が存在しない場合
        else
        {
            head = ptr;
            ptr->prev = NULL;
        }

        tail = ptr;
        tail->next = NULL;
        nElement++;
    }

    // 指定の要素を削除
    CMyList<T> *DeleteElement(CMyList<T> *ptr, BOOL bNext=TRUE)
    {
        CMyList<T> *p;
        if(bNext == TRUE)
        {
            p = ptr->next;
        }
        else
        {
            p = ptr->prev;
        }

        // 削除対象が先頭要素の場合
        if(ptr == head)
        {
            head = ptr->next;
        }
        else
        {
            ptr->prev->next = ptr->next;
        }

        // 削除対象が末尾要素の場合
        if(ptr == tail)
        {
            tail = ptr->prev;
        }
        else
        {
            ptr->next->prev = ptr->prev;
        }

        delete ptr;
        nElement--;

        return p;
    }
};

