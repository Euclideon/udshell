#include <type_traits>
#include <utility>

// epSlice
template<typename T>
inline epSlice<T>::epSlice()
  : length(0), ptr(nullptr)
{}

template<typename T>
inline epSlice<T>::epSlice(nullptr_t)
  : length(0), ptr(nullptr)
{}

template<typename T>
inline epSlice<T>::epSlice(std::initializer_list<ET> list)
  : length(list.size()) , ptr(list.begin())
{}

template<typename T>
inline epSlice<T>::epSlice(T* ptr, size_t length)
  : length(length), ptr(ptr)
{}

template<typename T>
template<typename U>
inline epSlice<T>::epSlice(epSlice<U> rh)
  : length(rh.length), ptr(rh.ptr)
{}

template<typename T>
template<typename U>
inline epSlice<T>& epSlice<T>::operator =(epSlice<U> rh)
{
  length = rh.length;
  ptr = rh.ptr;
  return *this;
}

template<typename T>
inline typename epSlice<T>::ET& epSlice<T>::operator[](ptrdiff_t i) const
{
  size_t offset = (size_t)(i < 0 ? i + length : i);
  EPASSERT(offset < length, "Index out of range!");
  return ((ET*)ptr)[offset];
}

template<typename T>
inline epSlice<T> epSlice<T>::slice(ptrdiff_t first, ptrdiff_t last) const
{
  size_t start = (size_t)(first < 0 ? first + length : first);
  size_t end = (size_t)(last < 0 ? last + length : last);
  EPASSERT(end <= length && start <= end, "Index out of range!");
  return epSlice<T>((ET*)ptr + start, end - start);
}

template<typename T>
inline bool epSlice<T>::empty() const
{
  return length == 0;
}

template<typename T>
inline bool epSlice<T>::operator ==(epSlice<const T> rh) const
{
  return ptr == rh.ptr && length == rh.length;
}

template<typename T>
inline bool epSlice<T>::operator !=(epSlice<const T> rh) const
{
  return ptr != rh.ptr || length != rh.length;
}

template<typename T>
template<typename U>
inline bool epSlice<T>::eq(epSlice<U> rh) const
{
  if(length != rh.length)
    return false;
  for(size_t i=0; i<length; ++i)
    if (((ET*)ptr)[i] != ((ET*)rh.ptr)[i])
      return false;
  return true;
}

template<typename T>
template<typename U>
inline ptrdiff_t epSlice<T>::cmp(epSlice<U> rh) const
{
  size_t len = length < rh.length ? length : rh.length;
  for (size_t i = 0; i < len; ++i)
  {
    if (((ET*)ptr)[i] == ((ET*)rh.ptr)[i])
      continue;
    return ((ET*)ptr)[i] < ((ET*)rh.ptr)[i] ? -1 : 1;
  }
  return length - rh.length;
}

template<typename T>
template<typename U>
inline bool epSlice<T>::beginsWith(epSlice<U> rh) const
{
  if (length < rh.length)
    return false;
  return slice(0, rh.length).eq(rh);
}
template<typename T>
template<typename U>
inline bool epSlice<T>::endsWith(epSlice<U> rh) const
{
  if (length < rh.length)
    return false;
  return slice(length - rh.length, length).eq(rh);
}

template<typename T>
inline epIterator<T> epSlice<T>::begin() const
{
  return epIterator<T>(&ptr[0]);
}
template<typename T>
inline epIterator<T> epSlice<T>::end() const
{
  return epIterator<T>(&ptr[length]);
}

template<typename T>
inline typename epSlice<T>::ET& epSlice<T>::front() const
{
  return ((ET*)ptr)[0];
}
template<typename T>
inline typename epSlice<T>::ET& epSlice<T>::back() const
{
  return ((ET*)ptr)[length-1];
}
template<typename T>
inline typename epSlice<T>::ET& epSlice<T>::popFront()
{
  *this = slice(1, length);
  return ((ET*)ptr)[-1];
}
template<typename T>
inline typename epSlice<T>::ET& epSlice<T>::popBack()
{
  *this = slice(0, length - 1);
  return ((ET*)ptr)[length];
}

template<typename T>
inline epSlice<T> epSlice<T>::get(ptrdiff_t n) const
{
  if (n < 0)
    return slice(n, length);
  else
    return slice(0, n);
}
template<typename T>
inline epSlice<T> epSlice<T>::pop(ptrdiff_t n)
{
  if (n < 0)
  {
    *this = slice(0, n);
    return epSlice<T>(ptr - n, -n);
  }
  else
  {
    *this = slice(n, length);
    return epSlice<T>(ptr - n, n);
  }
}
template<typename T>
inline epSlice<T> epSlice<T>::strip(ptrdiff_t n) const
{
  if (n < 0)
    return slice(0, n);
  else
    return slice(n, length);
}

template<typename T>
inline bool epSlice<T>::exists(const typename epSlice<T>::ET &c, size_t *pIndex) const
{
  size_t i = findFirst(c) != -1;
  if (pIndex)
    *pIndex = i;
  return i == length;
}

template<typename T>
inline size_t epSlice<T>::findFirst(const typename epSlice<T>::ET &c) const
{
  size_t offset = 0;
  while (offset < length && ptr[offset] != c)
    ++offset;
  return offset;
}
template<typename T>
inline size_t epSlice<T>::findLast(const typename epSlice<T>::ET &c) const
{
  ptrdiff_t last = length-1;
  while (last >= 0 && ptr[last] != c)
    --last;
  return last < 0 ? length : last;
}
template<typename T>
template<typename U>
inline size_t epSlice<T>::findFirst(epSlice<U> s) const
{
  if (s.empty())
    return 0;
  ptrdiff_t len = length-s.length;
  for (ptrdiff_t i = 0; i < len; ++i)
  {
    size_t j = 0;
    for (; j < s.length; ++j)
    {
      if (ptr[i + j] != s.ptr[j])
        break;
    }
    if (j == s.length)
      return i;
  }
  return length;
}
template<typename T>
template<typename U>
inline size_t epSlice<T>::findLast(epSlice<U> s) const
{
  if (s.empty())
    return length;
  for (ptrdiff_t i = length-s.length; i >= 0; --i)
  {
    size_t j = 0;
    for (; j < s.length; ++j)
    {
      if (ptr[i + j] != s.ptr[j])
        break;
    }
    if (j == s.length)
      return i;
  }
  return length;
}

template<typename T>
inline epSlice<T> epSlice<T>::getLeftAtFirst(const typename epSlice<T>::ET &c, bool bInclusive) const
{
  return slice(0, findFirst(c) + (bInclusive ? 1 : 0));
}
template<typename T>
inline epSlice<T> epSlice<T>::getLeftAtLast(const typename epSlice<T>::ET &c, bool bInclusive) const
{
  return slice(0, findLast(c) + (bInclusive ? 1 : 0));
}
template<typename T>
inline epSlice<T> epSlice<T>::getRightAtFirst(const typename epSlice<T>::ET &c, bool bInclusive) const
{
  return slice(findFirst(c) + (bInclusive ? 0 : 1), length);
}
template<typename T>
inline epSlice<T> epSlice<T>::getRightAtLast(const typename epSlice<T>::ET &c, bool bInclusive) const
{
  return slice(findLast(c) + (bInclusive ? 0 : 1), length);
}

template<typename T>
template<typename U>
inline epSlice<T> epSlice<T>::getLeftAtFirst(epSlice<U> s, bool bInclusive) const
{
  return slice(0, findFirst(s) + (bInclusive ? s.length : 0));
}
template<typename T>
template<typename U>
inline epSlice<T> epSlice<T>::getLeftAtLast(epSlice<U> s, bool bInclusive) const
{
  return slice(0, findLast(s) + (bInclusive ? s.length : 0));
}
template<typename T>
template<typename U>
inline epSlice<T> epSlice<T>::getRightAtFirst(epSlice<U> s, bool bInclusive) const
{
  return slice(findFirst(s) + (bInclusive ? 0 : s.length), length);
}
template<typename T>
template<typename U>
inline epSlice<T> epSlice<T>::getRightAtLast(epSlice<U> s, bool bInclusive) const
{
  return slice(findLast(s) + (bInclusive ? 0 : s.length), length);
}

template<typename T>
inline ptrdiff_t epSlice<T>::indexOfElement(const T *c) const
{
  size_t offset = 0;
  while (offset < length && &ptr[offset] != c)
    ++offset;
  return offset == length ? -1 : offset;
}

template<typename T>
inline T* epSlice<T>::search(Predicate pred) const
{
  for (auto &e : *this)
  {
    if (pred(e))
      return &e;
  }
  return nullptr;
}

template<typename T>
template<bool skipEmptyTokens>
inline epSlice<T> epSlice<T>::popToken(epSlice<T> delimiters)
{
  size_t offset = 0;
  if (skipEmptyTokens)
  {
    while (offset < length && delimiters.exists(ptr[offset]))
      ++offset;
    if (offset == length)
      return epSlice<T>();
  }
  size_t end = offset;
  while (end < length && !delimiters.exists(ptr[end]))
    ++end;
  epSlice<T> token = slice(offset, end);
  if (end < length)
    ++end;
  ptr += end;
  length -= end;
  return token;
}

template<typename T>
template<bool skipEmptyTokens>
inline epSlice<epSlice<T>> epSlice<T>::tokenise(epSlice<epSlice<T>> tokens, epSlice<T> delimiters)
{
  size_t numTokens = 0;
  size_t offset = 0;
  while (offset < length && numTokens < tokens.length)
  {
    if (!skipEmptyTokens)
    {
      size_t tokStart = offset;
      while (offset < length && !delimiters.canFind(ptr[offset]))
        ++offset;
      tokens[numTokens++] = slice(tokStart, offset);
      ++offset;
    }
    else
    {
      while (offset < length && delimiters.canFind(ptr[offset]))
        ++offset;
      if (offset == length)
        break;
      size_t tokStart = offset;
      while (offset < length && !delimiters.canFind(ptr[offset]))
        ++offset;
      tokens[numTokens++] = slice(tokStart, offset);
    }
  }
  if (!skipEmptyTokens && offset > length)
    offset = length;
  ptr += offset;
  length -= offset;
  return tokens.slice(0, numTokens);
}

template<typename T>
template<typename U>
inline void epSlice<T>::copyTo(epSlice<U> dest) const
{
  EPASSERT(length >= dest.length, "Not enough elements!");
  for (size_t i = 0; i<dest.length; ++i)
    dest.ptr[i] = ptr[i];
}


// epArray
template <typename T, size_t Count>
inline epArray<T, Count>::epArray()
{}

template <typename T, size_t Count>
inline epArray<T, Count>::epArray(nullptr_t)
{}

template <typename T, size_t Count>
inline epArray<T, Count>::epArray(std::initializer_list<T> list)
  : epArray(list.begin(), list.size())
{}

template <typename T, size_t Count>
inline epArray<T, Count>::epArray(const epArray<T, Count> &val)
  : epArray(val.ptr, val.length)
{}

template <typename T, size_t Count>
inline epArray<T, Count>::epArray(epArray<T, Count> &&rval)
{
  this->length = rval.length;
  if (rval.hasAllocation())
  {
    // we can copy large buffers efficiently!
    this->ptr = rval.ptr;
    rval.ptr = nullptr;
    rval.length = 0;
  }
  else
  {
    // copy items into the small array buffer
    this->ptr = (T*)buffer.ptr();
    if (std::is_pod<T>::value)
      memcpy((void*)this->ptr, rval.ptr, sizeof(T)*this->length);
    else
    {
      for (size_t i = 0; i < this->length; ++i)
        new((void*)&this->ptr[i]) T(std::move(rval.ptr[i]));
    }
  }
}

template <typename T, size_t Count>
template <typename U>
inline epArray<T, Count>::epArray(U *ptr, size_t length)
  : epSlice<T>()
{
  reserve(length);
  this->length = length;
//  if (std::is_pod<T>::value) // TODO: this is only valid if T and U are the same!
//    memcpy((void*)this->ptr, ptr, sizeof(T)*length);
//  else
  {
    for (size_t i = 0; i < length; ++i)
      new((void*)&this->ptr[i]) T(ptr[i]);
  }
}

template <typename T, size_t Count>
template <typename U>
inline epArray<T, Count>::epArray(epSlice<U> slice)
  : epArray<T, Count>(slice.ptr, slice.length)
{
}

template <typename T, size_t Count>
inline epArray<T, Count>::~epArray()
{
  for (size_t i = 0; i < this->length; ++i)
    this->ptr[i].~T();
  if (hasAllocation())
  {
    Header *pHeader = getHeader();
    udFree(pHeader);
  }
}

template <typename T, size_t Count>
inline void epArray<T, Count>::reserve(size_t count)
{
  bool hasAlloc = hasAllocation();
  if (!hasAlloc && count <= Count)
    this->ptr = buffer.ptr();
  else if (count > Count)
  {
    bool needsExtend = hasAlloc && getHeader()->numAllocated < count;
    if (!hasAlloc || needsExtend)
    {
      Header *pH = (Header*)udAlloc(sizeof(Header) + sizeof(T)*count);
      pH->numAllocated = count;
      T *pNew = (T*)&pH[1];
      if (std::is_pod<T>::value)
        memcpy((void*)pNew, this->ptr, sizeof(T)*this->length);
      else
      {
        for (size_t i = 0; i < this->length; ++i)
          new((void*)&(pNew[i])) T(this->ptr[i]);
      }
      if (hasAlloc)
      {
        pH = getHeader();
        udFree(pH);
      }
      this->ptr = pNew;
    }
  }
}

template <typename T, size_t Count>
inline epSlice<T> epArray<T, Count>::getBuffer() const
{
  if (hasAllocation())
    return epSlice<T>(this->ptr, getHeader()->numAllocated);
  return epSlice<T>(buffer.ptr(), Count);
}

template <typename T, size_t Count>
inline epArray<T, Count>& epArray<T, Count>::operator =(epArray<T, Count> &&rval)
{
  if (this != &rval)
  {
    this->~epArray();
    new(this) epArray<T, Count>(std::move(rval));
  }
  return *this;
}

template <typename T, size_t Count>
template <typename U>
inline epArray<T, Count>& epArray<T, Count>::operator =(epSlice<U> rh)
{
  this->~epArray();
  new(this) epArray<T, Count>(rh.ptr, rh.length);
  return *this;
}

template<typename T, size_t Count>
inline void epArray<T, Count>::clear()
{
  this->length = 0;
}

template <typename T, size_t Count>
template <typename U>
inline epArray<T, Count>& epArray<T, Count>::pushBack(U &&item)
{
  reserve(this->length + 1);
  new((void*)&(this->ptr[this->length++])) T(std::forward<U>(item));
  return *this;
}

template <typename T, size_t Count>
T& epArray<T, Count>::front() const
{
  return this->ptr[0];
}
template <typename T, size_t Count>
T epArray<T, Count>::popFront()
{
  // TODO: this should be removed and uses replaced with a udQueue type.
  T copy(std::move(this->ptr[0]));
  for (int i = 1; i < this->length; ++i)
  {
    this->ptr[i-1].~T();
    new((void*)&this->ptr[i-1]) T(std::move(this->ptr[i]));
  }
  this->ptr[--this->length].~T();
  return copy;
}

template <typename T, size_t Count>
inline T& epArray<T, Count>::pushBack()
{
  reserve(this->length + 1);
  new((void*)&(this->ptr[this->length])) T();
  return this->ptr[this->length++];
}

template <typename T, size_t Count>
void epArray<T, Count>::remove(size_t i)
{
  --this->length;
  for (; i < this->length; ++i)
    this->ptr[i] = this->ptr[i+1];
  this->ptr[i].~T();
}
template <typename T, size_t Count>
inline void epArray<T, Count>::remove(const T *pItem)
{
  remove(this->indexOfElement(pItem));
}
template <typename T, size_t Count>
inline void epArray<T, Count>::removeSwapLast(size_t i)
{
  if (i < this->length - 1)
    this->ptr[i] = this->ptr[this->length-1];
  this->ptr[--this->length].~T();
}
template <typename T, size_t Count>
inline void epArray<T, Count>::removeSwapLast(const T *pItem)
{
  removeSwapLast(this->indexOfElement(pItem));
}

template <typename T, size_t Count>
inline size_t epArray<T, Count>::numToAlloc(size_t i)
{
  // TODO: i'm sure we can imagine a better heuristic...
  return i > 16 ? i * 2 : 16;
}


// epSharedSlice
template <typename T>
inline epSharedSlice<T>::epSharedSlice()
  : rc(nullptr)
{}

template<typename T>
inline epSharedSlice<T>::epSharedSlice(nullptr_t)
  : rc(nullptr)
{}

template<typename T>
inline epSharedSlice<T>::epSharedSlice(std::initializer_list<typename epSharedSlice<T>::ET> list)
  : epSlice<T>(alloc(list.begin(), list.size()))
  , rc(nullptr)
{
  init(list.begin(), list.size());

  // TODO: we don't need to copy static contents into an allocated buffer!
  //       we might want to support RC slices to static data, where the RC is ignored
}

template <typename T>
inline epSharedSlice<T>::epSharedSlice(epSharedSlice<T> &&rval)
  : epSlice<T>(rval)
  , rc(rval.rc)
{
  rval.rc = nullptr;
}

template <typename T>
inline epSharedSlice<T>::epSharedSlice(const epSharedSlice<T> &rcslice)
  : epSlice<T>(rcslice)
  , rc(rcslice.rc)
{
  if (rc)
    ++rc->refCount;
}

template <typename T>
template <typename U>
inline epSharedSlice<T>::epSharedSlice(U *ptr, size_t length)
  : epSlice<T>(alloc(ptr, length))
  , rc(nullptr)
{
  init(ptr, length);
}

template <typename T>
template <typename U>
inline epSharedSlice<T>::epSharedSlice(epSlice<U> slice)
  : epSlice<T>(alloc(slice.ptr, slice.length))
  , rc(nullptr)
{
  init(slice.ptr, slice.length);
}

template <typename T>
inline epSharedSlice<T>::~epSharedSlice()
{
  if (rc && --rc->refCount == 0)
    udFree(rc);
}

template <typename T>
inline epSharedSlice<T>& epSharedSlice<T>::operator =(const epSharedSlice<T> &rh)
{
  if(rc != rh.rc)
  {
    this->~epSharedSlice();
    rc = rh.rc;
    ++rc->refCount;
  }
  this->ptr = rh.ptr; this->length = rh.length;
  return *this;
}

template <typename T>
inline epSharedSlice<T>& epSharedSlice<T>::operator =(epSharedSlice<T> &&rval)
{
  if (this != &rval)
  {
    this->~epSharedSlice();
    new(this) epSharedSlice<T>(std::move(rval));
  }
  return *this;
}

template <typename T>
template <typename U>
inline epSharedSlice<T>& epSharedSlice<T>::operator =(epSlice<U> rh)
{
  *this = epSharedSlice(rh);
  return *this;
}

template <typename T>
inline epSharedSlice<T> epSharedSlice<T>::alloc(size_t elements)
{
  return epSharedSlice<T>(udAllocType(ET, elements, udAF_None), elements);
}

template <typename T>
inline epSharedSlice<T> epSharedSlice<T>::slice(size_t first, size_t last) const
{
  EPASSERT(last <= this->length && first <= last, "Index out of range!");
  return epSharedSlice(this->ptr + first, last - first, rc);
}

template <typename T>
inline epSharedSlice<T>::epSharedSlice(T *ptr, size_t length, epRC *rc)
  : epSlice<T>(ptr, length)
  , rc(rc)
{
  if (rc)
    ++rc->refCount;
}

template <typename T>
inline size_t epSharedSlice<T>::numToAlloc(size_t i)
{
  // TODO: i'm sure we can imagine a better heuristic...
  return i > 16 ? i : 16;
}

template <typename T>
template <typename U>
inline epSlice<T> epSharedSlice<T>::alloc(U *ptr, size_t length)
{
  if(!ptr || !length)
    return epSlice<T>();
  size_t alloc = numToAlloc(length);
  return epSlice<T>((T*)udAlloc(sizeof(epRC) + alloc*sizeof(typename epSharedSlice<T>::ET)), alloc);
}

template <typename T>
template <typename U>
inline void epSharedSlice<T>::init(U *ptr, size_t length)
{
  if(!ptr || !length)
    return;

  // init the RC
  rc = (epRC*)this->ptr;
  rc->refCount = 1;
  rc->allocatedCount = this->length;

  // copy the data
  this->ptr = (T*)((char*)this->ptr + sizeof(epRC));
  this->length = length;
//  if (std::is_pod<T>::value) // TODO: this is only valid if T and U are the same!
//    memcpy((void*)this->ptr, ptr, sizeof(T)*length);
//  else
  {
    for (size_t i = 0; i<length; ++i)
      new((void*)&(this->ptr[i])) T(ptr[i]);
  }
}


/**** concatenation code ****/

// TODO: support concatenating compatible epSlice<T>'s

// functions that count the length of inputs
inline size_t count(size_t len) // terminator
{
  return len;
}
template<typename T, typename... Args>
inline size_t count(size_t len, const T &, const Args&... args) // T
{
  return count(len + 1, args...);
}

// functions that append inputs
template<typename T>
inline void append(T*) {}
template<typename T, typename U, typename... Args>
inline void append(T *pBuffer, const U &a, const Args&... args)
{
  new((void*)pBuffer) T(a);
  append(pBuffer + 1, args...);
}

template<typename T, size_t Count>
template<typename... Things>
inline epArray<T, Count>& epArray<T, Count>::concat(const Things&... things)
{
  size_t len = this->length + count(0, things...);
  reserve(len);
  append<T>(this->ptr + this->length, things...);
  this->length = len;
  return *this;
}

template<typename T>
template<typename... Things>
inline epSharedSlice<T> epSharedSlice<T>::concat(const Things&... things)
{
  size_t len = count(0, things...);
  epRC *pRC = (epRC*)udAlloc(sizeof(epRC) + sizeof(T)*len);
  pRC->refCount = 0;
  pRC->allocatedCount = len;
  T *ptr = (T*)(pRC + 1);
  append<T>(ptr, things...);
  return epSharedSlice(ptr, len, pRC);
}
