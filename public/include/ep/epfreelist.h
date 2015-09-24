#pragma once
#if !defined(_EPFREELIST_H)
#define _EPFREELIST_H

template<typename T>
class epFreeList
{
public:
  epFreeList(size_t listSize)
    : numAllocated(0)
  {
    pList = pFreeList = udAllocType(T, listSize, udAF_None);
    for (size_t i = 0; i<listSize; ++i)
      (T*&)pList[i] = i < listSize-1 ? &pList[i+1] : nullptr;
  }
  ~epFreeList()
  {
    // TODO: free allocated items...

    udFree(pList);
  }

  template<typename... Args>
  T* Alloc(Args... args)
  {
    T *pNew = pFreeList;
    if (!pNew)
      return nullptr;

    pFreeList = *(T**)pFreeList;
    ++numAllocated;

    return new(pNew) T(args...);
  }

  void Free(T *pItem)
  {
    pItem->~T();
    *(T**)pItem = pFreeList;
    pFreeList = pItem;
  }

  size_t Size() const { return numAllocated; }

private:
  T *pList, *pFreeList;
  size_t numAllocated;
};

#endif // _EPFREELIST_H
