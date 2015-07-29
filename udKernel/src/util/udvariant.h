#pragma once
#if !defined(UDVARIANT_H)
#define UDVARIANT_H

#include "util/udstring.h"
#include "util/uddelegate.h"

namespace ud
{
  SHARED_CLASS(Component);
  SHARED_CLASS(Resource);
  class LuaState;
};

struct udKeyValuePair;

struct udVariant
{
public:
  typedef udDelegate<udVariant(udSlice<udVariant>)> Delegate;

  enum class Type
  {
    Null,
    Bool,
    Int,
    Float,
    Component,
    Resource,
    Delegate,
    String,
    Array,
    AssocArray
  };

  udVariant();
  udVariant(udVariant &&rval);
  udVariant(const udVariant &rval);

  udVariant(bool);
  udVariant(int64_t);
  udVariant(double);
  udVariant(ud::Component *);
  udVariant(ud::Resource *);
  udVariant(const Delegate &d);
  udVariant(Delegate &&d);
  udVariant(udString);
  udVariant(udSlice<udVariant> a, bool ownsMemory = false);
  udVariant(udSlice<udKeyValuePair> aa, bool ownsMemory = false);

  template<typename T> udVariant(const T &v);
  template<typename T> udVariant(T &v);

  ~udVariant();

  // assignment operators
  udVariant& operator=(udVariant &&rval);
  udVariant& operator=(const udVariant &rval);

  // actual methods
  Type type() const;
  bool is(Type type) const;

  udRCString stringify() const;

  template<typename T>
  T as() const;

  bool asBool() const;
  int64_t asInt() const;
  double asFloat() const;
  ud::ComponentRef asComponent() const;
  ud::ResourceRef asResource() const;
  Delegate asDelegate() const;
  udString asString() const;
  udSlice<udVariant> asArray() const;
  udSlice<udKeyValuePair> asAssocArray() const;
  udSlice<udKeyValuePair> asAssocArraySeries() const;

  size_t arrayLen() const;
  size_t assocArraySeriesLen() const;

  udVariant operator[](size_t i) const;
  udVariant operator[](udString key) const;

  udVariant* allocArray(size_t len);
  udKeyValuePair* allocAssocArray(size_t len);

  void luaPush(ud::LuaState &l) const;
  static udVariant luaGet(ud::LuaState &l, int idx = -1);

private:
  size_t t : 4;
  size_t ownsArray : 1;
  size_t length : (sizeof(size_t)*8)-5;
  union
  {
    bool b;
    int64_t i;
    double f;
    ud::Component *c;
    ud::Resource *r;
    const char *s;
    udVariant *a;
    udKeyValuePair *aa;
    void *p;
  };
};

struct udKeyValuePair
{
  udKeyValuePair() {}
  udKeyValuePair(udKeyValuePair &&val) : key(std::move(val.key)), value(std::move(val.value)) {}
  udKeyValuePair(const udKeyValuePair &val) : key(val.key), value(val.value) {}

  udKeyValuePair(const udVariant &key, const udVariant &value) : key(key), value(value) {}
  udKeyValuePair(const udVariant &key, udVariant &&value) : key(key), value(std::move(value)) {}
  udKeyValuePair(udVariant &&key, const udVariant &value) : key(std::move(key)), value(value) {}
  udKeyValuePair(udVariant &&key, udVariant &&value) : key(std::move(key)), value(std::move(value)) {}

  udVariant key;
  udVariant value;
};

typedef udSlice<udKeyValuePair> InitParams;


#include "util/udvariant.inl"

#endif // UDVARIANT_H
