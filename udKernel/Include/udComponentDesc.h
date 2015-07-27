#pragma once
#ifndef UDCOMPONENTDESC_H
#define UDCOMPONENTDESC_H

#include "udPlatform.h"
#include "udString.h"
#include "udSharedPtr.h"
#include "udVariant.h"
#include "udEvent.h"

using namespace fastdelegate;

// TODO: remove this!
#if UDPLATFORM_WINDOWS
#pragma warning(disable: 4100)
#endif // UDPLATFORM_WINDOWS

#if !defined(_MSC_VER)
#include <cstddef>
using std::nullptr_t;
#endif //!defined(_MSC_VER)


#define PROTOTYPE_COMPONENT(Name) \
  SHARED_CLASS(Name)


class udKernel;
struct udComponentDesc;

PROTOTYPE_COMPONENT(udComponent);


// getter glue
struct udGetter
{
public:
  udGetter(nullptr_t);
  template <typename X, typename Type>
  udGetter(Type(X::*func)() const);

  operator bool() const { return shim != nullptr; }

  udVariant get(const udComponent *pThis) const;

protected:
  typedef udVariant(Shim)(const udGetter* const, const udComponent*);

  DelegateMemento m;
  Shim *shim;

  template<typename T>
  static udVariant shimFunc(const udGetter * const pGetter, const udComponent *pThis);
};

// setter glue
struct udSetter
{
public:
  udSetter(nullptr_t);
  template <typename X, typename Type>
  udSetter(void(X::*func)(Type));

  operator bool() const { return shim != nullptr; }

  void set(udComponent *pThis, const udVariant &value) const;

private:
  typedef void(Shim)(const udSetter* const, udComponent*, const udVariant&);

  DelegateMemento m;
  Shim *shim;

  template<typename T>
  static void shimFunc(const udSetter * const pSetter, udComponent *pThis, const udVariant &value);
};

// method glue
struct udMethod
{
public:
  udMethod(nullptr_t);
  template <typename X, typename Ret, typename... Args>
  udMethod(Ret(X::*func)(Args...));
  template <typename X, typename Ret, typename... Args>
  udMethod(Ret(X::*func)(Args...) const);

  operator bool() const { return shim != nullptr; }

  udVariant call(udComponent *pThis, udSlice<udVariant> args) const;

private:
  typedef udVariant(Shim)(const udMethod* const, udComponent*, udSlice<udVariant>);

  DelegateMemento m;
  Shim *shim;

  template<typename Ret, typename... Args>
  struct Partial // this allows us to perform partial specialisation for Ret == void
  {
    // this is a nasty hack to get ...S (integer sequence) as a parameter pack
    template<size_t ...S>
    static udVariant callFuncHack(udSlice<udVariant> args, FastDelegate<Ret(Args...)> d, Sequence<S...>);

    static udVariant shimFunc(const udMethod * const pSetter, udComponent *pThis, udSlice<udVariant> value);
  };
};

// event glue
struct udVarEvent
{
  udVarEvent(nullptr_t);
  template<typename X, typename... Args>
  udVarEvent(udEvent<Args...> X::*ev)
  {
    pSubscribe = &doSubscribe<X, Args...>;
    pEvent = (void* udVarEvent::*)ev;
  }

  operator bool() const { return pEvent != nullptr; }

  UDFORCE_INLINE void subscribe(const udComponentRef &c, const udVariant::Delegate &d)
  {
    pSubscribe(this, c, d);
  }

private:
  typedef void (SubscribeFunc)(const udVarEvent*, const udComponentRef&, const udVariant::Delegate&);

  void* udVarEvent::*pEvent;
  SubscribeFunc *pSubscribe = nullptr;

  template<typename X, typename... Args>
  static void doSubscribe(const udVarEvent *pEv, const udComponentRef &c, const udVariant::Delegate &d)
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
enum class udPropertyType : uint32_t
{
  Void,
  Boolean,
  Integer,
  Float,
  String,
  Component,
  Enum,
  Flags,
  Struct,
};

enum udPropertyFlags : uint32_t
{
  udPF_Immutable = 1<<0 // must be initialised during construction
};

struct udEnumKVP
{
  udEnumKVP(udString key, int64_t v) : key(key), value(v) {}

  udString key;
  int64_t value;
};
#define EnumKVP(e) udEnumKVP( #e, (int64_t)e )

struct udTypeDesc
{
  udTypeDesc(udPropertyType type, uint32_t arrayLength = 0, const udSlice<const udEnumKVP> kvp = nullptr)
    : type(type), arrayLength(arrayLength), kvp(kvp)
  {}
  udTypeDesc& operator=(const udTypeDesc&) = delete;

  udPropertyType type;
  uint32_t arrayLength;
  const udSlice<const udEnumKVP> kvp;
};

struct udPropertyDesc
{
  udPropertyDesc() = delete;

  udString id;
  udString displayName;
  udString description;

  udGetter getter;
  udSetter setter;

  udTypeDesc type;
  udString displayType;
  uint32_t flags;
};

struct udMethodDesc
{
  udMethodDesc() = delete;

  udString id;
  udString displayName;
  udString description;

  udMethod method;

  udTypeDesc result;
  const udSlice<const udTypeDesc> args;
};

struct udEventDesc
{
  udEventDesc() = delete;

  udString id;
  udString displayName;
  udString description;

  udVarEvent ev;

  const udSlice<const udTypeDesc> args;
};


// component description
enum { UDSHELL_APIVERSION = 100 };
enum { UDSHELL_PLUGINVERSION = UDSHELL_APIVERSION };

typedef udResult(InitComponent)();
typedef udComponent *(CreateInstanceCallback)(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams);

struct udComponentDesc
{
  udComponentDesc() = delete;
  udComponentDesc& operator=(const udComponentDesc&) = delete;

  const udComponentDesc *pSuperDesc;

  int udVersion;
  int pluginVersion;

  udString id;          // an id for this component
  udString displayName; // display name
  udString description; // description

  // icon image...

//  this
  InitComponent *pInit;
  CreateInstanceCallback *pCreateInstance;

  const udSlice<const udPropertyDesc> properties;
  const udSlice<const udMethodDesc> methods;
  const udSlice<const udEventDesc> events;
};


#include "udComponentDesc.inl"

#endif // UDCOMPONENTDESC_H
