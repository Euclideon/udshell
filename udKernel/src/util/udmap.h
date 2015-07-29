#pragma once
#if !defined(_UDMAP_H)
#define _UDMAP_H

#include "udfreelist.h"

#include <functional>


template<typename T>
class udMap
{
public:
  typedef ptrdiff_t (Pred)(const T&, const T&);

  udMap(size_t maxItems, Pred *pPred)
    : pool(maxItems), pPred(pPred)
  {}

  T* Insert(T &&item)
  {
    T *pItem = pool.Alloc(std::move(item));
    AddToTree(pHead, pItem);
    return pItem;
  }
  T* Insert(const T &item)
  {
    T *pItem = pool.Alloc(item);
    AddToTree(pHead, pItem);
    return pItem;
  }

  void Remove(T *pItem)
  {
    // TODO: ...
  }

  T* Get(udString id)
  {
    T *pN = pHead;
    while (pN)
    {
      ptrdiff_t d = pN->id.eq(id);
      if (d == 0)
        return pN;
      if (d < 0)
        pN = pN->pLeft;
      else
        pN = pN->pRight;
    }
    return nullptr;
  }

  size_t Size() const
  {
    return pool.Size();
  }

private:
  udFreeList<T> pool;
  T *pHead = nullptr;
  Pred *pPred;

  void AddToTree(T* &pNode, T *pItem)
  {
    if (!pNode)
      pNode = pItem;
    else if (pPred(*pNode, *pItem) < 0)
      AddToTree(pNode->pRight, pItem);
    else
      AddToTree(pNode->pLeft, pItem);
  }
};

#endif // _UDMAP_H
