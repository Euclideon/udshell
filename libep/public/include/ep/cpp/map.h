#pragma once
#if !defined(_EPMAP_HPP)
#define _EPMAP_HPP

#include "ep/cpp/range.h"

namespace ep {

template <typename Tree> struct SharedMap;
template <typename Tree>
ptrdiff_t epStringify(Slice<char> buffer, String format, const SharedMap<Tree> &map, const VarArg *pArgs)
{
  if (!map.ptr)
    return epStringifyTemplate(buffer, format, nullptr, pArgs);
  return epStringify(buffer, format, map.ptr->tree, pArgs);
}


template <typename Tree>
struct SharedMap
{
public:
  using MapType = Tree;
  using KeyType = typename Tree::KeyType;
  using ValueType = typename Tree::ValueType;
  using KeyValuePair = typename Tree::KeyValuePair;
  using Iterator = typename Tree::Iterator;

  SharedMap() {}
  SharedMap(nullptr_t) : ptr(nullptr) {}
  SharedMap(const SharedMap &rh)
    : ptr(rh.ptr)
  {}
  SharedMap(SharedMap &&rval)
    : ptr(std::move(rval.ptr))
  {}
//  SharedMap(const Tree &map) // TODO: consider; do we want deep-copying constructors?
//  {
//    alloc();
//    for (auto &&kvp : map)
//      ptr->tree.insert(kvp.key, kvp.value);
//  }
  SharedMap(Tree &&map)
  {
    alloc();
    new(&ptr->tree) Tree(std::move(map));
  }
  SharedMap(Slice<const typename Tree::KeyValuePair> arr)
  {
    alloc();
    for (auto &kvp : arr)
      ptr->tree.insert(kvp);
  }
  SharedMap(Slice<const ValueType> arr)
  {
    alloc();
    for (size_t i = 0; i < arr.length; ++i)
      ptr->tree.insert(i, arr[i]);
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

  size_t size() const { return ptr ? ptr->tree.size() : 0; }

  bool empty() const { return ptr ? ptr->tree.empty() : true; }

  void clear() { if(ptr) ptr->tree.clear(); }

  ValueType& insert(KeyType &&key, ValueType &&rval)
  {
    if (!ptr)
      alloc();
    return ptr->tree.insert(std::move(key), std::move(rval));
  }
  ValueType& insert(const KeyType &key, ValueType &&rval)
  {
    if (!ptr)
      alloc();
    return ptr->tree.insert(key, std::move(rval));
  }
  ValueType& insert(KeyType &&key, const ValueType &v)
  {
    if (!ptr)
      alloc();
    return ptr->tree.insert(std::move(key), v);
  }
  ValueType& insert(const KeyType &key, const ValueType &v)
  {
    if (!ptr)
      alloc();
    return ptr->tree.insert(key, v);
  }

  ValueType& insert(KVP<KeyType, ValueType> &&kvp)
  {
    if (!ptr)
      alloc();
    return ptr->tree.insert(std::move(kvp));
  }
  ValueType& insert(const KVP<KeyType, ValueType> &v)
  {
    if (!ptr)
      alloc();
    return ptr->tree.insert(v);
  }

  template <typename Key>
  ValueType& tryInsert(Key&& key, ValueType&& val)
  {
    if (!ptr)
      alloc();
    return ptr->tree.tryInsert(std::forward<Key>(key), std::move(val));
  }
  template <typename Key>
  ValueType& tryInsert(Key&& key, const ValueType& val)
  {
    if (!ptr)
      alloc();
    return ptr->tree.tryInsert(std::forward<Key>(key), val);
  }
  template <typename Key>
  ValueType& tryInsert(Key&& key, std::function<ValueType()> lazyValue)
  {
    if (!ptr)
      alloc();
    return ptr->tree.tryInsert(std::forward<Key>(key), lazyValue);
  }

  ValueType& replace(KeyType &&key, ValueType &&rval)
  {
    if (!ptr)
      alloc();
    return ptr->tree.replace(std::move(key), std::move(rval));
  }
  ValueType& replace(const KeyType &key, ValueType &&rval)
  {
    if (!ptr)
      alloc();
    return ptr->tree.replace(key, std::move(rval));
  }
  ValueType& replace(KeyType &&key, const ValueType &v)
  {
    if (!ptr)
      alloc();
    return ptr->tree.replace(std::move(key), v);
  }
  ValueType& replace(const KeyType &key, const ValueType &v)
  {
    if (!ptr)
      alloc();
    return ptr->tree.replace(key, v);
  }

  ValueType& replace(KVP<KeyType, ValueType> &&kvp)
  {
    if (!ptr)
      alloc();
    return ptr->tree.replace(std::move(kvp));
  }
  ValueType& replace(const KVP<KeyType, ValueType> &v)
  {
    if (!ptr)
      alloc();
    return ptr->tree.replace(v);
  }

  void remove(const KeyType &key)
  {
    if (ptr)
      ptr->tree.remove(key);
  }

  const ValueType* get(const KeyType &key) const
  {
    return ptr ? ptr->tree.get(key) : nullptr;
  }
  ValueType* get(const KeyType &key)
  {
    return ptr ? ptr->tree.get(key) : nullptr;
  }

  const ValueType& operator[](const KeyType &key) const
  {
    const ValueType *pV = get(key);
    EPASSERT_THROW(pV, Result::OutOfBounds, "Element not found: {0}", key);
    return *pV;
  }
  ValueType& operator[](const KeyType &key)
  {
    ValueType *pV = get(key);
    EPASSERT_THROW(pV, Result::OutOfBounds, "Element not found: {0}", key);
    return *pV;
  }
  const ValueType& at(const KeyType &key) const { return operator[](key); }
  ValueType& at(const KeyType &key) { return operator[](key); }

  TreeRange<Tree> getRange() const { return ptr ? TreeRange<Tree>(ptr->tree) : TreeRange<Tree>(); }

  Iterator begin() const { return ptr ? ptr->tree.begin() : ptr->tree.end(); }
  Iterator end() const { return ptr->tree.end(); }

private:
  friend struct Variant;
  friend ptrdiff_t epStringify<Tree>(Slice<char>, String, const SharedMap<Tree>&, const VarArg*);

  struct Node : public RefCounted
  {
    Tree tree;
  };

  SharedPtr<Node> ptr;

  void alloc()
  {
    ptr = SharedPtr<Node>::create();
  }
};

} // namespace ep

#endif // _EPMAP_HPP
