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

// static funcion glue
class StaticFunc
{
public:
  StaticFunc(nullptr_t) : shim(nullptr) {}

  operator bool() const { return shim != nullptr; }

  udVariant call(udSlice<udVariant> args) const
  {
    return shim(this, args);
  }

protected:
  typedef udVariant(Shim)(const StaticFunc* const, udSlice<udVariant>);
  Shim *shim;
};

// event glue
class VarEvent
{
public:
  VarEvent(nullptr_t) : pSubscribe(nullptr) {}

  operator bool() const { return pSubscribe != nullptr; }

  void subscribe(const ud::ComponentRef &c, const udVariant::VarDelegate &d)
  {
    pSubscribe(this, c, d);
  }

protected:
  typedef void (SubscribeFunc)(const VarEvent*, const ComponentRef&, const udVariant::VarDelegate&);
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

// static function glue
class CStaticFunc : public StaticFunc
{
public:
  CStaticFunc(nullptr_t) : StaticFunc(nullptr) {}
  template <typename Ret, typename... Args>
  CStaticFunc(Ret(*func)(Args...));

protected:
  void *f; // function pointer

  template<typename Ret, typename... Args>
  struct Partial // this allows us to perform partial specialisation for Ret == void
  {
    // this is a nasty hack to get ...S (integer sequence) as a parameter pack
    template<size_t ...S>
    static udVariant callFuncHack(udSlice<udVariant> args, Ret(*f)(Args...), Sequence<S...>);

    static udVariant shimFunc(const StaticFunc * const pSetter, udSlice<udVariant> value);
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
  static void doSubscribe(const VarEvent *pEv, const ComponentRef &c, const udVariant::VarDelegate &d);
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
  PropertyInfo(udString id, udString displayName, udString description, udString displayType = nullptr, uint32_t flags = 0)
    : id(id), displayName(displayName), description(description), displayType(displayType), flags(flags) {}
  PropertyInfo(const PropertyInfo &rh)
    : id(rh.id), displayName(rh.displayName), description(rh.description), displayType(rh.displayType), flags(rh.flags) {}

  udString id;
  udString displayName;
  udString description;

  udString displayType;
  uint32_t flags;
};

struct PropertyDesc
{
  PropertyDesc() = delete;
  PropertyDesc(const PropertyInfo &info, Getter *getter, Setter *setter)
    : info(info), getter(getter), setter(setter) {}
  PropertyDesc(const PropertyDesc &rh)
    : info(rh.info), getter(rh.getter), setter(rh.setter) {}

  PropertyInfo info;
  Getter *getter;
  Setter *setter;
};

struct FunctionInfo
{
  FunctionInfo() = delete;
  FunctionInfo(udString id, udString description)
    : id(id), description(description) {}
  FunctionInfo(const FunctionInfo &rh)
    : id(rh.id), description(rh.description) {}

  udString id;
  udString description;
};

struct MethodDesc
{
  MethodDesc() = delete;
  MethodDesc(const FunctionInfo &info, Method *method)
    : info(info), method(method) {}
  MethodDesc(const MethodDesc &rh)
    : info(rh.info), method(rh.method) {}

  FunctionInfo info;
  Method *method;
};

struct StaticFuncDesc
{
  StaticFuncDesc() = delete;
  StaticFuncDesc(const FunctionInfo &info, StaticFunc *staticFunc)
    : info(info), staticFunc(staticFunc) {}
  void operator=(const StaticFuncDesc&) = delete;

  FunctionInfo info;
  StaticFunc *staticFunc;
};

struct EventInfo
{
  EventInfo() = delete;
  EventInfo(udString id, udString displayName, udString description)
    : id(id), displayName(displayName), description(description) {}
  EventInfo(const EventInfo &rh)
    : id(rh.id), displayName(rh.displayName), description(rh.description) {}

  udString id;
  udString displayName;
  udString description;
};

struct EventDesc
{
  EventDesc() = delete;
  EventDesc(EventInfo info, VarEvent *ev)
    : info(info), ev(ev) {}
  EventDesc(const EventDesc &rh)
    : info(rh.info), ev(rh.ev) {}

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
  FunctionInfo info;
  CMethod method;
};

struct CEventDesc
{
  EventInfo info;
  CEvent ev;
};

struct CStaticFuncDesc
{
  FunctionInfo info;
  CStaticFunc staticFunc;
};

// component description
enum { UDSHELL_APIVERSION = 100 };
enum { UDSHELL_PLUGINVERSION = UDSHELL_APIVERSION };

typedef udResult(InitComponent)(Kernel*);
typedef Component *(CreateInstanceCallback)(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);

struct ComponentDesc
{
  ComponentDesc() = delete;
  ComponentDesc(ComponentDesc *pSuperDesc, int udVersion, int pluginVersion, udString id, udString displayName, udString description,
    udSlice<CPropertyDesc> properties = nullptr, udSlice<CMethodDesc> methods = nullptr, udSlice<CEventDesc> events = nullptr, udSlice<CStaticFuncDesc> staticFuncs = nullptr,
    InitComponent *pInit = nullptr, CreateInstanceCallback *pCreateInstance = nullptr)
    : pSuperDesc(pSuperDesc), udVersion(udVersion), pluginVersion(pluginVersion), id(id), displayName(displayName), description(description)
    , properties(properties), methods(methods), events(events), staticFuncs(staticFuncs), pInit(pInit), pCreateInstance(pCreateInstance) {}

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
  udSlice<CStaticFuncDesc> staticFuncs;

  InitComponent *pInit;
  CreateInstanceCallback *pCreateInstance;

  udAVLTree<udString, PropertyDesc> propertyTree;
  udAVLTree<udString, MethodDesc> methodTree;
  udAVLTree<udString, EventDesc> eventTree;
  udAVLTree<udString, StaticFuncDesc> staticFuncTree;

  StaticFunc *ComponentDesc::GetStaticFunc(udString id) const;

  void BuildSearchTrees();
  void InitProps();
  void InitMethods();
  void InitEvents();
  void InitStaticFuncs();
};

} // namespace ud


#include "componentdesc.inl"

#endif // UDCOMPONENTDESC_H
