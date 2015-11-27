#pragma once
#if !defined(_EPMAP_HPP)
#define _EPMAP_HPP

#include "ep/cpp/avltree.h"
#include "ep/cpp/sharedptr.h"

namespace ep {

template <typename Tree>
class SharedMap : public RefCounted
{
public:
  using KeyType = typename Tree::KeyType;
  using ValueType = typename Tree::ValueType;
  using Iterator = typename Tree::Iterator;

  size_t Size() const { return tree.Size(); }

  bool Empty() const { return tree.Empty(); }

  void Insert(KeyType &&key, ValueType &&rval)
  {
    tree.Insert(std::move(key), std::move(rval));
  }
  void Insert(const KeyType &key, ValueType &&rval)
  {
    tree.Insert(key, std::move(rval));
  }
  void Insert(KeyType &&key, const ValueType &v)
  {
    tree.Insert(std::move(key), v);
  }
  void Insert(const KeyType &key, const ValueType &v)
  {
    tree.Insert(key, v);
  }

  void Remove(const KeyType &key)
  {
    tree.Remove(key);
  }

  const ValueType* Get(const KeyType &key) const
  {
    return tree.Get(key);
  }
  ValueType* Get(const KeyType &key)
  {
    return tree.Get(key);
  }

  ValueType& operator[](const KeyType &key) const
  {
    const ValueType *v = Get(key);
    EPASSERT(v, "Invalid index");
    return *(ValueType*)v; // TODO: should we propagate const from Map to element?
  }

  Iterator begin() const { return tree.begin(); }
  Iterator end() const { return tree.end(); }

private:
  friend struct Variant;

  Tree tree;
};

template <typename Tree>
using SharedMapRef = SharedPtr<SharedMap<Tree>>;

} // namespace ep

#endif // _EPMAP_HPP
