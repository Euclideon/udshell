#include <utility>

namespace ep {

namespace internal {

template<typename T>
inline T* SliceAlloc(size_t elements)
{
  SliceHeader *pH = (SliceHeader*)epAlloc(sizeof(SliceHeader) + sizeof(T)*elements);
  pH->pFreeFunc = [](void *pMem) { epFree(pMem); };
  pH->allocatedCount = elements;
  pH->refCount = 1;
  return (T*)(pH + 1);
}
inline SliceHeader* GetSliceHeader(const void *pBuffer)
{
  return (SliceHeader*)pBuffer - 1;
}
template<typename T>
inline void SliceFree(T *pArray)
{
  SliceHeader *pHeader = GetSliceHeader(pArray);
  pHeader->pFreeFunc(pHeader);
}

// functions that counts inputs for append operations; support appending arrays
template<typename T>
inline size_t count() { return 0; }
// single element; SFINAE asserts that U is implicitly convertable to T
template<typename T, typename U, typename... Args>
inline auto count(U &&item, Args&&... args) -> decltype(T(std::forward<U>(item)), size_t()) // SFINAE fails if U is not implicitly convertable to T
{
  return count<T>(std::forward<Args>(args)...) + 1;
}
// array of element; SFINAE asserts that array element type U is implicitly convertable to T
template<typename T, typename U, typename... Args>
inline auto count(Slice<U> slice, Args&&... args) -> decltype(T(slice[0]), size_t()) // SFINAE fails if array element type U is not implicitly convertible to T
{
  return count<T>(std::forward<Args>(args)...) + slice.length;
}

// functions that append inputs (TODO: look into a not-recursive version?)
template<typename T>
inline T* append(T *pBuffer) { return pBuffer; }
// append single element; SFINAE asserts that U is implicitly convertable to T
template<typename T> T* ReturnTStar();
template<typename T, typename U, typename... Args>
inline auto append(T *pBuffer, U &&a, Args&&... args) -> decltype(T(std::forward<U>(a)), ReturnTStar<T>())
{
  epConstruct((void*)pBuffer) T(std::forward<U>(a));
  append(pBuffer + 1, std::forward<Args>(args)...);
  return pBuffer;
}
// append array of element; SFINAE asserts that array element type U is implicitly convertable to T
template<typename T, typename U, typename... Args>
inline auto append(T *pBuffer, Slice<U> s, Args&&... args) -> decltype(T(s.ptr[0]), ReturnTStar<T>())
{
  for(size_t i = 0; i < s.length; ++i)
    epConstruct((void*)(pBuffer + i)) T(s.ptr[i]);
  append(pBuffer + s.length, std::forward<Args>(args)...);
  return pBuffer;
}

} // namespace internal

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
template<typename U, size_t N>
inline Slice<T>::Slice(U(&arr)[N])
  : length(N), ptr(arr)
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
  EPASSERT_THROW(offset < length, Result::OutOfBounds, "Index out of range!");
  return ((ET*)ptr)[offset];
}

template<typename T>
inline Slice<T> Slice<T>::slice(ptrdiff_t first, ptrdiff_t last) const
{
  size_t start = (size_t)(first < 0 ? first + length : first);
  size_t end = (size_t)(last < 0 ? last + length : last);
  EPASSERT_THROW(end <= length && start <= end, Result::OutOfBounds, "Index out of range!");
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
  EPASSERT_THROW(length > 0, Result::OutOfBounds, "Empty slice!");
  ++ptr;
  --length;
  return ((ET*)ptr)[-1];
}
template<typename T>
inline typename Slice<T>::ET& Slice<T>::popBack()
{
  EPASSERT_THROW(length > 0, Result::OutOfBounds, "Empty slice!");
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

namespace internal {
  // *** horrible specialisation to handle slices of slices
  template<typename T> struct FindSpecialisation                                  { static inline bool eq(const T &a, const T &b) { return a == b; } };
  // TODO: maybe this mess can be eliminated with 'classy' use of std::enable_if<>... ;) ;)
  template<typename U> struct FindSpecialisation<const Slice<U>>                  { static inline bool eq(const Slice<U> &a, const Slice<U> &b) { return a.eq(b); } };
  template<typename U> struct FindSpecialisation<Slice<U>>                        { static inline bool eq(const Slice<U> &a, const Slice<U> &b) { return a.eq(b); } };
  template<typename U, size_t Len> struct FindSpecialisation<const Array<U, Len>> { static inline bool eq(const Slice<U> &a, const Slice<U> &b) { return a.eq(b); } };
  template<typename U, size_t Len> struct FindSpecialisation<Array<U, Len>>       { static inline bool eq(const Slice<U> &a, const Slice<U> &b) { return a.eq(b); } };
  template<typename U> struct FindSpecialisation<const SharedArray<U>>            { static inline bool eq(const Slice<U> &a, const Slice<U> &b) { return a.eq(b); } };
  template<typename U> struct FindSpecialisation<SharedArray<U>>                  { static inline bool eq(const Slice<U> &a, const Slice<U> &b) { return a.eq(b); } };
  template<> struct FindSpecialisation<const String>                              { static inline bool eq(const String &a, const String &b) { return a.eq(b); } };
  template<> struct FindSpecialisation<String>                                    { static inline bool eq(const String &a, const String &b) { return a.eq(b); } };
  template<size_t Len> struct FindSpecialisation<const MutableString<Len>>        { static inline bool eq(const String &a, const String &b) { return a.eq(b); } };
  template<size_t Len> struct FindSpecialisation<MutableString<Len>>              { static inline bool eq(const String &a, const String &b) { return a.eq(b); } };
  template<> struct FindSpecialisation<const SharedString>                        { static inline bool eq(const String &a, const String &b) { return a.eq(b); } };
  template<> struct FindSpecialisation<SharedString>                              { static inline bool eq(const String &a, const String &b) { return a.eq(b); } };
}
template<typename T>
inline size_t Slice<T>::findFirst(const typename Slice<T>::ET &c) const
{
  size_t offset = 0;
  while (offset < length && !internal::FindSpecialisation<typename Slice<T>::ET>::eq(ptr[offset], c))
    ++offset;
  return offset;
}
template<typename T>
inline size_t Slice<T>::findLast(const typename Slice<T>::ET &c) const
{
  ptrdiff_t last = length-1;
  while (last >= 0 && !internal::FindSpecialisation<typename Slice<T>::ET>::eq(ptr[last], c))
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
      if (!internal::FindSpecialisation<typename Slice<T>::ET>::eq(ptr[i + j], s.ptr[j]))
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
      if (!internal::FindSpecialisation<typename Slice<T>::ET>::eq(ptr[i + j], s.ptr[j]))
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
  size_t offset = findFirst(c);
  if (offset != this->length)
    offset += bInclusive ? 1 : 0;
  return Slice<T>(this->ptr, offset);
}
template<typename T>
inline Slice<T> Slice<T>::getLeftAtLast(const typename Slice<T>::ET &c, bool bInclusive) const
{
  size_t offset = findLast(c);
  if (offset != this->length)
    offset += bInclusive ? 1 : 0;
  return Slice<T>(this->ptr, offset);
}
template<typename T>
inline Slice<T> Slice<T>::getRightAtFirst(const typename Slice<T>::ET &c, bool bInclusive) const
{
  size_t offset = findFirst(c);
  if (offset != this->length)
    offset += bInclusive ? 0 : 1;
  return Slice<T>(this->ptr + offset, length - offset);
}
template<typename T>
inline Slice<T> Slice<T>::getRightAtLast(const typename Slice<T>::ET &c, bool bInclusive) const
{
  size_t offset = findLast(c);
  if (offset != this->length)
    offset += bInclusive ? 0 : 1;
  return Slice<T>(this->ptr + offset, length - offset);
}

template<typename T>
template<typename U>
inline Slice<T> Slice<T>::getLeftAtFirst(Slice<U> s, bool bInclusive) const
{
  size_t offset = findFirst(s);
  if (offset != this->length)
    offset += bInclusive ? s.length : 0;
  return Slice<T>(this->ptr, offset);
}
template<typename T>
template<typename U>
inline Slice<T> Slice<T>::getLeftAtLast(Slice<U> s, bool bInclusive) const
{
  size_t offset = findLast(s);
  if (offset != this->length)
    offset += bInclusive ? s.length : 0;
  return Slice<T>(this->ptr, offset);
}
template<typename T>
template<typename U>
inline Slice<T> Slice<T>::getRightAtFirst(Slice<U> s, bool bInclusive) const
{
  size_t offset = findFirst(s);
  if (offset != this->length)
    offset += bInclusive ? 0 : s.length;
  return Slice<T>(this->ptr + offset, length - offset);
}
template<typename T>
template<typename U>
inline Slice<T> Slice<T>::getRightAtLast(Slice<U> s, bool bInclusive) const
{
  size_t offset = findLast(s);
  if (offset != this->length)
    offset += bInclusive ? 0 : s.length;
  return Slice<T>(this->ptr + offset, length - offset);
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
      while (offset < length && !delimiters.exists(ptr[offset]))
        ++offset;
      tokens[numTokens++] = slice(tokStart, offset);
      ++offset;
    }
    else
    {
      while (offset < length && delimiters.exists(ptr[offset]))
        ++offset;
      if (offset == length)
        break;
      size_t tokStart = offset;
      while (offset < length && !delimiters.exists(ptr[offset]))
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
template<bool skipEmptyTokens>
inline size_t Slice<T>::tokenise(std::function<void(Slice<T> token, size_t index)> onToken, Slice<T> delimiters)
{
  size_t numTokens = 0;
  size_t offset = 0;
  while (offset < length)
  {
    if (!skipEmptyTokens)
    {
      size_t tokStart = offset;
      while (offset < length && !delimiters.exists(ptr[offset]))
        ++offset;
      onToken(slice(tokStart, offset++), numTokens++);
    }
    else
    {
      while (offset < length && delimiters.exists(ptr[offset]))
        ++offset;
      if (offset == length)
        break;
      size_t tokStart = offset;
      while (offset < length && !delimiters.exists(ptr[offset]))
        ++offset;
      onToken(slice(tokStart, offset), numTokens++);
    }
  }
  return numTokens;
}

template<typename T>
template<typename U>
inline size_t Slice<T>::copyTo(Slice<U> dest) const
{
  EPASSERT_THROW(dest.length >= length, Result::OutOfBounds, "Not enough elements!");
  for (size_t i = 0; i<length; ++i)
    dest.ptr[i] = ptr[i];
  return length;
}


// Array
template <typename T, size_t Count>
inline Array<T, Count>::Array()
{}

template <typename T, size_t Count>
inline Array<T, Count>::Array(nullptr_t)
{}

template <typename T, size_t Count>
inline Array<T, Count>::Array(Alloc_T, size_t count)
{
  alloc(count);
}
template <typename T, size_t Count>
inline Array<T, Count>::Array(Reserve_T, size_t count)
{
  reserve(count);
}
template <typename T, size_t Count>
template <typename... Items>
inline Array<T, Count>::Array(Concat_T, Items&&... items)
{
  this->length = internal::count<T>(std::forward<Items>(items)...);
  this->ptr = internal::append<T>(internal::SliceAlloc<T>(this->length), std::forward<Items>(items)...);
}

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
        epConstruct((void*)&this->ptr[i]) T(std::move(rval.ptr[i]));
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
      epConstruct((void*)&this->ptr[i]) T(ptr[i]);
  }
}

template <typename T, size_t Count>
template <typename U>
inline Array<T, Count>::Array(Slice<U> slice)
  : Array<T, Count>(slice.ptr, slice.length)
{}

template <typename T, size_t Count>
template <typename U, size_t N>
inline Array<T, Count>::Array(U(&arr)[N])
  : Array<T, Count>(arr, N)
{}


template <typename T, size_t Count>
inline Array<T, Count>::~Array()
{
  if (!this->ptr)
    return;
  for (size_t i = 0; i < this->length; ++i)
    this->ptr[i].~T();
  if (hasAllocation())
    internal::SliceFree(this->ptr);
}

template <typename T, size_t Count>
inline void Array<T, Count>::reserve(size_t count)
{
  bool hasAlloc = hasAllocation();
  if (!hasAlloc && count <= Count)
    this->ptr = buffer.ptr();
  else if (count > Count)
  {
    bool needsExtend = hasAlloc && internal::GetSliceHeader(this->ptr)->allocatedCount < count;
    if (!hasAlloc || needsExtend)
    {
      T *pNew = internal::SliceAlloc<T>(count);
      if (std::is_pod<T>::value)
        memcpy((void*)pNew, this->ptr, sizeof(T)*this->length);
      else
      {
        for (size_t i = 0; i < this->length; ++i)
        {
          epConstruct((void*)&(pNew[i])) T(std::move(this->ptr[i]));
          this->ptr[i].~T();
        }
      }
      if (hasAlloc)
        internal::SliceFree<T>(this->ptr);
      this->ptr = pNew;
    }
  }
}
template <typename T, size_t Count>
inline void Array<T, Count>::alloc(size_t count)
{
  clear();
  reserve(count);
  for (size_t i = 0; i < count; ++i)
    epConstruct(&this->ptr[i]) T();
  this->length = count;
}
template <typename T, size_t Count>
inline void Array<T, Count>::resize(size_t count)
{
  if (count < this->length)
  {
    for (size_t i = count; i < this->length; ++i)
      this->ptr[i].~T();
  }
  else if (this->length < count)
  {
    reserve(count);
    for (size_t i = this->length; i < count; ++i)
      epConstruct(&this->ptr[i]) T();
  }
  this->length = count;
}
template<typename T, size_t Count>
inline void Array<T, Count>::clear()
{
  for (size_t i = 0; i < this->length; ++i)
    this->ptr[i].~T();
  this->length = 0;
}

template <typename T, size_t Count>
inline Slice<T> Array<T, Count>::getBuffer() const
{
  if (hasAllocation())
    return Slice<T>(this->ptr, internal::GetSliceHeader(this->ptr)->allocatedCount);
  return Slice<T>(buffer.ptr(), Count);
}

template <typename T, size_t Count>
inline Array<T, Count>& Array<T, Count>::operator =(const Array<T, Count> &rh)
{
  if (this != &rh)
  {
    this->~Array();
    epConstruct(this) Array<T, Count>(rh);
  }
  return *this;
}
template <typename T, size_t Count>
inline Array<T, Count>& Array<T, Count>::operator =(Array<T, Count> &&rval)
{
  if (this != &rval)
  {
    this->~Array();
    epConstruct(this) Array<T, Count>(std::move(rval));
  }
  return *this;
}

template <typename T, size_t Count>
template <typename U>
inline Array<T, Count>& Array<T, Count>::operator =(Slice<U> rh)
{
  this->~Array();
  epConstruct(this) Array<T, Count>(rh.ptr, rh.length);
  return *this;
}

template <typename T, size_t Count>
template <typename U>
inline Array<T, Count>& Array<T, Count>::pushBack(U &&item)
{
  reserve(this->length + 1);
  epConstruct((void*)&(this->ptr[this->length++])) T(std::forward<U>(item));
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
    epConstruct((void*)&this->ptr[i-1]) T(std::move(this->ptr[i]));
  }
  this->ptr[--this->length].~T();
  return copy;
}
template <typename T, size_t Count>
T Array<T, Count>::popBack()
{
  --this->length;
  T copy(std::move(this->ptr[this->length]));
  this->ptr[this->length].~T();
  return copy;
}

template <typename T, size_t Count>
inline T& Array<T, Count>::pushBack()
{
  reserve(this->length + 1);
  epConstruct((void*)&(this->ptr[this->length])) T();
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


// SharedArray
template <typename T>
inline SharedArray<T>::SharedArray()
{}

template<typename T>
inline SharedArray<T>::SharedArray(nullptr_t)
{}

template<typename T>
template <typename... Items>
inline SharedArray<T>::SharedArray(Concat_T, Items&&... items)
{
  this->length = internal::count<T>(std::forward<Items>(items)...);
  this->ptr = internal::append<T>(internal::SliceAlloc<T>(this->length), std::forward<Items>(items)...);
}

template<typename T>
inline SharedArray<T>::SharedArray(std::initializer_list<typename SharedArray<T>::ET> list)
  : SharedArray<T>(list.begin(), list.size())
{}

template <typename T>
inline SharedArray<T>::SharedArray(const SharedArray<T> &rcslice)
  : Slice<T>(rcslice)
{
  if (this->ptr)
    ++internal::GetSliceHeader(this->ptr)->refCount;
}
template <typename T>
inline SharedArray<T>::SharedArray(SharedArray<T> &&rval)
  : Slice<T>(rval)
{
  rval.ptr = nullptr;
}

template <typename T>
template <typename U, size_t Len>
inline SharedArray<T>::SharedArray(const Array<U, Len> &arr)
  : SharedArray<T>(Slice<T>(arr))
{}
template <typename T>
template <typename U, size_t Len>
inline SharedArray<T>::SharedArray(Array<U, Len> &&rval)
{
  if (rval.hasAllocation())
  {
    // if the rvalue has an allocation, we can just claim it
    this->ptr = rval.ptr;
    this->length = rval.length;
    rval.ptr = nullptr;
  }
  else
    epConstruct(this) SharedArray<T>(Slice<T>(rval));
}

template <typename T>
template <typename U>
inline SharedArray<T>::SharedArray(U *ptr, size_t length)
  : Slice<T>(length ? internal::SliceAlloc<T>(length) : nullptr, length)
{
  // copy the data
//  if (std::is_pod<T>::value) // TODO: this is only valid if T and U are the same!
//    memcpy((void*)this->ptr, ptr, sizeof(T)*length);
//  else
  {
    for (size_t i = 0; i < length; ++i)
      epConstruct((void*)&this->ptr[i]) T(ptr[i]);
  }
}

template <typename T>
template <typename U>
inline SharedArray<T>::SharedArray(Slice<U> slice)
  : SharedArray<T>(slice.ptr, slice.length)
{}

template <typename T>
template <typename U, size_t N>
inline SharedArray<T>::SharedArray(U(&arr)[N])
  : SharedArray<T>(arr, N)
{}

template <typename T>
inline SharedArray<T>::~SharedArray()
{
  if (!this->ptr)
    return;
  internal::SliceHeader *pH = internal::GetSliceHeader(this->ptr);
  if (pH->refCount == 1)
    destroy();
  else
    --pH->refCount;
}

template <typename T>
size_t inline SharedArray<T>::refcount() const
{
  return this->ptr ? internal::GetSliceHeader(this->ptr)->refCount : 0;
}

template <typename T>
inline void SharedArray<T>::destroy()
{
  for (size_t i = 0; i < this->length; ++i)
    this->ptr[i].~T();
  internal::SliceFree(this->ptr);
}

template <typename T>
inline SharedArray<T>& SharedArray<T>::operator =(const SharedArray<T> &rh)
{
  if (this->ptr != rh.ptr)
  {
    this->~SharedArray();
    epConstruct(this) SharedArray<T>(rh);
  }
  return *this;
}

template <typename T>
inline SharedArray<T>& SharedArray<T>::operator =(SharedArray<T> &&rval)
{
  if (this != &rval)
  {
    this->~SharedArray();
    epConstruct(this) SharedArray<T>(std::move(rval));
  }
  return *this;
}

template <typename T>
template <typename U>
inline SharedArray<T>& SharedArray<T>::operator =(Slice<U> rh)
{
  if (this->ptr != rh.ptr)
    *this = SharedArray(rh);
  return *this;
}


/**** concatenation code ****/

// TODO: support concatenating compatible Slice<T>'s

template<typename T, size_t Count>
template<typename... Things>
inline Array<T, Count>& Array<T, Count>::concat(Things&&... things)
{
  size_t len = this->length + internal::count<T>(std::forward<Things>(things)...);
  reserve(len);
  internal::append<T>(this->ptr + this->length, std::forward<Things>(things)...);
  this->length = len;
  return *this;
}

template<typename T>
template<typename... Things>
inline SharedArray<T> SharedArray<T>::concat(Things&&... things)
{
  return SharedArray<T>(Concat, std::forward<Things>(things)...);
}

namespace internal {

  template<typename T>
  struct is_slice
  {
    enum { value = 0 };
  };
  template<typename T>
  struct is_slice<Slice<T>>
  {
    enum { value = 1 };
  };
  template<typename T>
  struct is_slice<const Slice<T>>
  {
    enum { value = 1 };
  };

} // internal

template <typename Dst, typename Src>
inline Dst slice_cast(Src src)
{
  static_assert(internal::is_slice<Src>::value && internal::is_slice<Dst>::value, "Dst and/or Src is not a Slice");
  EPASSERT_THROW(src.length * sizeof(typename Src::ET) % sizeof(typename Dst::ET) == 0, Result::OutOfBounds, "The destination slice cannot represent the source slice without truncating data");
  return Dst((typename Dst::ET*)src.ptr, src.length * sizeof(typename Src::ET) / sizeof(typename Dst::ET));
}

} // namespace ep
