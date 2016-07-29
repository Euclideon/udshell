#pragma once
#if !defined(EPVARIANT_HPP)
#define EPVARIANT_HPP

#include "ep/cpp/sharedptr.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/avltree.h"
#include "ep/cpp/map.h"
#include "ep/cpp/range.h"
#include "ep/cpp/event.h"

namespace ep {
class LuaState;
}

namespace ep {

struct Variant;
//! \cond
template<>
struct Compare<Variant>
{
  ptrdiff_t operator()(const Variant &a, const Variant &b);
};
//! \endcond

SHARED_CLASS(Component);

using VarMethod = MethodPointer<Variant(Slice<const Variant>)>;
using VarDelegate = Delegate<Variant(Slice<const Variant>)>;
using VarRange = VirtualRange<Variant>;

using KeyValuePair = KVP<Variant, Variant>;
using VariantAVLNode = AVLTreeNode<Variant, Variant>;
template<>
struct AVLTreeAllocator<VariantAVLNode>;

struct Variant
{
public:
  using VarArray = SharedArray<Variant>;
  using VarMap = SharedMap<AVLTree<Variant, Variant>>;

  enum class Type
  {
    Void,
    Error,

    Null,
    Bool,
    Int,
    Float,
    Enum,
    Bitfield,
    SharedPtr,
    String,
    Array,

    SmallString
  };

  enum class SharedPtrType
  {
    Unknown,
    Component,
    Delegate,
    Subscription,
    AssocArray,
    Range
  };

  Variant();
  Variant(Variant &&rval);
  Variant(const Variant &val);

  Variant(Type);
  Variant(nullptr_t);
  Variant(ErrorState *pErrorState);

  Variant(bool);
  Variant(int64_t);
  Variant(double);
  Variant(size_t val, const EnumDesc *pDesc, bool isBitfield);

  Variant(SharedPtr<RefCounted> &&d, SharedPtrType type = SharedPtrType::Unknown);
  Variant(const SharedPtr<RefCounted> &d, SharedPtrType type = SharedPtrType::Unknown, bool ownsContent = true);
  Variant(VarDelegate &&d);
  Variant(const VarDelegate &d);
  Variant(ComponentRef &&spC);
  Variant(const ComponentRef &spC);
  Variant(SubscriptionRef &&spS);
  Variant(const SubscriptionRef &spS);
  Variant(VarRange &&spS);
  Variant(const VarRange &spS);

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
  SharedPtrType spType() const;
  bool is(Type type) const;
  bool is(SharedPtrType type) const;

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
  SharedPtr<RefCounted> asSharedPtr() const;
  ComponentRef asComponent() const;
  VarDelegate asDelegate() const;
  SubscriptionRef asSubscription() const;
  String asString() const;
  SharedString asSharedString() const;
  Slice<Variant> asArray() const;
  SharedArray<Variant> asSharedArray() const;
  VarMap asAssocArray() const;
  VarRange asRange() const;

  size_t arrayLen() const;
  size_t assocArraySeriesLen() const;

  Variant& operator[](size_t i) const;
  Variant& operator[](String key) const;
  Variant* getItem(Variant key) const;
  Variant& insertItem(Variant key, Variant value);

  VarRange getRange() const;

  void throwError();

  // TODO: these shouldn't be part of the public API!
  void luaPush(LuaState &l) const;
  static Variant luaGet(LuaState &l, int idx = -1);

private:
  Variant(RefCounted *pRef, SharedPtrType type = SharedPtrType::Unknown, bool ownsContent = true);

  // HACK: variant is 16 bytes. TODO: 32bit variant should be 8 bytes to pass quickly
  uint64_t t : 4;
  uint64_t ownsContent : 1;
  uint64_t length : (sizeof(uint64_t)*8)-5; // NOTE: if you change this, update the shift's in asEnum()!!!
  union
  {
    char b;
    int64_t i;
    double f;
    const char *s;
    RefCounted *sp;
    Component *c;
    DelegateMemento *d;
    Subscription *sub;
    void *p;
    Variant *a;
    VarMap::Node *aa;
    VarRange::VirtualRangeImpl *r;
    ErrorState *err;
  };

  void copyContent(const Variant &val);
  void destroy();
};

class VarEvent : public ep::BaseEvent
{
public:
  ep::SubscriptionRef addSubscription(const ep::VarDelegate &del);
  void signal(ep::Slice<const ep::Variant> args);
};

namespace internal {
  static const size_t VariantSmallStringSize = sizeof(Variant) - 1;
} // namespace internal

} // namespace ep

#include "ep/cpp/internal/variant_inl.h"

#endif // EPVARIANT_HPP
