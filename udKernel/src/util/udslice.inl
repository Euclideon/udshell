#include <type_traits>
#include <utility>

// udSlice
template<typename T>
inline udSlice<T>::udSlice()
  : length(0), ptr(nullptr)
{}

template<typename T>
inline udSlice<T>::udSlice(nullptr_t)
  : length(0), ptr(nullptr)
{}

template<typename T>
inline udSlice<T>::udSlice(std::initializer_list<T> list)
  : length(list.size()) , ptr(list.begin())
{}

template<typename T>
inline udSlice<T>::udSlice(T* ptr, size_t length)
  : length(length), ptr(ptr)
{}

template<typename T>
template<typename U>
inline udSlice<T>::udSlice(udSlice<U> rh)
  : length(rh.length), ptr(rh.ptr)
{}

template<typename T>
template<typename U>
inline udSlice<T>& udSlice<T>::operator =(udSlice<U> rh)
{
  length = rh.length;
  ptr = rh.ptr;
  return *this;
}

template<typename T>
inline T& udSlice<T>::operator[](ptrdiff_t i) const
{
  size_t offset = (size_t)(i < 0 ? i + length : i);
  UDASSERT(offset < length, "Index out of range!");
  return ptr[offset];
}

template<typename T>
inline udSlice<T> udSlice<T>::slice(ptrdiff_t first, ptrdiff_t last) const
{
  size_t start = (size_t)(first < 0 ? first + length : first);
  size_t end = (size_t)(last < 0 ? last + length : last);
  UDASSERT(end <= length && start <= end, "Index out of range!");
  return udSlice<T>(ptr + start, end - start);
}

template<typename T>
inline bool udSlice<T>::empty() const
{
  return length == 0;
}

template<typename T>
inline bool udSlice<T>::operator ==(udSlice<const T> rh) const
{
  return ptr == rh.ptr && length == rh.length;
}

template<typename T>
inline bool udSlice<T>::operator !=(udSlice<const T> rh) const
{
  return ptr != rh.ptr || length != rh.length;
}

template<typename T>
template<typename U>
inline bool udSlice<T>::eq(udSlice<U> rh) const
{
  if(length != rh.length)
    return false;
  for(size_t i=0; i<length; ++i)
    if(ptr[i] != rh.ptr[i])
      return false;
  return true;
}

template<typename T>
template<typename U>
inline ptrdiff_t udSlice<T>::cmp(udSlice<U> rh) const
{
  size_t len = length < rh.length ? length : rh.length;
  for (size_t i = 0; i < len; ++i)
  {
    if (ptr[i] == rh.ptr[i])
      continue;
    return ptr[i] < rh.ptr[i] ? -1 : 1;
  }
  return length - rh.length;
}

template<typename T>
template<typename U>
inline bool udSlice<T>::beginsWith(udSlice<U> rh) const
{
  if (length < rh.length)
    return false;
  return slice(0, rh.length).eq(rh);
}
template<typename T>
template<typename U>
inline bool udSlice<T>::endsWith(udSlice<U> rh) const
{
  if (length < rh.length)
    return false;
  return slice(length - rh.length, length).eq(rh);
}

template<typename T>
inline udIterator<T> udSlice<T>::begin() const
{
  return udIterator<T>(&ptr[0]);
}
template<typename T>
inline udIterator<T> udSlice<T>::end() const
{
  return udIterator<T>(&ptr[length]);
}

template<typename T>
inline T& udSlice<T>::front() const
{
  return ptr[0];
}
template<typename T>
inline T& udSlice<T>::back() const
{
  return ptr[length-1];
}
template<typename T>
inline T& udSlice<T>::popFront()
{
  *this = slice(1, length);
  return ptr[-1];
}
template<typename T>
inline T& udSlice<T>::popBack()
{
  *this = slice(0, length - 1);
  return ptr[length];
}

template<typename T>
inline udSlice<T> udSlice<T>::get(ptrdiff_t n) const
{
  if (n < 0)
    return slice(n, length);
  else
    return slice(0, n);
}
template<typename T>
inline udSlice<T> udSlice<T>::pop(ptrdiff_t n)
{
  if (n < 0)
  {
    *this = slice(0, n);
    return udSlice<T>(ptr - n, -n);
  }
  else
  {
    *this = slice(n, length);
    return udSlice<T>(ptr - n, n);
  }
}
template<typename T>
inline udSlice<T> udSlice<T>::strip(ptrdiff_t n) const
{
  if (n < 0)
    return slice(0, n);
  else
    return slice(n, length);
}

template<typename T>
inline bool udSlice<T>::exists(const T &c, size_t *pIndex) const
{
  size_t i = findFirst(c) != -1;
  if (pIndex)
    *pIndex = i;
  return i == length;
}

template<typename T>
inline size_t udSlice<T>::findFirst(const T &c) const
{
  size_t offset = 0;
  while (offset < length && ptr[offset] != c)
    ++offset;
  return offset;
}
template<typename T>
inline size_t udSlice<T>::findLast(const T &c) const
{
  ptrdiff_t last = length-1;
  while (last >= 0 && ptr[last] != c)
    --last;
  return last < 0 ? length : last;
}
template<typename T>
template<typename U>
inline size_t udSlice<T>::findFirst(udSlice<U> s) const
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
inline size_t udSlice<T>::findLast(udSlice<U> s) const
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
inline udSlice<T> udSlice<T>::getLeftAtFirst(const T &c, bool bInclusive) const
{
  return slice(0, findFirst(c) + (bInclusive ? 1 : 0));
}
template<typename T>
inline udSlice<T> udSlice<T>::getLeftAtLast(const T &c, bool bInclusive) const
{
  return slice(0, findLast(c) + (bInclusive ? 1 : 0));
}
template<typename T>
inline udSlice<T> udSlice<T>::getRightAtFirst(const T &c, bool bInclusive) const
{
  return slice(findFirst(c) + (bInclusive ? 0 : 1), length);
}
template<typename T>
inline udSlice<T> udSlice<T>::getRightAtLast(const T &c, bool bInclusive) const
{
  return slice(findLast(c) + (bInclusive ? 0 : 1), length);
}

template<typename T>
template<typename U>
inline udSlice<T> udSlice<T>::getLeftAtFirst(udSlice<U> s, bool bInclusive) const
{
  return slice(0, findFirst(s) + (bInclusive ? s.length : 0));
}
template<typename T>
template<typename U>
inline udSlice<T> udSlice<T>::getLeftAtLast(udSlice<U> s, bool bInclusive) const
{
  return slice(0, findLast(s) + (bInclusive ? s.length : 0));
}
template<typename T>
template<typename U>
inline udSlice<T> udSlice<T>::getRightAtFirst(udSlice<U> s, bool bInclusive) const
{
  return slice(findFirst(s) + (bInclusive ? 0 : s.length), length);
}
template<typename T>
template<typename U>
inline udSlice<T> udSlice<T>::getRightAtLast(udSlice<U> s, bool bInclusive) const
{
  return slice(findLast(s) + (bInclusive ? 0 : s.length), length);
}

template<typename T>
inline ptrdiff_t udSlice<T>::indexOfElement(const T *c) const
{
  size_t offset = 0;
  while (offset < length && &ptr[offset] != c)
    ++offset;
  return offset == length ? -1 : offset;
}

template<typename T>
inline T* udSlice<T>::search(Predicate pred) const
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
inline udSlice<T> udSlice<T>::popToken(udSlice<T> delimiters)
{
  size_t offset = 0;
  if (skipEmptyTokens)
  {
    while (offset < length && delimiters.exists(ptr[offset]))
      ++offset;
    if (offset == length)
      return udSlice<T>();
  }
  size_t end = offset;
  while (end < length && !delimiters.exists(ptr[end]))
    ++end;
  udSlice<T> token = slice(offset, end);
  if (end < length)
    ++end;
  ptr += end;
  length -= end;
  return token;
}

template<typename T>
template<bool skipEmptyTokens>
inline udSlice<udSlice<T>> udSlice<T>::tokenise(udSlice<udSlice<T>> tokens, udSlice<T> delimiters)
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
inline void udSlice<T>::copyTo(udSlice<U> dest) const
{
  UDASSERT(length >= dest.length, "Not enough elements!");
  for (size_t i = 0; i<dest.length; ++i)
    dest.ptr[i] = ptr[i];
}


// udFixedSlice
template <typename T, size_t Count>
inline udFixedSlice<T, Count>::udFixedSlice()
{}

template <typename T, size_t Count>
inline udFixedSlice<T, Count>::udFixedSlice(nullptr_t)
{}

template <typename T, size_t Count>
inline udFixedSlice<T, Count>::udFixedSlice(std::initializer_list<T> list)
  : udFixedSlice(list.begin(), list.size())
{}

template <typename T, size_t Count>
inline udFixedSlice<T, Count>::udFixedSlice(const udFixedSlice<T, Count> &val)
  : udFixedSlice(val.ptr, val.length)
{}

template <typename T, size_t Count>
inline udFixedSlice<T, Count>::udFixedSlice(udFixedSlice<T, Count> &&rval)
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
inline udFixedSlice<T, Count>::udFixedSlice(U *ptr, size_t length)
  : udSlice<T>()
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
inline udFixedSlice<T, Count>::udFixedSlice(udSlice<U> slice)
  : udFixedSlice<T, Count>(slice.ptr, slice.length)
{
}

template <typename T, size_t Count>
inline udFixedSlice<T, Count>::~udFixedSlice()
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
inline void udFixedSlice<T, Count>::reserve(size_t count)
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
inline udSlice<T> udFixedSlice<T, Count>::getBuffer() const
{
  if (hasAllocation())
    return udSlice<T>(this->ptr, getHeader()->numAllocated);
  return udSlice<T>(buffer.ptr(), Count);
}

template <typename T, size_t Count>
inline udFixedSlice<T, Count>& udFixedSlice<T, Count>::operator =(udFixedSlice<T, Count> &&rval)
{
  if (this != &rval)
  {
    this->~udFixedSlice();
    new(this) udFixedSlice<T, Count>(std::move(rval));
  }
  return *this;
}

template <typename T, size_t Count>
template <typename U>
inline udFixedSlice<T, Count>& udFixedSlice<T, Count>::operator =(udSlice<U> rh)
{
  this->~udFixedSlice();
  new(this) udFixedSlice<T, Count>(rh.ptr, rh.length);
  return *this;
}

template<typename T, size_t Count>
inline void udFixedSlice<T, Count>::clear()
{
  this->length = 0;
}

template <typename T, size_t Count>
template <typename U>
inline udFixedSlice<T, Count>& udFixedSlice<T, Count>::pushBack(U &&item)
{
  reserve(this->length + 1);
  new((void*)&(this->ptr[this->length++])) T(std::forward<U>(item));
  return *this;
}

template <typename T, size_t Count>
T& udFixedSlice<T, Count>::front() const
{
  return this->ptr[0];
}
template <typename T, size_t Count>
T udFixedSlice<T, Count>::popFront()
{
  // TODO: this should be removed and uses replaced with a udQueue type.
  T copy(std::move(ptr[0]));
  for (int i = 1; i < length; ++i)
  {
    ptr[i-1].~T();
    new((void*)&ptr[i-1]) T(std::move(ptr[i]));
  }
  ptr[--length].~T();
  return copy;
}

template <typename T, size_t Count>
inline T& udFixedSlice<T, Count>::pushBack()
{
  reserve(this->length + 1);
  new((void*)&(this->ptr[this->length])) T();
  return this->ptr[this->length++];
}

template <typename T, size_t Count>
void udFixedSlice<T, Count>::remove(size_t i)
{
  --this->length;
  for (; i < this->length; ++i)
    this->ptr[i] = this->ptr[i+1];
  this->ptr[i].~T();
}
template <typename T, size_t Count>
inline void udFixedSlice<T, Count>::remove(const T *pItem)
{
  remove(this->indexOfElement(pItem));
}
template <typename T, size_t Count>
inline void udFixedSlice<T, Count>::removeSwapLast(size_t i)
{
  if (i < this->length - 1)
    this->ptr[i] = this->ptr[this->length-1];
  this->ptr[--this->length].~T();
}
template <typename T, size_t Count>
inline void udFixedSlice<T, Count>::removeSwapLast(const T *pItem)
{
  removeSwapLast(this->indexOfElement(pItem));
}

template <typename T, size_t Count>
inline size_t udFixedSlice<T, Count>::numToAlloc(size_t i)
{
  // TODO: i'm sure we can imagine a better heuristic...
  return i > 16 ? i * 2 : 16;
}


// udSharedSlice
template <typename T>
inline udSharedSlice<T>::udSharedSlice()
  : rc(nullptr)
{}

template<typename T>
inline udSharedSlice<T>::udSharedSlice(nullptr_t)
  : rc(nullptr)
{}

template<typename T>
inline udSharedSlice<T>::udSharedSlice(std::initializer_list<T> list)
  : udSlice<T>(alloc(list.begin(), list.size()))
  , rc(nullptr)
{
  init(list.begin(), list.size());

  // TODO: we don't need to copy static contents into an allocated buffer!
  //       we might want to support RC slices to static data, where the RC is ignored
}

template <typename T>
inline udSharedSlice<T>::udSharedSlice(udSharedSlice<T> &&rval)
  : udSlice<T>(rval)
  , rc(rval.rc)
{
  rval.rc = nullptr;
}

template <typename T>
inline udSharedSlice<T>::udSharedSlice(const udSharedSlice<T> &rcslice)
  : udSlice<T>(rcslice)
  , rc(rcslice.rc)
{
  if (rc)
    ++rc->refCount;
}

template <typename T>
template <typename U>
inline udSharedSlice<T>::udSharedSlice(U *ptr, size_t length)
  : udSlice<T>(alloc(ptr, length))
  , rc(nullptr)
{
  init(ptr, length);
}

template <typename T>
template <typename U>
inline udSharedSlice<T>::udSharedSlice(udSlice<U> slice)
  : udSlice<T>(alloc(slice.ptr, slice.length))
  , rc(nullptr)
{
  init(slice.ptr, slice.length);
}

template <typename T>
inline udSharedSlice<T>::~udSharedSlice()
{
  if (rc && --rc->refCount == 0)
    udFree(rc);
}

template <typename T>
inline udSharedSlice<T>& udSharedSlice<T>::operator =(const udSharedSlice<T> &rh)
{
  if(rc != rh.rc)
  {
    this->~udSharedSlice();
    rc = rh.rc;
    ++rc->refCount;
  }
  this->ptr = rh.ptr; this->length = rh.length;
  return *this;
}

template <typename T>
inline udSharedSlice<T>& udSharedSlice<T>::operator =(udSharedSlice<T> &&rval)
{
  if (this != &rval)
  {
    this->~udSharedSlice();
    new(this) udSharedSlice<T>(std::move(rval));
  }
  return *this;
}

template <typename T>
template <typename U>
inline udSharedSlice<T>& udSharedSlice<T>::operator =(udSlice<U> rh)
{
  *this = udSharedSlice(rh);
  return *this;
}

template <typename T>
inline udSharedSlice<T> udSharedSlice<T>::alloc(size_t elements)
{
  return udSharedSlice<T>(udAllocType(T, elements, udAF_None), elements);
}

template <typename T>
inline udSharedSlice<T> udSharedSlice<T>::slice(size_t first, size_t last) const
{
  UDASSERT(last <= this->length && first <= last, "Index out of range!");
  return udSharedSlice(this->ptr + first, last - first, rc);
}

template <typename T>
inline udSharedSlice<T>::udSharedSlice(T *ptr, size_t length, udRC *rc)
  : udSlice<T>(ptr, length)
  , rc(rc)
{
  if (rc)
    ++rc->refCount;
}

template <typename T>
inline size_t udSharedSlice<T>::numToAlloc(size_t i)
{
  // TODO: i'm sure we can imagine a better heuristic...
  return i > 16 ? i : 16;
}

template <typename T>
template <typename U>
inline udSlice<T> udSharedSlice<T>::alloc(U *ptr, size_t length)
{
  if(!ptr || !length)
    return udSlice<T>();
  size_t alloc = numToAlloc(length);
  return udSlice<T>((T*)udAlloc(sizeof(udRC) + alloc*sizeof(T)), alloc);
}

template <typename T>
template <typename U>
inline void udSharedSlice<T>::init(U *ptr, size_t length)
{
  if(!ptr || !length)
    return;

  // init the RC
  rc = (udRC*)this->ptr;
  rc->refCount = 1;
  rc->allocatedCount = this->length;

  // copy the data
  this->ptr = (T*)((char*)this->ptr + sizeof(udRC));
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

// TODO: support concatenating compatible udSlice<T>'s

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
inline udFixedSlice<T, Count>& udFixedSlice<T, Count>::concat(const Things&... things)
{
  size_t len = this->length + count(0, things...);
  reserve(len);
  append<T>(this->ptr + this->length, things...);
  this->length = len;
  return *this;
}

template<typename T>
template<typename... Things>
inline udSharedSlice<T> udSharedSlice<T>::concat(const Things&... things)
{
  size_t len = count(0, things...);
  udRC *pRC = (udRC*)udAlloc(sizeof(udRC) + sizeof(T)*len);
  pRC->refCount = 0;
  pRC->allocatedCount = len;
  T *ptr = (T*)(pRC + 1);
  append<T>(ptr, things...);
  return udSharedSlice(ptr, len, pRC);
}
