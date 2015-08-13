
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
inline T& udSlice<T>::operator[](size_t i) const
{
  UDASSERT(i < length, "Index out of range!");
  return ptr[i];
}

template<typename T>
inline udSlice<T> udSlice<T>::slice(size_t first, size_t last) const
{
  UDASSERT(last <= length && first <= last, "Index out of range!");
  return udSlice<T>(ptr + first, last - first);
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
udIterator<T> udSlice<T>::begin() const
{
  return udIterator<T>(&ptr[0]);
}
template<typename T>
udIterator<T> udSlice<T>::end() const
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
inline udSlice<T> udSlice<T>::front(size_t n) const
{
  return slice(0, n);
}
template<typename T>
inline udSlice<T> udSlice<T>::back(size_t n) const
{
  return slice(length-n, length);
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
inline udSlice<T> udSlice<T>::popFront(size_t n)
{
  *this = slice(n, length);
  return udSlice<T>(ptr - n, n);
}
template<typename T>
inline udSlice<T> udSlice<T>::popBack(size_t n)
{
  *this = slice(0, length - n);
  return udSlice<T>(ptr + length, n);
}

template<typename T>
inline udSlice<T> udSlice<T>::stripFront(size_t n) const
{
  return slice(n, length);
}
template<typename T>
inline udSlice<T> udSlice<T>::stripBack(size_t n) const
{
  return slice(0, length-n);
}

template<typename T>
inline ptrdiff_t udSlice<T>::offsetOf(const T &c) const
{
  size_t offset = 0;
  while (offset < length && ptr[offset] != c)
    ++offset;
  return offset == length ? -1 : offset;
}

template<typename T>
inline ptrdiff_t udSlice<T>::offsetOfLast(const T &c) const
{
  ptrdiff_t last = length-1;
  while (last >= 0 && ptr[last] != c)
    --last;
  return last;
}

template<typename T>
inline bool udSlice<T>::canFind(T c) const
{
  return offsetOf(c) != -1;
}

template<typename T>
inline udSlice<T> udSlice<T>::find(T c) const
{
  size_t offset = 0;
  while (offset < length && ptr[offset] != c)
    ++offset;
  return slice(offset, length);
}

template<typename T>
inline udSlice<T> udSlice<T>::findBack(T c) const
{
  size_t last = length;
  while (ptr[last-1] != c && last > 0)
    --last;
  return slice(0, last);
}

template<typename T>
T* udSlice<T>::search(Predicate pred) const
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
    while (offset < length && delimiters.canFind(ptr[offset]))
      ++offset;
    if (offset == length)
      return udSlice<T>();
  }
  size_t end = offset;
  while (end < length && !delimiters.canFind(ptr[end]))
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
udSlice<udSlice<T>> udSlice<T>::tokenise(udSlice<udSlice<T>> tokens, udSlice<T> delimiters)
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
void udSlice<T>::copyTo(udSlice<U> dest) const
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
    this->ptr = (T*)buffer;
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
void udFixedSlice<T, Count>::reserve(size_t count)
{
  bool hasAlloc = hasAllocation();
  if (!hasAlloc && count <= Count)
    this->ptr = (T*)buffer;
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
udSlice<T> udFixedSlice<T, Count>::getBuffer() const
{
  if (hasAllocation())
    return udSlice<T>(this->ptr, getHeader()->numAllocated);
  return udSlice<T>((T*)buffer, Count);
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
template <typename U> udFixedSlice<T, Count>& udFixedSlice<T, Count>::pushBack(const U &item)
{
  reserve(this->length + 1);
  new((void*)&(this->ptr[this->length++])) T(item);
  return *this;
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
void udFixedSlice<T, Count>::remove(const T& item)
{
  removeSwapLast(this->offsetOf(item));
}
template <typename T, size_t Count>
void udFixedSlice<T, Count>::removeSwapLast(size_t i)
{
  if (i < this->length - 1)
    this->ptr[i] = this->ptr[this->length-1];
  this->ptr[--this->length].~T();
}
template <typename T, size_t Count>
void udFixedSlice<T, Count>::removeSwapLast(const T& item)
{
  removeSwapLast(this->offsetOf(item));
}

template <typename T, size_t Count>
inline size_t udFixedSlice<T, Count>::numToAlloc(size_t i)
{
  // TODO: i'm sure we can imagine a better heuristic...
  return i > 16 ? i * 2 : 16;
}


// udRCSlice
template <typename T>
inline udRCSlice<T>::udRCSlice()
  : rc(nullptr)
{}

template <typename T>
inline udRCSlice<T>::udRCSlice(udRCSlice<T> &&rval)
  : udSlice<T>(rval)
  , rc(rval.rc)
{
  rval.rc = nullptr;
}

template <typename T>
inline udRCSlice<T>::udRCSlice(const udRCSlice<T> &rcslice)
  : udSlice<T>(rcslice)
  , rc(rcslice.rc)
{
  if (rc)
    ++rc->refCount;
}

template <typename T>
template <typename U>
inline udRCSlice<T>::udRCSlice(U *ptr, size_t length)
  : udSlice<T>(alloc(ptr, length))
  , rc(nullptr)
{
  init(ptr, length);
}

template <typename T>
template <typename U>
inline udRCSlice<T>::udRCSlice(udSlice<U> slice)
  : udSlice<T>(alloc(slice.ptr, slice.length))
  , rc(nullptr)
{
  init(slice.ptr, slice.length);
}

template <typename T>
inline udRCSlice<T>::~udRCSlice()
{
  if (rc && --rc->refCount == 0)
    udFree(rc);
}

template <typename T>
inline udRCSlice<T>& udRCSlice<T>::operator =(const udRCSlice<T> &rh)
{
  if(rc != rh.rc)
  {
    this->~udRCSlice();
    rc = rh.rc;
    ++rc->refCount;
  }
  this->ptr = rh.ptr; this->length = rh.length;
  return *this;
}

template <typename T>
inline udRCSlice<T>& udRCSlice<T>::operator =(udRCSlice<T> &&rval)
{
  if (this != &rval)
  {
    this->~udRCSlice();
    new(this) udRCSlice<T>(std::move(rval));
  }
  return *this;
}

template <typename T>
template <typename U>
inline udRCSlice<T>& udRCSlice<T>::operator =(udSlice<U> rh)
{
  *this = udRCSlice(rh);
  return *this;
}

template <typename T>
inline udRCSlice<T> udRCSlice<T>::slice(size_t first, size_t last) const
{
  UDASSERT(last <= this->length && first <= last, "Index out of range!");
  return udRCSlice(this->ptr + first, last - first, rc);
}

template <typename T>
inline udRCSlice<T>::udRCSlice(T *ptr, size_t length, udRC *rc)
  : udSlice<T>(ptr, length)
  , rc(rc)
{
  if (rc)
    ++rc->refCount;
}

template <typename T>
inline size_t udRCSlice<T>::numToAlloc(size_t i)
{
  // TODO: i'm sure we can imagine a better heuristic...
  return i > 16 ? i : 16;
}

template <typename T>
template <typename U>
inline udSlice<T> udRCSlice<T>::alloc(U *ptr, size_t length)
{
  if(!ptr || !length)
    return udSlice<T>();
  size_t alloc = numToAlloc(length);
  return udSlice<T>((T*)udAlloc(sizeof(udRC) + alloc*sizeof(T)), alloc);
}

template <typename T>
template <typename U>
inline void udRCSlice<T>::init(U *ptr, size_t length)
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
inline udRCSlice<T> udRCSlice<T>::concat(const Things&... things)
{
  size_t len = count(0, things...);
  udRC *pRC = (udRC*)udAlloc(sizeof(udRC) + sizeof(T)*len);
  pRC->refCount = 0;
  pRC->allocatedCount = len;
  T *ptr = (T*)(pRC + 1);
  append<T>(ptr, things...);
  return udRCSlice(ptr, len, pRC);
}
