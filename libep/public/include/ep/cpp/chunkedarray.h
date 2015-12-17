#ifndef _EP_CHUNKEDARRAY_HPP
#define _EP_CHUNKEDARRAY_HPP

namespace ep {

// TODO: this needs C++11-ification

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
struct ChunkedArray
{
  ChunkedArray();
  ~ChunkedArray();

  epResult Clear();

  T &operator[](size_t index);
  T *GetElement(size_t index);
  void SetElement(size_t index, const T &data);

  T *PushBack();
  T *PushFront();
  epResult GrowBack(size_t numberOfNewElements);

  bool PopBack(T *pData = nullptr);
  bool PopFront(T *pData = nullptr);

  // Remove the element at index, swapping with the last element to ensure array is contiguous
  void RemoveSwapLast(size_t index);
  void RemoveSwapLast(T *pEl) { RemoveSwapLast(GetElementIndex(pEl)); }

  size_t ChunkElementCount()      { return chunkElementCount; }
  size_t ElementSize()              { return sizeof(T); }

  size_t GetElementIndex(T *pEl);

  struct chunk_t
  {
    T data[chunkElementCount];
  };

  enum { ptrArrayInc = 32 };

  chunk_t **ppChunks;
  size_t ptrArraySize;
  size_t chunkCount;

  size_t length;
  size_t inset;

  epResult AddChunks(size_t numberOfNewChunks);
};

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline ChunkedArray<T, chunkElementCount>::ChunkedArray()
{
  EP_STATICASSERT(chunkElementCount >= 32, _Chunk_Count_Must_Be_At_Least_32);

  size_t c = 0;

  chunkCount = 1;
  length = 0;
  inset = 0;

  if (chunkCount > ptrArrayInc)
    ptrArraySize = ((chunkCount + ptrArrayInc - 1) / ptrArrayInc) * ptrArrayInc;
  else
    ptrArraySize = ptrArrayInc;

  ppChunks = epAllocType(chunk_t*, ptrArraySize, epAF_Zero);
  if (!ppChunks)
    EPTHROW(epR_MemoryAllocationFailure, "Alloc failed");

  for (; c < chunkCount; ++c)
  {
    ppChunks[c] = epAllocType(chunk_t, 1, epAF_None);
    if (!ppChunks[c])
      EPTHROW(epR_MemoryAllocationFailure, "Alloc failure");
  }
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline ChunkedArray<T, chunkElementCount>::~ChunkedArray()
{
  for (size_t c = 0; c < chunkCount; ++c)
    epFree(ppChunks[c]);

  epFree(ppChunks);

  chunkCount = 0;
  length = 0;
  inset = 0;
}

// --------------------------------------------------------------------------
// Author: Paul Fox, June 2015
template <typename T, size_t chunkElementCount>
inline epResult ChunkedArray<T, chunkElementCount>::Clear()
{
  length = 0;
  inset = 0;

  return epR_Success;
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline epResult ChunkedArray<T, chunkElementCount>::AddChunks(size_t numberOfNewChunks)
{
  size_t newChunkCount = chunkCount + numberOfNewChunks;

  if (newChunkCount > ptrArraySize)
  {
    size_t newPtrArraySize = ((newChunkCount + ptrArrayInc - 1) / ptrArrayInc) * ptrArrayInc;
    chunk_t **newppChunks = epAllocType(chunk_t*, newPtrArraySize, epAF_Zero);
    if (!newppChunks)
      return epR_MemoryAllocationFailure;

    memcpy(newppChunks, ppChunks, ptrArraySize * sizeof(chunk_t*));
    epFree(ppChunks);

    ppChunks = newppChunks;
    ptrArraySize = newPtrArraySize;
  }

  for (size_t c = chunkCount; c < newChunkCount; ++c)
  {
    ppChunks[c] = epAllocType(chunk_t, 1, epAF_None);
    if (!ppChunks[c])
    {
      chunkCount = c;
      return epR_MemoryAllocationFailure;
    }
  }

  chunkCount = newChunkCount;
  return epR_Success;
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline epResult ChunkedArray<T, chunkElementCount>::GrowBack(size_t numberOfNewElements)
{
  if (numberOfNewElements == 0)
    return epR_InvalidParameter_;

  size_t oldLength = inset + length;
  size_t newLength = oldLength + numberOfNewElements;
  size_t prevUsedChunkCount = (oldLength + chunkElementCount - 1) / chunkElementCount;

  const size_t capacity = chunkElementCount * chunkCount;
  if (newLength > capacity)
  {
    size_t requiredEntries = newLength - capacity;
    size_t numberOfNewChunksToAdd = (requiredEntries + chunkElementCount - 1) / chunkElementCount;

    epResult result = AddChunks(numberOfNewChunksToAdd);
    if (result != epR_Success)
      return result;
  }

  // Zero new elements
  size_t newUsedChunkCount = (newLength + chunkElementCount - 1) / chunkElementCount;
  size_t usedChunkDelta = newUsedChunkCount - prevUsedChunkCount;
  size_t head = oldLength % chunkElementCount;

  if (usedChunkDelta)
  {
    if (head)
      memset(&ppChunks[prevUsedChunkCount - 1]->data[head], 0, (chunkElementCount - head) * sizeof(T));

    size_t tail = newLength % chunkElementCount;

    for (size_t chunkIndex = prevUsedChunkCount; chunkIndex < (newUsedChunkCount - 1 + !tail); ++chunkIndex)
      memset(ppChunks[chunkIndex]->data, 0, sizeof(chunk_t));

    if (tail)
      memset(&ppChunks[newUsedChunkCount - 1]->data[0], 0, tail * sizeof(T));
  }
  else
  {
    memset(&ppChunks[prevUsedChunkCount - 1]->data[head], 0, numberOfNewElements * sizeof(T));
  }

  length += numberOfNewElements;

  return epR_Success;
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline T &ChunkedArray<T, chunkElementCount>::operator[](size_t index)
{
  EPASSERT(index < length, "Index out of bounds");
  index += inset;
  size_t chunkIndex = index / chunkElementCount;
  return ppChunks[chunkIndex]->data[index % chunkElementCount];
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline T* ChunkedArray<T, chunkElementCount>::GetElement(size_t  index)
{
  EPASSERT(index < length, "Index out of bounds");
  index += inset;
  size_t chunkIndex = index / chunkElementCount;
  return &ppChunks[chunkIndex]->data[index % chunkElementCount];
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline void ChunkedArray<T, chunkElementCount>::SetElement(size_t index, const T &data)
{
  EPASSERT(index < length, "Index out of bounds");
  index += inset;
  size_t chunkIndex = index / chunkElementCount;
  ppChunks[chunkIndex]->data[index % chunkElementCount] = data;
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline T *ChunkedArray<T, chunkElementCount>::PushBack()
{
  size_t newIndex = inset + length;
  if (newIndex >= (chunkElementCount * chunkCount))
    AddChunks(1);

  size_t chunkIndex = size_t(newIndex / uint64_t(chunkElementCount));

  T *pElement = &ppChunks[chunkIndex]->data[newIndex % chunkElementCount];
  memset(pElement, 0, sizeof(T));

  ++length;
  return pElement;
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline T *ChunkedArray<T, chunkElementCount>::PushFront()
{
  if (inset)
  {
    --inset;
  }
  else
  {
    if (length)
    {
      // Are we out of pointers
      if ((chunkCount + 1) > ptrArraySize)
      {
        chunk_t **ppNewChunks = epAllocType(chunk_t*, (ptrArraySize + ptrArrayInc), epAF_Zero);
        if (!ppNewChunks)
          return nullptr;

        ptrArraySize += ptrArrayInc;
        memcpy(ppNewChunks + 1, ppChunks, chunkCount * sizeof(chunk_t*));

        epFree(ppChunks);
        ppChunks = ppNewChunks;
      }
      else
      {
        memmove(ppChunks + 1, ppChunks, chunkCount * sizeof(chunk_t*));
      }

      // See if we have an unused chunk at the end of the array
      if (((chunkCount * chunkElementCount) - length) > chunkElementCount)
      {
        ppChunks[0] = ppChunks[chunkCount - 1];
        ppChunks[chunkCount - 1] = nullptr;
      }
      else
      {
        chunk_t *pNewBlock = epAllocType(chunk_t, 1, epAF_None);
        if (!pNewBlock)
        {
          memmove(ppChunks, ppChunks + 1, chunkCount * sizeof(chunk_t*));
          return nullptr;
        }
        ppChunks[0] = pNewBlock;
        ++chunkCount;
      }
    }
    inset = chunkElementCount - 1;
  }

  ++length;

  T *pElement = ppChunks[0]->data + inset;
  memset(pElement, 0, sizeof(T));

  return pElement;
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline bool ChunkedArray<T, chunkElementCount>::PopBack(T *pDest)
{
  if (length)
  {
    if (pDest)
      *pDest = *GetElement(length - 1);
    --length;

    if (length == 0)
      inset = 0;
    return true;
  }
  return false;
}

// --------------------------------------------------------------------------
// Author: David Ely, May 2015
template <typename T, size_t chunkElementCount>
inline bool ChunkedArray<T, chunkElementCount>::PopFront(T *pDest)
{
  if (length)
  {
    if (pDest)
      *pDest = *GetElement(inset);
    ++inset;
    if (inset == chunkElementCount)
    {
      inset = 0;
      if (chunkCount > 1)
      {
        chunk_t *pHead = ppChunks[0];
        memmove(ppChunks, ppChunks + 1, (chunkCount - 1) * sizeof(chunk_t*));
        ppChunks[chunkCount - 1] = pHead;
      }
    }

    --length;

    if (length == 0)
      inset = 0;
    return true;
  }
  return false;
}


// --------------------------------------------------------------------------
// Author: Dave Pevreal, May 2015
template <typename T, size_t chunkElementCount>
inline void ChunkedArray<T, chunkElementCount>::RemoveSwapLast(size_t index)
{
  // Only copy the last element over if the element being removed isn't the last element
  if (index != (length - 1))
    SetElement(index, *GetElement(length - 1));
  PopBack();
}

// --------------------------------------------------------------------------
// Author: Manu Evans, Sep 2015
template <typename T, size_t chunkElementCount>
inline size_t ChunkedArray<T, chunkElementCount>::GetElementIndex(T *pEl)
{
  for (size_t i = 0; i < chunkCount; ++i)
  {
    if (pEl >= &ppChunks[i]->data[0] && pEl < &ppChunks[i]->data[chunkElementCount])
      return (i*chunkElementCount + (pEl - &ppChunks[i]->data[0])) - inset;
  }
  EPASSERT(false, "Not in array?!");
  return 0;
}

} // namespace ep

#endif // _EP_CHUNKEDARRAY_HPP
