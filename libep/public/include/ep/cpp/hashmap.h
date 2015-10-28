#if !defined(_EP_HASHMAP_HPP)
#define _EP_HASHMAP_HPP

#include "ep/cpp/chunkedarray.h"

//#define SUPPORT_FLEXIBLE_TABLE_SIZE

namespace ep {

// TODO: this hash map sucks! it's not even C++11, and the key type should also be given i reckon

template <typename T>
class HashMap
{
private:
  struct Item
  {
    Item() {}
    Item(const T &item) : item(item) {}
    Item(T &&item) : item(std::move(item)) {}

    T item;
    size_t hash;
    Item *pNext;
  };

public:
  class Iterator
  {
    friend class HashMap;
  public:
    Iterator(const HashMap<T> &hashMap, Item* pItem) : hashMap(hashMap), pItem(pItem) {}
    bool operator!=(Iterator rh) const { return pItem != rh.pItem; }      // compare
    Iterator& operator++() { pItem = hashMap.Next(*this).pItem; return *this; } // increment
    T& operator*() const { return pItem->item; }                          // value

  private:
    const HashMap &hashMap;
    Item* pItem;

    Iterator& operator=(const Iterator &rh)
    {
      new(this) Iterator(rh.hashMap, pItem);
      return *this;
    }
  };

  HashMap(size_t _tableSize);
  ~HashMap();

  T& operator[](const char *pKey) const   { return *Get(HashString(pKey)); }

  T* Get(size_t hash) const;
  T* Get(const char *pKey) const          { return Get(HashString(pKey)); }

  T& Create(size_t hash);
  T& Create(const char *pKey)             { return Create(HashString(pKey)); }

  T& Add(size_t hash, const T& item);
  T& Add(const char *pKey, const T& item) { return Add(HashString(pKey), item); }
#if EP_CPP11
  T& Add(size_t hash, T&& item);
  T& Add(const char *pKey, T&& item)      { return Add(HashString(pKey), std::move(item)); }
#endif

  void Destroy(size_t hash);
  void Destroy(const char *pKey)          { Destroy(HashString(pKey)); }

  bool Destroy(const T& item);
  bool DestroyItem(const T& item);  // this will only destroy an item that was allocated by this hash table

  Iterator First() const;
  Iterator Next(Iterator i) const;

  Iterator NextMatch(Iterator i) const;

  int GetNumItems() const { return itemCount; }

  Iterator begin() const  { return First(); }
  Iterator end() const;

protected:
  ChunkedArray<Item, 256> itemPool;
  Item **ppItems;
  size_t tableSize;
  size_t itemCount;

  inline size_t GetTableIndex(size_t hash) const
  {
#if !defined(SUPPORT_FLEXIBLE_TABLE_SIZE)
    // 8 bit hash folding... we'll just fold the bottom 16 bits :/
    return ((hash >> 8) ^ hash) & (0xFF);
#else
    return hash % tableSize;
#endif
  }

  inline uint32_t HashString(const char *pString, uint32_t hash = 0) const
  {
    unsigned char *s = (unsigned char *)pString;
    while (*s)
    {
      hash ^= (uint32_t)*s++;
      hash *= 0x01000193;
    }
    return hash;
  }
};

inline uint32_t HashPointer(void *pPtr)
{
  size_t r = (size_t)pPtr;
  if (sizeof(size_t) > 4)
    r = (uint32_t)(r & 0xFFFFFFFF) ^ (uint32_t)(r >> 32);
  return (uint32_t)r;
}

} // namespace ep

#include "internal/hashmap_inl.h"

#endif // _EP_HASHMAP_HPP
