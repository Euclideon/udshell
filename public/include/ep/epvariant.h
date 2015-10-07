#pragma once
#if !defined(EPVARIANT_H)
#define EPVARIANT_H

#include "ep/epplatform.h"
#include "ep/epenum.h"
#include "ep/epdelegate.h"

namespace ep
{
  SHARED_CLASS(Component);
  SHARED_CLASS(Resource);
  class LuaState;
};

struct epKeyValuePair;

struct epVariant
{
public:
  typedef epDelegate<epVariant(epSlice<epVariant>)> VarDelegate;

  enum class Type
  {
    Null,
    Bool,
    Int,
    Float,
    Enum,
    Bitfield,
    Component,
    Delegate,
    String,
    Array,
    AssocArray
  };

  epVariant();
  epVariant(epVariant &&rval);
  epVariant(const epVariant &val);

  epVariant(bool);
  epVariant(int64_t);
  epVariant(double);
  epVariant(size_t val, const epEnumDesc *pDesc, bool isBitfield);
  epVariant(ep::ComponentRef &&spC);
  epVariant(const ep::ComponentRef &spC);
  epVariant(VarDelegate &&d);
  epVariant(const VarDelegate &d);
  epVariant(epString, bool ownsMemory = false);
  epVariant(epSlice<epVariant> a, bool ownsMemory = false);
  epVariant(epSlice<epKeyValuePair> aa, bool ownsMemory = false);

  template<typename T> epVariant(T &&rval);

  ~epVariant();

  // assignment operators
  epVariant& operator=(epVariant &&rval);
  epVariant& operator=(const epVariant &rval);

  // actual methods
  Type type() const;
  bool is(Type type) const;

  epSharedString stringify() const;

  template<typename T>
  T as() const;

  ptrdiff_t compare(const epVariant &v) const;

  bool asBool() const;
  int64_t asInt() const;
  double asFloat() const;
  const epEnumDesc* asEnum(size_t *pVal) const;
  ep::ComponentRef asComponent() const;
  VarDelegate asDelegate() const;
  epString asString() const;
  epSlice<epVariant> asArray() const;
  epSlice<epKeyValuePair> asAssocArray() const;
  epSlice<epKeyValuePair> asAssocArraySeries() const;

  size_t arrayLen() const;
  size_t assocArraySeriesLen() const;

  epVariant operator[](size_t i) const;
  epVariant operator[](epString key) const;

  epVariant* allocArray(size_t len);
  epKeyValuePair* allocAssocArray(size_t len);

  void luaPush(ep::LuaState &l) const;
  static epVariant luaGet(ep::LuaState &l, int idx = -1);

private:
  size_t t : 4;
  size_t ownsContent : 1;
  size_t length : (sizeof(size_t)*8)-5; // NOTE: if you change this, update the shift's in asEnum()!!!
  union
  {
    bool b;
    int64_t i;
    double f;
    ep::Component *c;
    const char *s;
    epVariant *a;
    epKeyValuePair *aa;
    void *p;
  };
};

struct epKeyValuePair
{
  epKeyValuePair() {}
  epKeyValuePair(epKeyValuePair &&val) : key(std::move(val.key)), value(std::move(val.value)) {}
  epKeyValuePair(const epKeyValuePair &val) : key(val.key), value(val.value) {}

  epKeyValuePair(const epVariant &key, const epVariant &value) : key(key), value(value) {}
  epKeyValuePair(const epVariant &key, epVariant &&value) : key(key), value(std::move(value)) {}
  epKeyValuePair(epVariant &&key, const epVariant &value) : key(std::move(key)), value(value) {}
  epKeyValuePair(epVariant &&key, epVariant &&value) : key(std::move(key)), value(std::move(value)) {}

  epVariant key;
  epVariant value;
};

class epInitParams
{
public:
  epInitParams() {}
  epInitParams(nullptr_t) {}
  epInitParams(const epInitParams& rh) : params(rh.params) {}
  epInitParams(epSlice<const epKeyValuePair> kvp) : params(kvp) {}
  epInitParams(std::initializer_list<const epKeyValuePair> list) : epInitParams(epSlice<const epKeyValuePair>(list.begin(), list.size())) {}

  epSlice<const epKeyValuePair> params;

  const epKeyValuePair& operator[](size_t index) const
  {
    return params[index];
  }
  const epVariant& operator[](epString key) const
  {
    for (auto &p : params)
    {
      if (p.key.is(epVariant::Type::String) && p.key.asString().eq(key))
        return p.value;
    }
    return varNull;
  }

  epIterator<const epKeyValuePair> begin() const
  {
    return params.begin();
  }
  epIterator<const epKeyValuePair> end() const
  {
    return params.end();
  }

private:
  static const epVariant varNull;
};


#include "ep/epvariant.inl"

#endif // EPVARIANT_H
