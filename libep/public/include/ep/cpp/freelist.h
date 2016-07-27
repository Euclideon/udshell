#pragma once
#if !defined(_EPFREELIST_HPP)
#define _EPFREELIST_HPP

namespace ep {

template<typename T>
struct FreeList
{
public:
  FreeList(size_t listSize) :
    blockSize(listSize){}

  FreeList(FreeList &&rval) :
    pBlockList(rval.pBlockList), pFreeList(rval.pFreeList), numAllocated(rval.numAllocated), blockSize(rval.blockSize)
  {
    rval.numAllocated = rval.blockSize = 0;
    rval.pBlockList = nullptr;
    rval.pFreeList = nullptr;
  }

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
  T* create(Args&&... args)
  {
    T *pMem = _alloc();
    epscope(fail) { _free(pMem); };
    return epConstruct(pMem) T(std::forward<Args>(args)...);
  }

  T* _alloc()
  {
    T *pNew = pFreeList;
    if (!pNew)
    {
      T *pNewBlock = (T*)epAllocFlags(blockSize * sizeof(T) + sizeof(void*), epAF_None);
      EPTHROW_IF_NULL(pNewBlock, Result::AllocFailure, "Memory allocation failed");
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

  void destroy(T *pItem)
  {
    pItem->~T();
    _free(pItem);
  }

  void _free(T *pItem)
  {
    *(T**)pItem = pFreeList;
    pFreeList = pItem;
    --numAllocated;
  }

  size_t size() const { return numAllocated; }

private:

  void *pBlockList = nullptr;
  T *pFreeList = nullptr;
  size_t numAllocated = 0;
  size_t blockSize;
};

} // namespace ep

#endif // _EPFREELIST_HPP
