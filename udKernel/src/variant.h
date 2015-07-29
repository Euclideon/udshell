#pragma once
#if !defined(UDVARIANT_H)
#define UDVARIANT_H

#include "udstring.h"
#include "delegate.h"

namespace ud
{

SHARED_CLASS(Component);
struct KeyValuePair;
class LuaState;

struct Variant
{
public:
  typedef udDelegate<Variant(udSlice<Variant>)> Delegate;

  enum class Type
  {
    Null,
    Bool,
    Int,
    Float,
    Component,
    Delegate,
    String,
    Array,
    AssocArray
  };

  Variant();
  Variant(Variant &&rval);
  Variant(const Variant &rval);

  Variant(bool);
  Variant(int64_t);
  Variant(double);
  Variant(Component *);
  Variant(const Delegate &d);
  Variant(Delegate &&d);
  Variant(udString);
  Variant(udSlice<Variant> a, bool ownsMemory = false);
  Variant(udSlice<KeyValuePair> aa, bool ownsMemory = false);

  template<typename T> Variant(const T &v);
  template<typename T> Variant(T &v);

  ~Variant();

  // assignment operators
  Variant& operator=(Variant &&rval);
  Variant& operator=(const Variant &rval);

  // actual methods
  Type type() const;
  bool is(Type type) const;

  udRCString stringify() const;

  template<typename T>
  T as() const;

  bool asBool() const;
  int64_t asInt() const;
  double asFloat() const;
  ComponentRef asComponent() const;
  Delegate asDelegate() const;
  udString asString() const;
  udSlice<Variant> asArray() const;
  udSlice<KeyValuePair> asAssocArray() const;
  udSlice<KeyValuePair> asAssocArraySeries() const;

  size_t arrayLen() const;
  size_t assocArraySeriesLen() const;

  Variant operator[](size_t i) const;
  Variant operator[](udString key) const;

  Variant* allocArray(size_t len);
  KeyValuePair* allocAssocArray(size_t len);

  void luaPush(LuaState &l) const;
  static Variant luaGet(LuaState &l, int idx = -1);

private:
  size_t t : 4;
  size_t ownsArray : 1;
  size_t length : (sizeof(size_t)*8)-5;
  union
  {
    bool b;
    int64_t i;
    double f;
    Component *c;
    const char *s;
    Variant *a;
    KeyValuePair *aa;
    void *p;
  };
};

struct KeyValuePair
{
  KeyValuePair() {}
  KeyValuePair(KeyValuePair &&val) : key(std::move(val.key)), value(std::move(val.value)) {}
  KeyValuePair(const KeyValuePair &val) : key(val.key), value(val.value) {}

  KeyValuePair(const Variant &key, const Variant &value) : key(key), value(value) {}
  KeyValuePair(const Variant &key, Variant &&value) : key(key), value(std::move(value)) {}
  KeyValuePair(Variant &&key, const Variant &value) : key(std::move(key)), value(value) {}
  KeyValuePair(Variant &&key, Variant &&value) : key(std::move(key)), value(std::move(value)) {}

  Variant key;
  Variant value;
};

typedef udSlice<KeyValuePair> InitParams;

} // namespace ud


#include "variant.inl"

#endif // UDVARIANT_H
