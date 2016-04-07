#pragma once
#if !defined(_EPFREELIST_HPP)
#define _EPFREELIST_HPP

namespace ep {

template<typename T>
class FreeList
{
public:
  FreeList(size_t listSize) :
    blockSize(listSize){}

  ~FreeList()
  {
    // TODO: Consider for each item searching the free list and if it doesn't exist calling its destructor.
    EPASSERT(numAllocated == 0, "numAllocated must be zero as ~FreeList() doesn't call the destructors");
    while (pBlockList)
    {
      void *pBlock = pBlockList;
      pBlockList = *(void**)((T*)pBlockList + blockSize);
      epFree(pBlock);
    }
  }

  template<typename... Args>
  T* New(Args... args)
  {
    T *pMem = Alloc();
    return epConstruct(pMem) T(args...);
  }

  T* Alloc()
  {
    T *pNew = pFreeList;
    if (!pNew)
    {
      T *pNewBlock = (T*)epAllocFlags(blockSize * sizeof(T) + sizeof(void*), epAF_None);
      EPTHROW_IF_NULL(pNewBlock, epR_AllocFailure, "Memory allocation failed");
      *(void**)(pNewBlock + blockSize) = pBlockList;
      pBlockList = pNewBlock;

      T *pList = pFreeList = pNewBlock;
      for (size_t i = 0; i<blockSize; ++i)
        (T*&)pList[i] = i < blockSize - 1 ? &pList[i + 1] : nullptr;

      pNew = pFreeList;
    }

    pFreeList = *(T**)pFreeList;
    ++numAllocated;

    return pNew;
  }

  void Delete(T *pItem)
  {
    pItem->~T();
    Free(pItem);
  }

  void Free(T *pItem)
  {
    *(T**)pItem = pFreeList;
    pFreeList = pItem;
    --numAllocated;
  }

  size_t Size() const { return numAllocated; }

private:

  void *pBlockList = nullptr;
  T *pFreeList = nullptr;
  size_t numAllocated = 0;
  size_t blockSize;
};

} // namespace ep

#endif // _EPFREELIST_HPP
