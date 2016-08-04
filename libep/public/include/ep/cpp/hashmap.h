#if !defined(_EP_HASHMAP_HPP)
#define _EP_HASHMAP_HPP

#include "ep/cpp/platform.h"
#include "ep/cpp/keyvaluepair.h"
#include "ep/cpp/freelist.h"
#include "ep/cpp/delegate.h"

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
//! \cond
template<>
struct Hash<SharedString>
{
  static uint32_t hash(const SharedString &v) { return v.hash(); }
  static bool eq(const SharedString &a, const SharedString &b) { return a.eq(b); }
};
//! \endcond


template <typename K, typename V, typename HashPred = Hash<K>>
class HashMap
{
public:
  using KeyType = K;
  using ValueType = V;
  using KeyValuePair = KVP<K, V>;

  enum { DefaultTableSize = 0x2000 };
  enum { DefaultItemBucketSize = 0x100 };

  HashMap(size_t tableSize = DefaultTableSize, size_t itemBucketSize = DefaultItemBucketSize);
  HashMap(HashMap &&rval);

  HashMap(Slice<const KeyValuePair> arr, size_t tableSize = DefaultTableSize, size_t itemBucketSize = DefaultItemBucketSize)
    : HashMap(tableSize, itemBucketSize)
  {
    for (auto &kvp : arr)
      insert(kvp);
  }

  HashMap(std::initializer_list<KeyValuePair> init, size_t tableSize = DefaultTableSize, size_t itemBucketSize = DefaultItemBucketSize)
    : HashMap(Slice<const KeyValuePair>(init.begin(), init.size()), tableSize, itemBucketSize)
  {}

  ~HashMap();

  size_t size() const;
  bool empty() const;

  void clear();

  V& insert(const K &key, const V &val);
  V& insert(const K &key, V &&val);
  V& insert(K &&key, const V &val);
  V& insert(K &&key, V &&val);

  V& insert(KVP<K, V> &&v);
  V& insert(const KVP<K, V> &v);

  V& tryInsert(const K &key, const V &val);
  V& tryInsert(const K &key, V &&val);
  V& tryInsert(K &&key, const V &val);
  V& tryInsert(K &&key, V &&val);

  V& tryInsert(KVP<K, V> &&v);
  V& tryInsert(const KVP<K, V> &v);

  V& tryInsert(const K &key, Delegate<V()> lazyValue);
  V& tryInsert(K &&key, Delegate<V()> lazyValue);

  V& replace(const K &key, const V &val);
  V& replace(const K &key, V &&val);
  V& replace(K &&key, const V &val);
  V& replace(K&& key, V&& val);

  V& replace(const KVP<K, V> &v);
  V& replace(KVP<K, V> &&v);

  void remove(const K &key);

  const V* get(const K &key) const;
  V* get(const K &key);

  const V& operator[](const K &key) const;
  V& operator[](const K &key);
  const V& at(const K &key) const { return operator[](key); }
  V& at(const K &key) { return operator[](key); }

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

    const K& key() const;
    const V& value() const;
    V& value();

    KVPRef<const K, const V> operator*() const;
    KVPRef<const K, V> operator*();
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
