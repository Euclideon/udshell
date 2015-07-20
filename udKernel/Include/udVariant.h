#pragma once
#ifndef UDVARIANT_H
#define UDVARIANT_H

#include "udString.h"
#include "udSharedPtr.h"
#include "udMath.h"

struct udKeyValuePair;
class udComponent;
typedef udSharedPtr<udComponent> udComponentRef;
class LuaState;

struct udVariant
{
public:
  enum Type : size_t
  {
    Null,
    Bool,
    Int,
    Float,
    Component,
    String,
    Array,
    AssocArray
  };

  udVariant();
  udVariant(udVariant &&rval);
  udVariant(const udVariant &rval);

  udVariant(nullptr_t);
  udVariant(bool b);
  udVariant(int8_t i);
  udVariant(uint8_t i);
  udVariant(int16_t i);
  udVariant(uint16_t i);
  udVariant(int32_t i);
  udVariant(uint32_t i);
  udVariant(int64_t i);
  udVariant(uint64_t i);
  udVariant(float f);
  udVariant(double f);
  udVariant(udComponentRef &c);
  udVariant(udString s);
  udVariant(udSlice<udVariant> a);
  udVariant(udSlice<udKeyValuePair> aa);

  // math types
  template<typename U>
  udVariant(const udVector2<U> &v);
  template<typename U>
  udVariant(const udVector3<U> &v);
  template<typename U>
  udVariant(const udVector4<U> &v);
  template<typename U>
  udVariant(const udMatrix4x4<U> &m);

  ~udVariant();

  // assignment operators
  udVariant& operator=(udVariant &&rval);
  udVariant& operator=(const udVariant &rval);

  // actual methods
  Type type() const;

  udRCString stringify() const;

  template<typename T>
  T as() const;

  bool asBool() const;
  int64_t asInt() const;
  double asFloat() const;
  udComponentRef asComponent() const;
  udString asString() const;
  udSlice<udVariant> asArray() const;
  udSlice<udKeyValuePair> asAssocArray() const;
  udSlice<udKeyValuePair> asAssocArraySeries() const;

  size_t arrayLen() const;
  size_t assocArraySeriesLen() const;

  udVariant operator[](size_t i) const;
  udVariant operator[](udString key) const;

  void luaPush(LuaState &l) const;
  static udVariant luaGet(LuaState &l, int idx = -1);

private:
  size_t t : 3;
  size_t ownsArray : 1;
  size_t length : (sizeof(size_t)*8)-4;
  union
  {
    bool b;
    int64_t i;
    double f;
    udComponent *c;
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

typedef udSlice<udKeyValuePair> udInitParams;


#include "udVariant.inl"

#endif // UDVARIANT_H
