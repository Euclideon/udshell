#pragma once
#ifndef UDCOMPONENTDESC_H
#define UDCOMPONENTDESC_H

#include "udPlatform.h"

#include "util/udstring.h"
#include "util/udsharedptr.h"
#include "util/udvariant.h"
#include "util/udevent.h"
#include "util/udavltree.h"


// TODO: remove this!
#if UDPLATFORM_WINDOWS
#pragma warning(disable: 4100)
#endif // UDPLATFORM_WINDOWS


#define PROTOTYPE_COMPONENT(Name) \
  SHARED_CLASS(Name)


namespace ud
{

class Kernel;
struct ComponentDesc;

PROTOTYPE_COMPONENT(Component);


// getter glue
struct Getter
{
public:
  Getter(nullptr_t);
  template <typename X, typename Type>
  Getter(Type(X::*func)() const);

  operator bool() const { return shim != nullptr; }

  udVariant get(const ud::Component *pThis) const;

protected:
  typedef udVariant(Shim)(const Getter* const, const ud::Component*);

  FastDelegateMemento m;
  Shim *shim;

  template<typename T>
  static udVariant shimFunc(const Getter * const pGetter, const ud::Component *pThis);
};

// setter glue
struct Setter
{
public:
  Setter(nullptr_t);
  template <typename X, typename Type>
  Setter(void(X::*func)(Type));

  operator bool() const { return shim != nullptr; }

  void set(ud::Component *pThis, const udVariant &value) const;

private:
  typedef void(Shim)(const Setter* const, ud::Component*, const udVariant&);

  FastDelegateMemento m;
  Shim *shim;

  template<typename T>
  static void shimFunc(const Setter * const pSetter, ud::Component *pThis, const udVariant &value);
};

// method glue
struct Method
{
public:
  Method(nullptr_t);
  template <typename X, typename Ret, typename... Args>
  Method(Ret(X::*func)(Args...));
  template <typename X, typename Ret, typename... Args>
  Method(Ret(X::*func)(Args...) const);

  operator bool() const { return shim != nullptr; }

  udVariant call(ud::Component *pThis, udSlice<udVariant> args) const;

private:
  typedef udVariant(Shim)(const Method* const, ud::Component*, udSlice<udVariant>);

  FastDelegateMemento m;
  Shim *shim;

  template<typename Ret, typename... Args>
  struct Partial // this allows us to perform partial specialisation for Ret == void
  {
    // this is a nasty hack to get ...S (integer sequence) as a parameter pack
    template<size_t ...S>
    static udVariant callFuncHack(udSlice<udVariant> args, FastDelegate<Ret(Args...)> d, Sequence<S...>);

    static udVariant shimFunc(const Method * const pSetter, ud::Component *pThis, udSlice<udVariant> value);
  };
};

// event glue
struct VarEvent
{
  VarEvent(nullptr_t);
  template<typename X, typename... Args>
  VarEvent(udEvent<Args...> X::*ev)
  {
    pSubscribe = &doSubscribe<X, Args...>;
    pEvent = (void* VarEvent::*)ev;
  }

  operator bool() const { return pEvent != nullptr; }

  UDFORCE_INLINE void subscribe(const ud::ComponentRef &c, const udVariant::Delegate &d)
  {
    pSubscribe(this, c, d);
  }

private:
  typedef void (SubscribeFunc)(const VarEvent*, const ComponentRef&, const udVariant::Delegate&);

  void* VarEvent::*pEvent;
  SubscribeFunc *pSubscribe = nullptr;

  template<typename X, typename... Args>
  static void doSubscribe(const VarEvent *pEv, const ComponentRef &c, const udVariant::Delegate &d)
  {
    // cast the pointer-to-member back to it's real type
    udEvent<Args...> X::*ev = (udEvent<Args...> X::*)pEv->pEvent;

    // TODO: validate that 'X' is actually a component?
    X *pComponent = (X*)c.ptr();

    // deref the pointer-to-member to get the event we want to subscribe to
    udEvent<Args...> &e = pComponent->*ev;

    udVariant v(d);
    e.Subscribe(v.as<udDelegate<void(Args...)>>());
  }
};


// property description
enum class PropertyType : uint32_t
{
  Void,
  Boolean,
  Integer,
  Float,
  String,
  Variant,
  Component,
  Resource,
  Enum,
  Flags,
  Delegate,
  Struct,
};

enum PropertyFlags : uint32_t
{
  udPF_Immutable = 1<<0 // must be initialised during construction
};

struct EnumKVP
{
  EnumKVP(udString key, int64_t v) : key(key), value(v) {}

  udString key;
  int64_t value;
};
#define EnumKVP(e) EnumKVP( #e, (int64_t)e )

struct TypeDesc
{
  TypeDesc(PropertyType type, uint32_t arrayLength = 0, const udSlice<const EnumKVP> kvp = nullptr)
    : type(type), arrayLength(arrayLength), kvp(kvp)
  {}
  TypeDesc& operator=(const TypeDesc&) = delete;

  PropertyType type;
  uint32_t arrayLength;
  const udSlice<const EnumKVP> kvp;
};

struct PropertyDesc
{
  PropertyDesc() = delete;
  void operator=(const PropertyDesc&) = delete;

  udString id;
  udString displayName;
  udString description;

  Getter getter;
  Setter setter;

  TypeDesc type;
  udString displayType;
  uint32_t flags;

  uint32_t index;
};

struct MethodDesc
{
  MethodDesc() = delete;
  void operator=(const MethodDesc&) = delete;

  udString id;
  udString description;

  Method method;

  TypeDesc result;
  const udSlice<const TypeDesc> args;

  uint32_t index;
};

struct EventDesc
{
  EventDesc() = delete;
  void operator=(const EventDesc&) = delete;

  udString id;
  udString displayName;
  udString description;

  VarEvent ev;

  const udSlice<const TypeDesc> args;

  uint32_t index;
};


// component description
enum { UDSHELL_APIVERSION = 100 };
enum { UDSHELL_PLUGINVERSION = UDSHELL_APIVERSION };

typedef udResult(InitComponent)();
typedef Component *(CreateInstanceCallback)(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams);

struct ComponentDesc
{
  ComponentDesc() = delete;
  ComponentDesc& operator=(const ComponentDesc&) = delete;

  ComponentDesc *pSuperDesc;

  int udVersion;
  int pluginVersion;

  udString id;          // an id for this component
  udString displayName; // display name
  udString description; // description

  // icon image...

  // TODO: add flags ('Abstract' (can't create) flag)

  const udSlice<PropertyDesc> properties;
  const udSlice<MethodDesc> methods;
  const udSlice<EventDesc> events;

  InitComponent *pInit;
  CreateInstanceCallback *pCreateInstance;

  struct StringCompare {
    inline ptrdiff_t operator()(udString a, udString b)
    {
      return a.cmp(b);
    }
  };

  udAVLTree<udString, PropertyDesc*, StringCompare> propertyTree;
  udAVLTree<udString, MethodDesc*, StringCompare> methodTree;
  udAVLTree<udString, EventDesc*, StringCompare> eventTree;

  void BuildSearchTree();

  size_t NumProperties() const { return propertyTree.Size(); }
  size_t NumMethods() const { return methodTree.Size(); }
  size_t NumEvents() const { return eventTree.Size(); }
};

} // namespace ud


#include "componentdesc.inl"

#endif // UDCOMPONENTDESC_H
