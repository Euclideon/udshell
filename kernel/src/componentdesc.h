#pragma once
#ifndef EPCOMPONENTDESC_H
#define EPCOMPONENTDESC_H

#include "ep/epplatform.h"

#include "ep/epstring.h"
#include "ep/epsharedptr.h"
#include "ep/epvariant.h"
#include "ep/epevent.h"
#include "ep/epavltree.h"


// TODO: remove this!
#if defined(EP_WINDOWS)
#pragma warning(disable: 4100)
#endif // defined(EP_WINDOWS)


#define PROTOTYPE_COMPONENT(Name) \
  SHARED_CLASS(Name)


namespace ep
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

  explicit operator bool() const { return shim != nullptr; }

  epVariant get(const ep::Component *pThis) const
  {
    return shim(this, pThis);
  }

protected:
  typedef epVariant(Shim)(const Getter* const, const ep::Component*);
  Shim *shim;
};

// setter glue
class Setter
{
public:
  Setter(nullptr_t) : shim(nullptr) {}

  explicit operator bool() const { return shim != nullptr; }

  void set(ep::Component *pThis, const epVariant &value) const
  {
    shim(this, pThis, value);
  }

protected:
  typedef void(Shim)(const Setter* const, ep::Component*, const epVariant&);
  Shim *shim;
};

// method glue
class Method
{
public:
  Method(nullptr_t) : shim(nullptr) {}

  explicit operator bool() const { return shim != nullptr; }

  epVariant call(ep::Component *pThis, epSlice<epVariant> args) const
  {
    return shim(this, pThis, args);
  }

protected:
  typedef epVariant(Shim)(const Method* const, ep::Component*, epSlice<epVariant>);
  Shim *shim;
};

// static funcion glue
class StaticFunc
{
public:
  StaticFunc(nullptr_t) : shim(nullptr) {}

  explicit operator bool() const { return shim != nullptr; }

  epVariant call(epSlice<epVariant> args) const
  {
    return shim(this, args);
  }

protected:
  typedef epVariant(Shim)(const StaticFunc* const, epSlice<epVariant>);
  Shim *shim;
};

// event glue
class VarEvent
{
public:
  VarEvent(nullptr_t) : pSubscribe(nullptr) {}

  explicit operator bool() const { return pSubscribe != nullptr; }

  void subscribe(const ep::ComponentRef &c, const epVariant::VarDelegate &d)
  {
    pSubscribe(this, c, d);
  }

protected:
  typedef void (SubscribeFunc)(const VarEvent*, const ComponentRef&, const epVariant::VarDelegate&);
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
  static epVariant shimFunc(const Getter * const pGetter, const ep::Component *pThis);
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
  static void shimFunc(const Setter * const pSetter, ep::Component *pThis, const epVariant &value);
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
    static epVariant callFuncHack(epSlice<epVariant> args, FastDelegate<Ret(Args...)> d, Sequence<S...>);

    static epVariant shimFunc(const Method * const pSetter, ep::Component *pThis, epSlice<epVariant> value);
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
    static epVariant callFuncHack(epSlice<epVariant> args, Ret(*f)(Args...), Sequence<S...>);

    static epVariant shimFunc(const StaticFunc * const pSetter, epSlice<epVariant> value);
  };
};

// event glue
class CEvent : public VarEvent
{
public:
  CEvent(nullptr_t) : VarEvent(nullptr) {}
  template<typename X, typename... Args>
  CEvent(epEvent<Args...> X::*ev);

protected:
  void* CEvent::*pEvent;

  template<typename X, typename... Args>
  static void doSubscribe(const VarEvent *pEv, const ComponentRef &c, const epVariant::VarDelegate &d);
};


// property description
enum PropertyFlags : uint32_t
{
  udPF_Immutable = 1<<0 // must be initialised during construction
};

struct EnumKVP
{
  EnumKVP(epString key, int64_t v) : key(key), value(v) {}

  epString key;
  int64_t value;
};
#define EnumKVP(e) EnumKVP( #e, (int64_t)e )

struct PropertyInfo
{
  PropertyInfo() = delete;
  PropertyInfo(epString id, epString displayName, epString description, epString displayType = nullptr, uint32_t flags = 0)
    : id(id), displayName(displayName), description(description), displayType(displayType), flags(flags) {}
  PropertyInfo(const PropertyInfo &rh)
    : id(rh.id), displayName(rh.displayName), description(rh.description), displayType(rh.displayType), flags(rh.flags) {}

  epString id;
  epString displayName;
  epString description;

  epString displayType;
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
  FunctionInfo(epString id, epString description)
    : id(id), description(description) {}
  FunctionInfo(const FunctionInfo &rh)
    : id(rh.id), description(rh.description) {}

  epString id;
  epString description;
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
  EventInfo(epString id, epString displayName, epString description)
    : id(id), displayName(displayName), description(description) {}
  EventInfo(const EventInfo &rh)
    : id(rh.id), displayName(rh.displayName), description(rh.description) {}

  epString id;
  epString displayName;
  epString description;
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
typedef Component *(CreateInstanceCallback)(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);

struct ComponentDesc
{
  ComponentDesc() = delete;
  ComponentDesc(ComponentDesc *pSuperDesc, int udVersion, int pluginVersion, epString id, epString displayName, epString description,
    epSlice<CPropertyDesc> properties = nullptr, epSlice<CMethodDesc> methods = nullptr, epSlice<CEventDesc> events = nullptr, epSlice<CStaticFuncDesc> staticFuncs = nullptr,
    InitComponent *pInit = nullptr, CreateInstanceCallback *pCreateInstance = nullptr)
    : pSuperDesc(pSuperDesc), udVersion(udVersion), pluginVersion(pluginVersion), id(id), displayName(displayName), description(description)
    , properties(properties), methods(methods), events(events), staticFuncs(staticFuncs), pInit(pInit), pCreateInstance(pCreateInstance) {}

  ComponentDesc& operator=(const ComponentDesc&) = delete;

  ComponentDesc *pSuperDesc;

  int udVersion;
  int pluginVersion;

  epString id;          // an id for this component
  epString displayName; // display name
  epString description; // description

  // icon image...

  // TODO: add flags ('Abstract' (can't create) flag)

  epSlice<CPropertyDesc> properties;
  epSlice<CMethodDesc> methods;
  epSlice<CEventDesc> events;
  epSlice<CStaticFuncDesc> staticFuncs;

  InitComponent *pInit;
  CreateInstanceCallback *pCreateInstance;

  epAVLTree<epString, PropertyDesc> propertyTree;
  epAVLTree<epString, MethodDesc> methodTree;
  epAVLTree<epString, EventDesc> eventTree;
  epAVLTree<epString, StaticFuncDesc> staticFuncTree;

  StaticFunc *ComponentDesc::GetStaticFunc(epString id) const;

  void BuildSearchTrees();
  void InitProps();
  void InitMethods();
  void InitEvents();
  void InitStaticFuncs();
};

} // namespace ep


#include "componentdesc.inl"

#endif // EPCOMPONENTDESC_H
