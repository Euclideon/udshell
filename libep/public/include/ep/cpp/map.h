#pragma once
#if !defined(_EPMAP_HPP)
#define _EPMAP_HPP

#include "ep/cpp/range.h"
#include "ep/cpp/delegate.h"
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
  SharedMap(Tree &&map)
  {
    alloc();
    new(&ptr->tree) Tree(std::move(map));
  }
  SharedMap(Slice<const KeyValuePair> arr)
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
  SharedMap(std::initializer_list<KeyValuePair> init)
    : SharedMap(Slice<const KeyValuePair>(init.begin(), init.size()))
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

  size_t use_count() const { return ptr.use_count(); }
  size_t incRef() { return ptr.incRef(); }
  size_t decRef() { return ptr.decRef(); }
  bool unique() const { return ptr.unique(); }

  bool operator == (const SharedMap &rh) const { return ptr == rh.ptr; }
  bool operator != (const SharedMap &rh) const { return ptr != rh.ptr; }

  explicit operator bool() const { return ptr.operator bool(); }

  MapType claim()
  {
    EPTHROW_IF(!unique(), Result::InvalidCall, "SharedMap must be unique!");
    return std::move(ptr->tree);
  }
  MapType clone()
  {
    if(ptr)
      return MapType(ptr->tree);
    return MapType();
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

  bool exists(const KeyType &key)
  {
    return ptr ? ptr->tree.exists(key) : false;
  }

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

// Range retrieval
template <typename Tree>
TreeRange<SharedMap<Tree>> range(const SharedMap<Tree> &input) { return input.getRange(); }

} // namespace ep

#endif // _EPMAP_HPP
