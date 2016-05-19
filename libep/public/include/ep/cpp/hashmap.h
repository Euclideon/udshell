#if !defined(_EP_HASHMAP_HPP)
#define _EP_HASHMAP_HPP

#include "ep/cpp/platform.h"
#include "ep/cpp/keyvaluepair.h"
#include "ep/cpp/freelist.h"
#include "ep/cpp/string.h"

#include <functional>

namespace ep {

template<typename T>
struct Hash
{
  static uint32_t hash(const T &)
  {
    // TODO: some CRC or something
    return 0;
  }
  static bool eq(const T &a, const T &b) { return a == b; }
};
/// \cond
template<>
struct Hash<SharedString>
{
  static uint32_t hash(const SharedString &v) { return v.hash(); }
  static bool eq(const SharedString &a, const SharedString &b) { return a.eq(b); }
};
/// \endcond


template <typename V, typename K = SharedString, typename HashPred = Hash<K>>
class HashMap
{
public:
  HashMap(size_t tableSize = 0x2000, size_t itemBucketSize = 0x100);
  HashMap(HashMap &&rval);

  ~HashMap();

  size_t size() const;
  bool empty() const;

  void clear();

  template <typename Key, typename Val>
  V& insert(Key&& key, Val&& val);
  V& insert(KVP<K, V> &&v);
  V& insert(const KVP<K, V> &v);

  template <typename Key>
  V& tryInsert(Key&& key, V&& val);
  template <typename Key>
  V& tryInsert(Key&& key, const V& val);
  template <typename Key>
  V& tryInsert(Key&& key, std::function<V()> lazyValue);

  template <typename Key, typename Val>
  V& replace(Key&& key, Val&& val);
  V& replace(KVP<K, V> &&v);
  V& replace(const KVP<K, V> &v);

  void remove(const K &key);

  const V* get(const K &key) const;
  V* get(const K &key);

  const V& operator[](const K &key) const;
  V& operator[](const K &key);

  bool exists(const K &key);

  class Iterator;
  Iterator begin() const;
  Iterator end() const;

  Iterator find(const K &key);
  Iterator erase(Iterator it);

private:
  struct Node
  {
    template <typename... Args>
    Node(Args&&... args) : data(std::forward<Args>(args)...) {}

    KVP<K, V> data;
    Node *pNext;
  };

  Node **ppTable = nullptr;
  size_t tableSizeMask;
  FreeList<Node> pool;

  Node** getBucket(const K &key) const;
  V* getValue(Node *pBucket, const K &key) const;

public:
  class Iterator
  {
  public:
    Iterator(Node **ppStart, Node **ppEnd);

    Iterator &operator++();

    bool operator!=(Iterator rhs) const;
    bool operator==(Iterator rhs) const;

    const KVPRef<K, V> operator*() const;
    KVPRef<K, V> operator*();
    const V* operator->() const;
    V* operator->();

  private:
    friend class HashMap;

    Node **ppStart, **ppEnd;
    Node *pItem;
  };
};


inline uint32_t HashPointer(void *pPtr)
{
#if defined(EP_32BIT)
  return (uint32_t)pPtr;
#else // EP_32BIT
  size_t r = (size_t)pPtr;
  if (sizeof(size_t) > 4)
    r = (uint32_t)(r & 0xFFFFFFFF) ^ (uint32_t)(r >> 32);
  return (uint32_t)r;
#endif // EP_32BIT
}

} // namespace ep

#include "internal/hashmap_inl.h"

#endif // _EP_HASHMAP_HPP
