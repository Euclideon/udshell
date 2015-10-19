#pragma once
#ifndef EPCOMPONENTDESC_H
#define EPCOMPONENTDESC_H

#include "ep/cpp/platform.h"
#include "ep/epversion.h"
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/event.h"
#include "ep/cpp/avltree.h"


// TODO: remove this!
#if defined(EP_WINDOWS)
#pragma warning(disable: 4100)
#endif // defined(EP_WINDOWS)


#define PROTOTYPE_COMPONENT(Name) \
  SHARED_CLASS(Name)

struct epComponentDesc;

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

  Variant get(const Component *pThis) const
  {
    return shim(this, pThis);
  }

protected:
  typedef Variant(Shim)(const Getter* const, const Component*);
  Shim *shim;
};

// setter glue
class Setter
{
public:
  Setter(nullptr_t) : shim(nullptr) {}

  explicit operator bool() const { return shim != nullptr; }

  void set(Component *pThis, const Variant &value) const
  {
    shim(this, pThis, value);
  }

protected:
  typedef void(Shim)(const Setter* const, Component*, const Variant&);
  Shim *shim;
};

// method glue
class Method
{
public:
  Method(nullptr_t) : shim(nullptr) {}

  explicit operator bool() const { return shim != nullptr; }

  Variant call(Component *pThis, Slice<Variant> args) const
  {
    return shim(this, pThis, args);
  }

protected:
  typedef Variant(Shim)(const Method* const, Component*, Slice<Variant>);
  Shim *shim;
};

// static funcion glue
class StaticFunc
{
public:
  StaticFunc(nullptr_t) : shim(nullptr) {}

  explicit operator bool() const { return shim != nullptr; }

  Variant call(Slice<Variant> args) const
  {
    return shim(this, args);
  }

protected:
  typedef Variant(Shim)(const StaticFunc* const, Slice<Variant>);
  Shim *shim;
};

// event glue
class VarEvent
{
public:
  VarEvent(nullptr_t) : pSubscribe(nullptr) {}

  explicit operator bool() const { return pSubscribe != nullptr; }

  void subscribe(const ComponentRef &c, const Variant::VarDelegate &d)
  {
    pSubscribe(this, c, d);
  }

protected:
  typedef void (SubscribeFunc)(const VarEvent*, const ComponentRef&, const Variant::VarDelegate&);
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
  static Variant shimFunc(const Getter * const pGetter, const Component *pThis);
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
  static void shimFunc(const Setter * const pSetter, Component *pThis, const Variant &value);
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
    static Variant callFuncHack(Slice<Variant> args, FastDelegate<Ret(Args...)> d, internal::Sequence<S...>);

    static Variant shimFunc(const Method * const pSetter, Component *pThis, Slice<Variant> value);
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
    static Variant callFuncHack(Slice<Variant> args, Ret(*f)(Args...), internal::Sequence<S...>);

    static Variant shimFunc(const StaticFunc * const pSetter, Slice<Variant> value);
  };
};

// event glue
class CEvent : public VarEvent
{
public:
  CEvent(nullptr_t) : VarEvent(nullptr) {}
  template<typename X, typename... Args>
  CEvent(Event<Args...> X::*ev);

protected:
  void* CEvent::*pEvent;

  template<typename X, typename... Args>
  static void doSubscribe(const VarEvent *pEv, const ComponentRef &c, const Variant::VarDelegate &d);
};


// property description
enum PropertyFlags : uint32_t
{
  udPF_Immutable = 1<<0 // must be initialised during construction
};

struct EnumKVP
{
  EnumKVP(String key, int64_t v) : key(key), value(v) {}

  String key;
  int64_t value;
};
#define EnumKVP(e) EnumKVP( #e, (int64_t)e )

struct PropertyInfo
{
  PropertyInfo() = delete;
  PropertyInfo(String id, String displayName, String description, String displayType = nullptr, uint32_t flags = 0)
    : id(id), displayName(displayName), description(description), displayType(displayType), flags(flags) {}
  PropertyInfo(const PropertyInfo &rh)
    : id(rh.id), displayName(rh.displayName), description(rh.description), displayType(rh.displayType), flags(rh.flags) {}

  String id;
  String displayName;
  String description;

  String displayType;
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
  FunctionInfo(String id, String description)
    : id(id), description(description) {}
  FunctionInfo(const FunctionInfo &rh)
    : id(rh.id), description(rh.description) {}

  String id;
  String description;
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
  EventInfo(String id, String displayName, String description)
    : id(id), displayName(displayName), description(description) {}
  EventInfo(const EventInfo &rh)
    : id(rh.id), displayName(rh.displayName), description(rh.description) {}

  String id;
  String displayName;
  String description;
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
typedef epResult(InitComponent)(Kernel*);
typedef Component *(CreateInstanceCallback)(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

struct ComponentDesc
{
  ComponentDesc() = delete;
  ComponentDesc(ComponentDesc *pSuperDesc, int epVersion, int pluginVersion, String id, String displayName, String description,
    Slice<CPropertyDesc> properties = nullptr, Slice<CMethodDesc> methods = nullptr, Slice<CEventDesc> events = nullptr, Slice<CStaticFuncDesc> staticFuncs = nullptr,
    InitComponent *pInit = nullptr, CreateInstanceCallback *pCreateInstance = nullptr, const epComponentDesc *pExternalDesc = nullptr)
    : pSuperDesc(pSuperDesc), epVersion(epVersion), pluginVersion(pluginVersion), id(id), displayName(displayName), description(description)
    , properties(properties), methods(methods), events(events), staticFuncs(staticFuncs), pInit(pInit), pCreateInstance(pCreateInstance), pExternalDesc(pExternalDesc) {}

  ComponentDesc& operator=(const ComponentDesc&) = delete;

  ComponentDesc *pSuperDesc;

  int epVersion;
  int pluginVersion;

  String id;          // an id for this component
  String displayName; // display name
  String description; // description

  // icon image...

  // TODO: add flags ('Abstract' (can't create) flag)

  Slice<CPropertyDesc> properties;
  Slice<CMethodDesc> methods;
  Slice<CEventDesc> events;
  Slice<CStaticFuncDesc> staticFuncs;

  InitComponent *pInit;
  CreateInstanceCallback *pCreateInstance;

  const epComponentDesc *pExternalDesc;

  AVLTree<String, PropertyDesc> propertyTree;
  AVLTree<String, MethodDesc> methodTree;
  AVLTree<String, EventDesc> eventTree;
  AVLTree<String, StaticFuncDesc> staticFuncTree;

  StaticFunc *GetStaticFunc(String id) const;

  void BuildSearchTrees();
  void InitProps();
  void InitMethods();
  void InitEvents();
  void InitStaticFuncs();
};

} // namespace ep


#include "componentdesc.inl"

#endif // EPCOMPONENTDESC_H
