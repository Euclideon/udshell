#pragma once
#if !defined(_EPFREELIST_HPP)
#define _EPFREELIST_HPP

namespace ep {

template<typename T>
class FreeList
{
public:
  FreeList(size_t listSize)
    : numAllocated(0)
  {
    pList = pFreeList = epAllocType(T, listSize, epAF_None);
    for (size_t i = 0; i<listSize; ++i)
      (T*&)pList[i] = i < listSize-1 ? &pList[i+1] : nullptr;
  }
  ~FreeList()
  {
    // TODO: free allocated items...

    epFree(pList);
  }

  template<typename... Args>
  T* Alloc(Args... args)
  {
    T *pNew = pFreeList;
    if (!pNew)
      return nullptr;

    pFreeList = *(T**)pFreeList;
    ++numAllocated;

    return new(pNew)T(args...);
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

} // namespace ep

#endif // _EPFREELIST_HPP
