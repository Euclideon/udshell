#pragma once
#ifndef UDVARIANT_H
#define UDVARIANT_H

#include "udString.h"
#include "udSharedPtr.h"
#include "udMath.h"

struct udKeyValuePair;
class udComponent;
typedef udSharedPtr<udComponent> udComponentRef;

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

    // NOTE: key-value-pair's union the value pointer with the type
    // if 't' is a real pointer (a big number), assume type == AssocArray
  };

  udVariant();
  udVariant(bool b);
  udVariant(int64_t i);
  udVariant(double f);
  udVariant(udComponentRef &_c);
  udVariant(udString s);
  udVariant(udSlice<udVariant> a);
  udVariant(udSlice<udKeyValuePair> aa);

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

private:
  size_t t : 3;
  size_t length : (sizeof(size_t)*8)-3;
  union
  {
    bool b;
    int64_t i;
    double f;
    udComponent *c;
    const char *s;
    udVariant *a;
    udKeyValuePair *aa;
  };
};

struct udKeyValuePair
{
  udKeyValuePair(udVariant key, udVariant value) : key(key), value(value) {}

  udVariant key;
  udVariant value;
};

typedef udSlice<udKeyValuePair> udInitParams;


#include "udVariant.inl"

#endif // UDVARIANT_H
