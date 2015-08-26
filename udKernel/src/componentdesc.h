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


// interface for getters, setters, methods, events

// getter glue
class Getter
{
public:
  Getter(nullptr_t) : shim(nullptr) {}

  operator bool() const { return shim != nullptr; }

  udVariant get(const ud::Component *pThis) const
  {
    return shim(this, pThis);
  }

protected:
  typedef udVariant(Shim)(const Getter* const, const ud::Component*);
  Shim *shim;
};

// setter glue
class Setter
{
public:
  Setter(nullptr_t) : shim(nullptr) {}

  operator bool() const { return shim != nullptr; }

  void set(ud::Component *pThis, const udVariant &value) const
  {
    shim(this, pThis, value);
  }

protected:
  typedef void(Shim)(const Setter* const, ud::Component*, const udVariant&);
  Shim *shim;
};

// method glue
class Method
{
public:
  Method(nullptr_t) : shim(nullptr) {}

  operator bool() const { return shim != nullptr; }

  udVariant call(ud::Component *pThis, udSlice<udVariant> args) const
  {
    return shim(this, pThis, args);
  }

protected:
  typedef udVariant(Shim)(const Method* const, ud::Component*, udSlice<udVariant>);
  Shim *shim;
};

// event glue
class VarEvent
{
public:
  VarEvent(nullptr_t) : pSubscribe(nullptr) {}

  operator bool() const { return pSubscribe != nullptr; }

  void subscribe(const ud::ComponentRef &c, const udVariant::Delegate &d)
  {
    pSubscribe(this, c, d);
  }

protected:
  typedef void (SubscribeFunc)(const VarEvent*, const ComponentRef&, const udVariant::Delegate&);
  SubscribeFunc *pSubscribe = nullptr;
};


// C/C++ implementations of the getters/setters/methods/events

// getter glue
class CGetter : public Getter
{
public:
  CGetter(nullptr_t) : Getter(nullptr) {}
  template <typename X, typename Type>
  CGetter(Type(X::*func)() const);

protected:
  FastDelegateMemento m;

  template<typename T>
  static udVariant shimFunc(const Getter * const pGetter, const ud::Component *pThis);
};

// setter glue
class CSetter : public Setter
{
public:
  CSetter(nullptr_t) : Setter(nullptr) {}
  template <typename X, typename Type>
  CSetter(void(X::*func)(Type));

protected:
  FastDelegateMemento m;

  template<typename T>
  static void shimFunc(const Setter * const pSetter, ud::Component *pThis, const udVariant &value);
};

// method glue
class CMethod : public Method
{
public:
  CMethod(nullptr_t) : Method(nullptr) {}
  template <typename X, typename Ret, typename... Args>
  CMethod(Ret(X::*func)(Args...));
  template <typename X, typename Ret, typename... Args>
  CMethod(Ret(X::*func)(Args...) const);

protected:
  FastDelegateMemento m;

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
class CEvent : public VarEvent
{
public:
  CEvent(nullptr_t) : VarEvent(nullptr) {}
  template<typename X, typename... Args>
  CEvent(udEvent<Args...> X::*ev);

protected:
  void* CEvent::*pEvent;

  template<typename X, typename... Args>
  static void doSubscribe(const VarEvent *pEv, const ComponentRef &c, const udVariant::Delegate &d);
};


// property description
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

struct PropertyInfo
{
  PropertyInfo() = delete;

  udString id;
  udString displayName;
  udString description;

  udString displayType;
  uint32_t flags;
};

struct PropertyDesc
{
  PropertyDesc() = delete;
  void operator=(const PropertyDesc&) = delete;

  PropertyInfo info;
  Getter *getter;
  Setter *setter;
};

struct MethodInfo
{
  MethodInfo() = delete;

  udString id;
  udString description;
};

struct MethodDesc
{
  MethodDesc() = delete;
  void operator=(const MethodDesc&) = delete;

  MethodInfo info;
  Method *method;
};

struct EventInfo
{
  EventInfo() = delete;
  void operator=(const EventInfo&) = delete;

  udString id;
  udString displayName;
  udString description;
};

struct EventDesc
{
  EventDesc() = delete;
  void operator=(const EventDesc&) = delete;

  EventInfo info;
  VarEvent *ev;
};


struct CPropertyDesc
{
  PropertyInfo info;
  CGetter getter;
  CSetter setter;
};

struct CMethodDesc
{
  MethodInfo info;
  CMethod method;
};

struct CEventDesc
{
  EventInfo info;
  CEvent ev;
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

  udSlice<CPropertyDesc> properties;
  udSlice<CMethodDesc> methods;
  udSlice<CEventDesc> events;

  InitComponent *pInit;
  CreateInstanceCallback *pCreateInstance;

  udAVLTree<udString, PropertyDesc> propertyTree;
  udAVLTree<udString, MethodDesc> methodTree;
  udAVLTree<udString, EventDesc> eventTree;

  size_t NumProperties() const { return propertyTree.Size(); }
  size_t NumMethods() const { return methodTree.Size(); }
  size_t NumEvents() const { return eventTree.Size(); }

  void BuildSearchTrees();
  void InitProps();
  void InitMethods();
  void InitEvents();
};

} // namespace ud


#include "componentdesc.inl"

#endif // UDCOMPONENTDESC_H
