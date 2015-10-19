#pragma once
#if !defined(EPVARIANT_HPP)
#define EPVARIANT_HPP

#include "ep/cpp/sharedptr.h"
#include "ep/cpp/delegate.h"

#include "ep/c/variant.h"

namespace ep {

SHARED_CLASS(Component);
class LuaState;

struct KeyValuePair;

struct Variant : protected epVariant
{
public:
  typedef Delegate<Variant(Slice<Variant>)> VarDelegate;

  enum class Type
  {
    Null = epVT_Null,
    Bool = epVT_Bool,
    Int = epVT_Int,
    Float = epVT_Float,
    Enum = epVT_Enum,
    Bitfield = epVT_Bitfield,
    Component = epVT_Component,
    Delegate = epVT_Delegate,
    String = epVT_String,
    Array = epVT_Array,
    AssocArray = epVT_AssocArray,
    SmallString = epVT_SmallString
  };

  Variant();
  Variant(Variant &&rval);
  Variant(const Variant &val);

  Variant(epVariant &&rval);
  Variant(const epVariant &val);

  Variant(bool);
  Variant(int64_t);
  Variant(double);
  Variant(size_t val, const epEnumDesc *pDesc, bool isBitfield);
  Variant(ComponentRef &&spC);
  Variant(const ComponentRef &spC);
  Variant(VarDelegate &&d);
  Variant(const VarDelegate &d);
  Variant(String, bool ownsMemory = false);
  Variant(Slice<Variant> a, bool ownsMemory = false);
  Variant(Slice<KeyValuePair> aa, bool ownsMemory = false);

  template<typename T> Variant(T &&rval);

  ~Variant();

  // assignment operators
  Variant& operator=(Variant &&rval);
  Variant& operator=(const Variant &rval);

  // actual methods
  Type type() const;
  bool is(Type type) const;

  SharedString stringify() const;

  template<typename T>
  T as() const;

  ptrdiff_t compare(const Variant &v) const;

  bool asBool() const;
  int64_t asInt() const;
  double asFloat() const;
  const epEnumDesc* asEnum(size_t *pVal) const;
  ComponentRef asComponent() const;
  VarDelegate asDelegate() const;
  String asString() const;
  Slice<Variant> asArray() const;
  Slice<KeyValuePair> asAssocArray() const;
  Slice<KeyValuePair> asAssocArraySeries() const;

  size_t arrayLen() const;
  size_t assocArraySeriesLen() const;

  Variant operator[](size_t i) const;
  Variant operator[](String key) const;

  Variant* allocArray(size_t len);
  KeyValuePair* allocAssocArray(size_t len);

  // TODO: these shouldn't be part of the public API!
  void luaPush(LuaState &l) const;
  static Variant luaGet(LuaState &l, int idx = -1);

private:
  void copyContent(const Variant &val);
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

class InitParams
{
public:
  InitParams() {}
  InitParams(nullptr_t) {}
  InitParams(const InitParams& rh) : params(rh.params) {}
  InitParams(Slice<const KeyValuePair> kvp) : params(kvp) {}
  InitParams(std::initializer_list<const KeyValuePair> list) : InitParams(Slice<const KeyValuePair>(list.begin(), list.size())) {}

  Slice<const KeyValuePair> params;

  const KeyValuePair& operator[](size_t index) const
  {
    return params[index];
  }
  const Variant& operator[](String key) const
  {
    for (auto &p : params)
    {
      if (p.key.is(Variant::Type::String) && p.key.asString().eq(key))
        return p.value;
    }
    return varNull;
  }

  Iterator<const KeyValuePair> begin() const
  {
    return params.begin();
  }
  Iterator<const KeyValuePair> end() const
  {
    return params.end();
  }

private:
  static const Variant varNull;
};

} // namespace ep

#include "ep/cpp/internal/variant_inl.h"

#endif // EPVARIANT_HPP
