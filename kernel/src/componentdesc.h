#pragma once
#ifndef EPCOMPONENTDESC_H
#define EPCOMPONENTDESC_H

#include "ep/cpp/componentdesc.h"
#include "ep/cpp/platform.h"
#include "ep/epversion.h"
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/event.h"
#include "ep/cpp/map.h"


struct epComponentDesc;

namespace ep {

SHARED_CLASS(Component);

// property description
enum PropertyFlags : uint32_t
{
  epPF_Immutable = 1<<0 // must be initialised during construction
};

struct EnumKVP
{
  EnumKVP(String key, int64_t v) : key(key), value(v) {}

  String key;
  int64_t value;
};
#define EnumKVP(e) EnumKVP( #e, (int64_t)e )

}

namespace kernel {

class Kernel;
struct ComponentDesc;

// interface for getters, setters, methods, events

// getter glue
class GetterShim
{
public:
  GetterShim(void *pGetter, SharedPtr<const RefCounted> data = nullptr) : pGetter(pGetter), data(data) {}

  explicit operator bool() const { return pGetter != nullptr; }

  Variant get(const ep::Component *pThis) const;

protected:
  void *pGetter;
  SharedPtr<const RefCounted> data;
};

// setter glue
class SetterShim
{
public:
  SetterShim(void *pSetter, SharedPtr<const RefCounted> data = nullptr) : pSetter(pSetter), data(data) {}

  explicit operator bool() const { return pSetter != nullptr; }

  void set(ep::Component *pThis, const Variant &value) const;

protected:
  void *pSetter;
  SharedPtr<const RefCounted> data;
};

// method glue
class MethodShim
{
public:
  MethodShim(void *pMethod, SharedPtr<const RefCounted> data = nullptr) : pMethod(pMethod), data(data) {}

  Variant call(ep::Component *pThis, Slice<const Variant> args) const;

protected:
  void *pMethod;
  SharedPtr<const RefCounted> data;
};

// static funcion glue
class StaticFuncShim
{
public:
  StaticFuncShim(void *pFunc, SharedPtr<const RefCounted> data = nullptr) : pFunc(pFunc), data(data) {}

  Variant call(Slice<const Variant> args) const;

protected:
  void *pFunc;
  SharedPtr<const RefCounted> data;
};

// event glue
class EventShim
{
public:
  EventShim(void *pSubscribe, SharedPtr<const RefCounted> data = nullptr) : pSubscribe(pSubscribe), data(data) {}

  void subscribe(ep::Component *pThis, const Variant::VarDelegate &d) const;

protected:
  void *pSubscribe;
  SharedPtr<const RefCounted> data;
};


struct PropertyDesc : public PropertyInfo
{
  PropertyDesc(const PropertyInfo &info, const GetterShim &getter, const SetterShim &setter)
    : PropertyInfo(info), getter(getter), setter(setter)
  {}

  GetterShim getter;
  SetterShim setter;
};

struct MethodDesc : public MethodInfo
{
  MethodDesc(const MethodInfo &desc, const MethodShim &method)
    : MethodInfo(desc), method(method)
  {}

  MethodShim method;
};

struct StaticFuncDesc : public StaticFuncInfo
{
  StaticFuncDesc(const StaticFuncInfo &desc, const StaticFuncShim &staticFunc)
    : StaticFuncInfo(desc), staticFunc(staticFunc)
  {}

  StaticFuncShim staticFunc;
};

struct EventDesc : public EventInfo
{
  EventDesc(const EventInfo &desc, const EventShim &ev)
    : EventInfo(desc), ev(ev)
  {}

  EventShim ev;
};


// component description
struct ComponentDesc : public ep::ComponentDesc
{
  ComponentDesc(const ep::ComponentDesc &desc);

  AVLTree<String, PropertyDesc> propertyTree;
  AVLTree<String, MethodDesc> methodTree;
  AVLTree<String, EventDesc> eventTree;
  AVLTree<String, StaticFuncDesc> staticFuncTree;

  const StaticFuncShim* GetStaticFunc(String id) const;
};

} // namespace kernel


#include "componentdesc.inl"

#endif // EPCOMPONENTDESC_H
