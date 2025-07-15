#include "CCommon.h"

////////////////////////////////////////////////////////////////
// テンプレートクラス
////////////////////////////////////////////////////////////////
// 先頭に要素を追加
template <class T>
void CMyListBase<T>::AddHead(CMyList<T> *ptr)
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

////////////////////////////////////////////////////////////////
// 末尾に要素を追加
template <class T>
void CMyListBase<T>::AddTail(CMyList<T> *ptr)
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

////////////////////////////////////////////////////////////////
// 指定の要素を削除
template <class T>
CMyList<T> *CMyListBase<T>::DeleteElement(CMyList<T> *ptr)
{
    CMyList<T> *p;
    p = ptr->next;

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

