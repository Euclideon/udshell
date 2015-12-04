#pragma once
#if !defined(EPVARIANT_HPP)
#define EPVARIANT_HPP

#include "ep/cpp/sharedptr.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/map.h"

#include "ep/c/variant.h"

namespace kernel {
class LuaState;
}

namespace ep {

struct Variant;
template<>
struct Compare<Variant>
{
  ptrdiff_t operator()(Variant a, Variant b);
};

SHARED_CLASS(Component);

struct KeyValuePair;

struct Variant
{
public:
  typedef Delegate<Variant(Slice<Variant>)> VarDelegate;
  typedef SharedArray<Variant> VarArray;
  typedef SharedMap<AVLTree<Variant, Variant>> VarMap;

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
    Void = epVT_Void,
    SmallString = epVT_SmallString
  };

  Variant();
  Variant(Variant &&rval);
  Variant(const Variant &val);

  Variant(epVariant &&rval);
  Variant(const epVariant &val);

  Variant(Type);
  Variant(nullptr_t);
  Variant(bool);
  Variant(int64_t);
  Variant(double);
  Variant(size_t val, const EnumDesc *pDesc, bool isBitfield);

  Variant(VarDelegate &&d);
  Variant(const VarDelegate &d);

  Variant(ComponentRef &&spC);
  Variant(const ComponentRef &spC);

  // all the different strings
  Variant(String s, bool unsafeReference = false);
  template<size_t Len> Variant(const MutableString<Len> &s);
  template<size_t Len> Variant(MutableString<Len> &&s);
  Variant(const SharedString &s);
  Variant(SharedString &&s);

  // arrays
  Variant(Slice<Variant> a, bool unsafeReference = false);
  template<size_t Len> Variant(const Array<Variant, Len> &a);
  template<size_t Len> Variant(Array<Variant, Len> &&a);
  Variant(const VarArray &a);
  Variant(VarArray &&a);

  // assoc arrays
  Variant(Slice<KeyValuePair> aa);
  Variant(const VarMap &aa);
  Variant(VarMap &&aa);

  template<typename T> Variant(T &&rval);

  ~Variant();

  // assignment operators
  Variant& operator=(Variant &&rval);
  Variant& operator=(const Variant &rval);

  // actual methods
  Type type() const;
  bool is(Type type) const;

  bool isValid() const;
  bool isNull() const;

  SharedString stringify() const;

  template<typename T>
  T as() const;

  ptrdiff_t compare(const Variant &v) const;

  bool asBool() const;
  int64_t asInt() const;
  double asFloat() const;
  const EnumDesc* asEnum(size_t *pVal) const;
  ComponentRef asComponent() const;
  VarDelegate asDelegate() const;
  String asString() const;
  SharedString asSharedString() const;
  Slice<Variant> asArray() const;
  VarMap asAssocArray() const;
//  Slice<KeyValuePair> asAssocArraySeries() const;

  size_t arrayLen() const;
  size_t assocArraySeriesLen() const;

  Variant operator[](size_t i) const;
  Variant operator[](String key) const;

  // TODO: these shouldn't be part of the public API!
  void luaPush(kernel::LuaState &l) const;
  static Variant luaGet(kernel::LuaState &l, int idx = -1);

private:
  size_t t : 4;
  size_t ownsContent : 1;
  size_t length : (sizeof(size_t)*8)-5; // NOTE: if you change this, update the shift's in asEnum()!!!
  union
  {
    char b;
    int64_t i;
    double f;
    const char *s;
    Component *c;
    void *p;
    Variant *a;
    AVLTree<Variant, Variant> *aa;
  };

  void copyContent(const Variant &val);
  void destroy();
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

  KeyValuePair(const Variant::VarMap::Iterator::KVP &kvp) : key(kvp.key), value(kvp.value) {}

  Variant key;
  Variant value;
};

// TODO: abolish InitParams!
class InitParams
{
public:
  InitParams() {}
  InitParams(nullptr_t) {}
  InitParams(const InitParams& rh) : params(rh.params) {}
  InitParams(InitParams&& rh) : params(std::move(rh.params)) {}
  InitParams(Variant::VarMap kvp) : params(kvp) {}
  InitParams(Slice<const KeyValuePair> list)
  {
    for (auto &kvp : list)
      params.Insert(kvp.key, kvp.value);
  }
  InitParams(std::initializer_list<const KeyValuePair> list)
    : InitParams(Slice<const KeyValuePair>(list.begin(), list.size())) {}

  Variant::VarMap params;

  const KeyValuePair operator[](size_t index) const
  {
    const Variant *pV = params.Get(index);
    return KeyValuePair(index, pV ? *pV : Variant());
  }
  KeyValuePair operator[](size_t index)
  {
    Variant *pV = params.Get(index);
    return KeyValuePair(index, pV ? *pV : Variant());
  }
  const Variant& operator[](String key) const
  {
    const Variant *pV = params.Get(key);
    return pV ? *pV : varNone;
  }

  Variant::VarMap::Iterator begin() const
  {
    return params.begin();
  }
  Variant::VarMap::Iterator end() const
  {
    return params.end();
  }

private:
  static const Variant varNone;
};

namespace internal {
  static const size_t VariantSmallStringSize = sizeof(Variant) - 1;
} // namespace internal

} // namespace ep

#include "ep/cpp/internal/variant_inl.h"

// unit tests
epResult epVariant_Test();

#endif // EPVARIANT_HPP
