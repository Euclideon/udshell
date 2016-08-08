#pragma once
#ifndef _EP_METADATA_H
#define _EP_METADATA_H

#include "ep/cpp/internal/i/imetadata.h"
#include "ep/cpp/component/resource/kvpstore.h"

namespace ep {

SHARED_CLASS(Metadata);

class Metadata : public KVPStore
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Metadata, IMetadata, KVPStore, EPKERNEL_PLUGINVERSION, "Metadata resource", 0)

public:
  virtual size_t numRecords() const { return pImpl->NumRecords(); }

  virtual void insert(Variant &&key, Variant &&value) { pImpl->InsertRR(std::move(key), std::move(value)); }
  virtual void insert(const Variant &key, Variant &&value) { pImpl->InsertLR(key, std::move(value)); }
  virtual void insert(Variant &&key, const Variant &value) { pImpl->InsertRL(std::move(key), value); }
  virtual void insert(const Variant &key, const Variant &value) { pImpl->InsertLL(key, value); }

  void insertAt(const Variant &value, Slice<const Variant> keys)
  {
    if (keys.length == 1)
      insert(keys[0], value);
    else
    {
      Variant item = get(keys[0]);
      if (item.is(Variant::SharedPtrType::AssocArray))
      {
        Variant::VarMap map = item.asAssocArray();
        Variant *i = map.get(keys[1]);
        if (i)
        {
          insertAtImpl(*i, value, keys.slice(2, keys.length));
        }
        else
        {
          Variant::VarMap::MapType m = map.clone();
          m.insert(keys[1], makeTree(value, keys.slice(2, keys.length)));
          insert(keys[0], std::move(m));
        }
      }
      else if (item.is(Variant::Type::Array))
      {
        Variant::VarArray arr = item.asSharedArray();
        size_t i = (size_t)-1;
        if(keys[1].is(Variant::Type::Int))
          i = keys[1].as<size_t>();
        if (i < arr.length)
        {
          insertAtImpl(arr[i], value, keys.slice(2, keys.length));
        }
        else if (i == arr.length)
        {
          // array extended by one...
          Array<Variant> a = arr.clone();
          a.push_back(makeTree(value, keys.slice(2, keys.length)));
          insert(keys[0], std::move(a));
        }
        else
        {
          // array has become sparse... promote to a map!
          Variant::VarMap::MapType map;
          for (size_t j = 0; j < arr.length; ++j)
            map.insert(j, arr[j]);
          map.insert(keys[1], makeTree(value, keys.slice(2, keys.length)));
          insert(keys[0], std::move(map));
        }
      }
      else
        insert(keys[0], makeTree(value, keys.slice(1, keys.length)));
    }
  }
  template <typename Val, typename... Keys>
  void insertAt(Val &&value, Keys&&... keys)
  {
    insertAt((const Variant&)Variant(std::forward<Val>(value)), Slice<const Variant>{ std::forward<Keys>(keys)... });
  }

  virtual void remove(const Variant &key) { pImpl->Remove(key); }
  virtual bool exists(const Variant &key) const { return pImpl->Exists(key); }
  virtual Variant get(const Variant &key) const { return pImpl->Get(key); }

protected:
  Metadata(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : KVPStore(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;

private:
  void insertMethod(const Variant &key, const Variant &value) { insert(key, value); }

  Variant makeTree(const Variant &value, Slice<const Variant> keys)
  {
    if (!keys)
      return value;
    if (keys[0].is(Variant::Type::Int) && keys[0].asInt() == 0)
    {
      Array<Variant> arr(Reserve, 1);
      arr.push_back(makeTree(value, keys.slice(1, keys.length)));
      return std::move(arr);
    }
    else
    {
      Variant::VarMap::MapType map;
      map.insert(keys[0], makeTree(value, keys.slice(1, keys.length)));
      return std::move(map);
    }
  }

  void insertAtImpl(Variant &owner, const Variant &value, Slice<const Variant> keys)
  {
    if (!keys)
    {
      owner = value;
      return;
    }

    Variant *item = owner.getItem(keys[0]);
    if (item)
    {
      insertAtImpl(*item, value, keys.slice(1, keys.length));
      return;
    }

    if (owner.is(Variant::SharedPtrType::AssocArray))
    {
      Variant::VarMap::MapType map = owner.asAssocArray().clone();
      map.insert(keys[0], makeTree(value, keys.slice(1, keys.length)));
      owner = std::move(map);
    }
    else if (owner.is(Variant::Type::Array))
    {
      Variant::VarArray arr = owner.asSharedArray();
      size_t i = (size_t)-1;
      if (keys[1].is(Variant::Type::Int))
        i = keys[1].as<size_t>();
      if (i == arr.length)
      {
        // array extended by one...
        Array<Variant> a = arr.clone();
        a.push_back(makeTree(value, keys.slice(1, keys.length)));
        owner = std::move(a);
      }
      else
      {
        // array has become sparse... promote to a map!
        Variant::VarMap::MapType map;
        for (size_t j = 0; j < arr.length; ++j)
          map.insert(j, arr[j]);
        map.insert(keys[1], makeTree(value, keys.slice(1, keys.length)));
        owner = std::move(map);
      }
    }
    else
      owner = makeTree(value, keys);
  }
};

}

#endif // _EP_METADATA_H
