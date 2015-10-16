#include <type_traits>
#include <utility>

namespace ep {

// Slice
template<typename T>
inline Slice<T>::Slice()
  : length(0), ptr(nullptr)
{}

template<typename T>
inline Slice<T>::Slice(nullptr_t)
  : length(0), ptr(nullptr)
{}

template<typename T>
inline Slice<T>::Slice(std::initializer_list<ET> list)
  : length(list.size()), ptr(list.begin())
{}

template<typename T>
inline Slice<T>::Slice(T* ptr, size_t length)
  : length(length), ptr(ptr)
{}

template<typename T>
template<typename U>
inline Slice<T>::Slice(Slice<U> rh)
  : length(rh.length), ptr(rh.ptr)
{}

template<typename T>
template<typename U>
inline Slice<T>& Slice<T>::operator =(Slice<U> rh)
{
  length = rh.length;
  ptr = rh.ptr;
  return *this;
}

template<typename T>
inline typename Slice<T>::ET& Slice<T>::operator[](ptrdiff_t i) const
{
  size_t offset = (size_t)(i < 0 ? i + length : i);
  EPASSERT(offset < length, "Index out of range!");
  return ((ET*)ptr)[offset];
}

template<typename T>
inline Slice<T> Slice<T>::slice(ptrdiff_t first, ptrdiff_t last) const
{
  size_t start = (size_t)(first < 0 ? first + length : first);
  size_t end = (size_t)(last < 0 ? last + length : last);
  EPASSERT(end <= length && start <= end, "Index out of range!");
  return Slice<T>((ET*)ptr + start, end - start);
}

template<typename T>
inline bool Slice<T>::empty() const
{
  return length == 0;
}

template<typename T>
inline bool Slice<T>::operator ==(Slice<const T> rh) const
{
  return ptr == rh.ptr && length == rh.length;
}

template<typename T>
inline bool Slice<T>::operator !=(Slice<const T> rh) const
{
  return ptr != rh.ptr || length != rh.length;
}

template<typename T>
template<typename U>
inline bool Slice<T>::eq(Slice<U> rh) const
{
  if (length != rh.length)
    return false;
  for (size_t i = 0; i<length; ++i)
    if (((ET*)ptr)[i] != ((ET*)rh.ptr)[i])
      return false;
  return true;
}

template<typename T>
template<typename U>
inline ptrdiff_t Slice<T>::cmp(Slice<U> rh) const
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
inline bool Slice<T>::beginsWith(Slice<U> rh) const
{
  if (length < rh.length)
    return false;
  return slice(0, rh.length).eq(rh);
}
template<typename T>
template<typename U>
inline bool Slice<T>::endsWith(Slice<U> rh) const
{
  if (length < rh.length)
    return false;
  return slice(length - rh.length, length).eq(rh);
}

template<typename T>
inline Iterator<T> Slice<T>::begin() const
{
  return Iterator<T>(&ptr[0]);
}
template<typename T>
inline Iterator<T> Slice<T>::end() const
{
  return Iterator<T>(&ptr[length]);
}

template<typename T>
inline typename Slice<T>::ET& Slice<T>::front() const
{
  return ((ET*)ptr)[0];
}
template<typename T>
inline typename Slice<T>::ET& Slice<T>::back() const
{
  return ((ET*)ptr)[length-1];
}
template<typename T>
inline typename Slice<T>::ET& Slice<T>::popFront()
{
  EPASSERT(length > 0, "Empty slice!");
  ++ptr;
  --length;
  return ((ET*)ptr)[-1];
}
template<typename T>
inline typename Slice<T>::ET& Slice<T>::popBack()
{
  EPASSERT(length > 0, "Empty slice!");
  --length;
  return ((ET*)ptr)[length];
}

template<typename T>
inline Slice<T> Slice<T>::get(ptrdiff_t n) const
{
  if (n < 0)
    return slice(n, length);
  else
    return slice(0, n);
}
template<typename T>
inline Slice<T> Slice<T>::pop(ptrdiff_t n)
{
  if (n < 0)
  {
    *this = slice(0, n);
    return Slice<T>(ptr - n, -n);
  }
  else
  {
    *this = slice(n, length);
    return Slice<T>(ptr - n, n);
  }
}
template<typename T>
inline Slice<T> Slice<T>::strip(ptrdiff_t n) const
{
  if (n < 0)
    return slice(0, n);
  else
    return slice(n, length);
}

template<typename T>
inline bool Slice<T>::exists(const typename Slice<T>::ET &c, size_t *pIndex) const
{
  size_t i = findFirst(c);
  if (pIndex)
    *pIndex = i;
  return i != length;
}

template<typename T>
inline size_t Slice<T>::findFirst(const typename Slice<T>::ET &c) const
{
  size_t offset = 0;
  while (offset < length && ptr[offset] != c)
    ++offset;
  return offset;
}
template<typename T>
inline size_t Slice<T>::findLast(const typename Slice<T>::ET &c) const
{
  ptrdiff_t last = length-1;
  while (last >= 0 && ptr[last] != c)
    --last;
  return last < 0 ? length : last;
}
template<typename T>
template<typename U>
inline size_t Slice<T>::findFirst(Slice<U> s) const
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
inline size_t Slice<T>::findLast(Slice<U> s) const
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
inline Slice<T> Slice<T>::getLeftAtFirst(const typename Slice<T>::ET &c, bool bInclusive) const
{
  return slice(0, findFirst(c) + (bInclusive ? 1 : 0));
}
template<typename T>
inline Slice<T> Slice<T>::getLeftAtLast(const typename Slice<T>::ET &c, bool bInclusive) const
{
  return slice(0, findLast(c) + (bInclusive ? 1 : 0));
}
template<typename T>
inline Slice<T> Slice<T>::getRightAtFirst(const typename Slice<T>::ET &c, bool bInclusive) const
{
  return slice(findFirst(c) + (bInclusive ? 0 : 1), length);
}
template<typename T>
inline Slice<T> Slice<T>::getRightAtLast(const typename Slice<T>::ET &c, bool bInclusive) const
{
  return slice(findLast(c) + (bInclusive ? 0 : 1), length);
}

template<typename T>
template<typename U>
inline Slice<T> Slice<T>::getLeftAtFirst(Slice<U> s, bool bInclusive) const
{
  return slice(0, findFirst(s) + (bInclusive ? s.length : 0));
}
template<typename T>
template<typename U>
inline Slice<T> Slice<T>::getLeftAtLast(Slice<U> s, bool bInclusive) const
{
  return slice(0, findLast(s) + (bInclusive ? s.length : 0));
}
template<typename T>
template<typename U>
inline Slice<T> Slice<T>::getRightAtFirst(Slice<U> s, bool bInclusive) const
{
  return slice(findFirst(s) + (bInclusive ? 0 : s.length), length);
}
template<typename T>
template<typename U>
inline Slice<T> Slice<T>::getRightAtLast(Slice<U> s, bool bInclusive) const
{
  return slice(findLast(s) + (bInclusive ? 0 : s.length), length);
}

template<typename T>
inline ptrdiff_t Slice<T>::indexOfElement(const T *c) const
{
  size_t offset = 0;
  while (offset < length && &ptr[offset] != c)
    ++offset;
  return offset == length ? -1 : offset;
}

template<typename T>
inline T* Slice<T>::search(Predicate pred) const
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
inline Slice<T> Slice<T>::popToken(Slice<T> delimiters)
{
  size_t offset = 0;
  if (skipEmptyTokens)
  {
    while (offset < length && delimiters.exists(ptr[offset]))
      ++offset;
    if (offset == length)
    {
      ptr += offset;
      length = 0;
      return Slice<T>();
    }
  }
  size_t end = offset;
  while (end < length && !delimiters.exists(ptr[end]))
    ++end;
  Slice<T> token = slice(offset, end);
  if (end < length)
    ++end;
  ptr += end;
  length -= end;
  return token;
}

template<typename T>
template<bool skipEmptyTokens>
inline Slice<Slice<T>> Slice<T>::tokenise(Slice<Slice<T>> tokens, Slice<T> delimiters)
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
inline void Slice<T>::copyTo(Slice<U> dest) const
{
  EPASSERT(dest.length >= length, "Not enough elements!");
  for (size_t i = 0; i<length; ++i)
    dest.ptr[i] = ptr[i];
}


// Array
template <typename T, size_t Count>
inline Array<T, Count>::Array()
{}

template <typename T, size_t Count>
inline Array<T, Count>::Array(nullptr_t)
{}

template <typename T, size_t Count>
inline Array<T, Count>::Array(std::initializer_list<T> list)
  : Array(list.begin(), list.size())
{}

template <typename T, size_t Count>
inline Array<T, Count>::Array(const Array<T, Count> &val)
  : Array(val.ptr, val.length)
{}

template <typename T, size_t Count>
inline Array<T, Count>::Array(Array<T, Count> &&rval)
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
inline Array<T, Count>::Array(U *ptr, size_t length)
  : Slice<T>()
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
inline Array<T, Count>::Array(Slice<U> slice)
  : Array<T, Count>(slice.ptr, slice.length)
{
}

template <typename T, size_t Count>
inline Array<T, Count>::~Array()
{
  for (size_t i = 0; i < this->length; ++i)
    this->ptr[i].~T();
  if (hasAllocation())
  {
    Header *pHeader = getHeader();
    epFree(pHeader);
  }
}

template <typename T, size_t Count>
inline void Array<T, Count>::reserve(size_t count)
{
  bool hasAlloc = hasAllocation();
  if (!hasAlloc && count <= Count)
    this->ptr = buffer.ptr();
  else if (count > Count)
  {
    bool needsExtend = hasAlloc && getHeader()->numAllocated < count;
    if (!hasAlloc || needsExtend)
    {
      Header *pH = (Header*)epAlloc(sizeof(Header) + sizeof(T)*count);
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
        epFree(pH);
      }
      this->ptr = pNew;
    }
  }
}

template <typename T, size_t Count>
inline Slice<T> Array<T, Count>::getBuffer() const
{
  if (hasAllocation())
    return Slice<T>(this->ptr, getHeader()->numAllocated);
  return Slice<T>(buffer.ptr(), Count);
}

template <typename T, size_t Count>
inline Array<T, Count>& Array<T, Count>::operator =(Array<T, Count> &&rval)
{
  if (this != &rval)
  {
    this->~Array();
    new(this) Array<T, Count>(std::move(rval));
  }
  return *this;
}

template <typename T, size_t Count>
template <typename U>
inline Array<T, Count>& Array<T, Count>::operator =(Slice<U> rh)
{
  this->~Array();
  new(this) Array<T, Count>(rh.ptr, rh.length);
  return *this;
}

template<typename T, size_t Count>
inline void Array<T, Count>::clear()
{
  this->length = 0;
}

template <typename T, size_t Count>
template <typename U>
inline Array<T, Count>& Array<T, Count>::pushBack(U &&item)
{
  reserve(this->length + 1);
  new((void*)&(this->ptr[this->length++])) T(std::forward<U>(item));
  return *this;
}

template <typename T, size_t Count>
T& Array<T, Count>::front() const
{
  return this->ptr[0];
}
template <typename T, size_t Count>
T Array<T, Count>::popFront()
{
  // TODO: this should be removed and uses replaced with a udQueue type.
  T copy(std::move(this->ptr[0]));
  for (size_t i = 1; i < this->length; ++i)
  {
    this->ptr[i-1].~T();
    new((void*)&this->ptr[i-1]) T(std::move(this->ptr[i]));
  }
  this->ptr[--this->length].~T();
  return copy;
}

template <typename T, size_t Count>
inline T& Array<T, Count>::pushBack()
{
  reserve(this->length + 1);
  new((void*)&(this->ptr[this->length])) T();
  return this->ptr[this->length++];
}

template <typename T, size_t Count>
void Array<T, Count>::remove(size_t i)
{
  --this->length;
  for (; i < this->length; ++i)
    this->ptr[i] = this->ptr[i+1];
  this->ptr[i].~T();
}
template <typename T, size_t Count>
inline void Array<T, Count>::remove(const T *pItem)
{
  remove(this->indexOfElement(pItem));
}
template <typename T, size_t Count>
inline void Array<T, Count>::removeSwapLast(size_t i)
{
  if (i < this->length - 1)
    this->ptr[i] = this->ptr[this->length-1];
  this->ptr[--this->length].~T();
}
template <typename T, size_t Count>
inline void Array<T, Count>::removeSwapLast(const T *pItem)
{
  removeSwapLast(this->indexOfElement(pItem));
}

template <typename T, size_t Count>
inline size_t Array<T, Count>::numToAlloc(size_t i)
{
  // TODO: i'm sure we can imagine a better heuristic...
  return i > 16 ? i * 2 : 16;
}


// SharedSlice
template <typename T>
inline SharedSlice<T>::SharedSlice()
  : rc(nullptr)
{}

template<typename T>
inline SharedSlice<T>::SharedSlice(nullptr_t)
  : rc(nullptr)
{}

template<typename T>
inline SharedSlice<T>::SharedSlice(std::initializer_list<typename SharedSlice<T>::ET> list)
  : Slice<T>(alloc(list.begin(), list.size()))
  , rc(nullptr)
{
  init(list.begin(), list.size());

  // TODO: we don't need to copy static contents into an allocated buffer!
  //       we might want to support RC slices to static data, where the RC is ignored
}

template <typename T>
inline SharedSlice<T>::SharedSlice(SharedSlice<T> &&rval)
  : Slice<T>(rval)
  , rc(rval.rc)
{
  rval.rc = nullptr;
}

template <typename T>
inline SharedSlice<T>::SharedSlice(const SharedSlice<T> &rcslice)
  : Slice<T>(rcslice)
  , rc(rcslice.rc)
{
  if (rc)
    ++rc->refCount;
}

template <typename T>
template <typename U>
inline SharedSlice<T>::SharedSlice(U *ptr, size_t length)
  : Slice<T>(alloc(ptr, length))
  , rc(nullptr)
{
  init(ptr, length);
}

template <typename T>
template <typename U>
inline SharedSlice<T>::SharedSlice(Slice<U> slice)
  : Slice<T>(alloc(slice.ptr, slice.length))
  , rc(nullptr)
{
  init(slice.ptr, slice.length);
}

template <typename T>
inline SharedSlice<T>::~SharedSlice()
{
  if (rc && --rc->refCount == 0)
    epFree(rc);
}

template <typename T>
inline SharedSlice<T>& SharedSlice<T>::operator =(const SharedSlice<T> &rh)
{
  if (rc != rh.rc)
  {
    this->~SharedSlice();
    rc = rh.rc;
    ++rc->refCount;
  }
  this->ptr = rh.ptr; this->length = rh.length;
  return *this;
}

template <typename T>
inline SharedSlice<T>& SharedSlice<T>::operator =(SharedSlice<T> &&rval)
{
  if (this != &rval)
  {
    this->~SharedSlice();
    new(this) SharedSlice<T>(std::move(rval));
  }
  return *this;
}

template <typename T>
template <typename U>
inline SharedSlice<T>& SharedSlice<T>::operator =(Slice<U> rh)
{
  *this = SharedSlice(rh);
  return *this;
}

template <typename T>
inline SharedSlice<T> SharedSlice<T>::alloc(size_t elements)
{
  return SharedSlice<T>((typename SharedSlice<T>::ET*)epAlloc(sizeof(typename SharedSlice<T>::ET) * elements), elements);
}

template <typename T>
inline SharedSlice<T> SharedSlice<T>::slice(size_t first, size_t last) const
{
  EPASSERT(last <= this->length && first <= last, "Index out of range!");
  return SharedSlice(this->ptr + first, last - first, rc);
}

template <typename T>
inline SharedSlice<T>::SharedSlice(T *ptr, size_t length, RC *rc)
  : Slice<T>(ptr, length)
  , rc(rc)
{
  if (rc)
    ++rc->refCount;
}

template <typename T>
inline size_t SharedSlice<T>::numToAlloc(size_t i)
{
  // TODO: i'm sure we can imagine a better heuristic...
  return i > 16 ? i : 16;
}

template <typename T>
template <typename U>
inline Slice<T> SharedSlice<T>::alloc(U *ptr, size_t length)
{
  if (!ptr || !length)
    return Slice<T>();
  size_t alloc = numToAlloc(length);
  return Slice<T>((T*)epAlloc(sizeof(RC) + alloc*sizeof(typename SharedSlice<T>::ET)), alloc);
}

template <typename T>
template <typename U>
inline void SharedSlice<T>::init(U *ptr, size_t length)
{
  if (!ptr || !length)
    return;

  // init the RC
  rc = (RC*)this->ptr;
  rc->refCount = 1;
  rc->allocatedCount = this->length;

  // copy the data
  this->ptr = (T*)((char*)this->ptr + sizeof(RC));
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

// TODO: support concatenating compatible Slice<T>'s

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
inline Array<T, Count>& Array<T, Count>::concat(const Things&... things)
{
  size_t len = this->length + count(0, things...);
  reserve(len);
  append<T>(this->ptr + this->length, things...);
  this->length = len;
  return *this;
}

template<typename T>
template<typename... Things>
inline SharedSlice<T> SharedSlice<T>::concat(const Things&... things)
{
  size_t len = count(0, things...);
  RC *pRC = (RC*)epAlloc(sizeof(RC) + sizeof(T)*len);
  pRC->refCount = 0;
  pRC->allocatedCount = len;
  T *ptr = (T*)(pRC + 1);
  append<T>(ptr, things...);
  return SharedSlice(ptr, len, pRC);
}

} // namespace ep
