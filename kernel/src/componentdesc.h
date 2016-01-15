#pragma once
#ifndef EPCOMPONENTDESC_H
#define EPCOMPONENTDESC_H

#include "ep/version.h"
#include "ep/cpp/componentdesc.h"
#include "ep/cpp/platform.h"
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

#endif // EPCOMPONENTDESC_H
