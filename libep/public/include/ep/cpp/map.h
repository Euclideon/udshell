#pragma once
#if !defined(_EPMAP_HPP)
#define _EPMAP_HPP

#include "ep/cpp/avltree.h"

namespace ep {

template <typename Tree>
struct SharedMap
{
public:
  using KeyType = typename Tree::KeyType;
  using ValueType = typename Tree::ValueType;
  using Iterator = typename Tree::Iterator;

  SharedMap() {}
  SharedMap(nullptr_t) : ptr(nullptr) {}
  SharedMap(const SharedMap &rh)
    : ptr(rh.ptr)
  {}
  SharedMap(SharedMap &&rval)
    : ptr(std::move(rval.ptr))
  {}
  SharedMap(Slice<const typename Tree::KeyValuePair> arr)
  {
    Alloc();
    for (auto &kvp : arr)
      ptr->tree.Insert(kvp.key, kvp.value);
  }
  SharedMap(std::initializer_list<typename Tree::KeyValuePair> init)
    : SharedMap(Slice<const typename Tree::KeyValuePair>(init.begin(), init.size()))
  {}

  SharedMap& operator =(const SharedMap &rh)
  {
    ptr = rh.ptr;
    return *this;
  }
  SharedMap& operator =(SharedMap &&rh)
  {
    ptr = std::move(rh.ptr);
    return *this;
  }

  size_t Size() const { return ptr ? ptr->tree.Size() : 0; }

  bool Empty() const { return ptr ? ptr->tree.Empty() : true; }

  void Insert(KeyType &&key, ValueType &&rval)
  {
    if (!ptr)
      Alloc();
    ptr->tree.Insert(std::move(key), std::move(rval));
  }
  void Insert(const KeyType &key, ValueType &&rval)
  {
    if (!ptr)
      Alloc();
    ptr->tree.Insert(key, std::move(rval));
  }
  void Insert(KeyType &&key, const ValueType &v)
  {
    if (!ptr)
      Alloc();
    ptr->tree.Insert(std::move(key), v);
  }
  void Insert(const KeyType &key, const ValueType &v)
  {
    if (!ptr)
      Alloc();
    ptr->tree.Insert(key, v);
  }

  void Insert(KVP<KeyType, ValueType> &&kvp)
  {
    if (!ptr)
      Alloc();
    ptr->tree.Insert(std::move(kvp));
  }
  void Insert(const KVP<KeyType, ValueType> &v)
  {
    if (!ptr)
      Alloc();
    ptr->tree.Insert(v);
  }

  void Remove(const KeyType &key)
  {
    if (ptr)
      ptr->tree.Remove(key);
  }

  const ValueType* Get(const KeyType &key) const
  {
    return ptr ? ptr->tree.Get(key) : nullptr;
  }
  ValueType* Get(const KeyType &key)
  {
    return ptr ? ptr->tree.Get(key) : nullptr;
  }

  const ValueType& operator[](const KeyType &key) const
  {
    const ValueType *pV = Get(key);
    EPASSERT_THROW(pV, epR_OutOfBounds, "Element not found: {0}", key);
    return *pV;
  }
  ValueType& operator[](const KeyType &key)
  {
    ValueType *pV = Get(key);
    EPASSERT_THROW(pV, epR_OutOfBounds, "Element not found: {0}", key);
    return *pV;
  }

  Iterator begin() const { return ptr ? ptr->tree.begin() : ptr->tree.end(); }
  Iterator end() const { return ptr->tree.end(); }

private:
  friend struct Variant;

  struct Node : public RefCounted
  {
    Tree tree;
  };

  SharedPtr<Node> ptr;

  void Alloc()
  {
    ptr = SharedPtr<Node>::create();
  }
};

} // namespace ep

#endif // _EPMAP_HPP
