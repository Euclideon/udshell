#pragma once
#ifndef UDCOMPONENT_H
#define UDCOMPONENT_H

#include "udPlatform.h"
#include "udHashMap.h"
#include "udString.h"
#include "udHelpers.h"
#include "udSharedPtr.h"

#include "3rdparty\FastDelegate.h"
using namespace fastdelegate;

#include <stdio.h>


#if defined(SendMessage)
# undef SendMessage
#endif

// TODO: remove this!
#if UDPLATFORM_WINDOWS
#pragma warning(disable: 4100)
#endif //

#define ARRAY_LENGTH(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define UD_COMPONENT(Name) \
  static const udComponentDesc descriptor; \
  typedef udSharedPtr<Name> Ref;

#define PROTOTYPE_COMPONENT(Name) \
  class Name; \
  typedef udSharedPtr<Name> Name##Ref;


enum { UDSHELL_APIVERSION = 100 };
enum { UDSHELL_PLUGINVERSION = UDSHELL_APIVERSION };


class udKernel;
class udComponent;
struct udComponentDesc;

PROTOTYPE_COMPONENT(udComponent);


typedef udResult (InitComponent)();
typedef udResult (InitRender)();
typedef udComponent *(CreateInstanceCallback)(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams);


enum class udComponentType : uint32_t
{
  Component,
  Command,
  View,
  Scene,
  Node,
  Camera,
  Gizmo,
  UI
};

enum class udPropertyType : uint32_t
{
  Boolean,
  Integer,
  Float,
  String,
  Component,
};

enum udPropertyFlags : uint32_t
{
  udPF_NoRead = 1<<0,
  udPF_NoWrite = 1<<1,
  udPF_MustInit = 1<<2,
  udPF_Immutable = udPF_MustInit | udPF_NoWrite | udPF_NoRead,
};

enum class udPropertyDisplayType : uint32_t
{
  Default
};

struct udGetter
{
public:
  udGetter(nullptr_t) {}
  template <class X, class Type>
  udGetter(udResult(X::*func)(Type))
  {
    m = MakeDelegate((X*)nullptr, func).GetMemento();
  }
  template<typename Type>
  udResult get(udComponent *pThis, Type &value) const
  {
    auto m = this->m;
    m.SetThis(pThis);
    FastDelegate<udResult(Type&)> d;
    d.SetMemento(m);
    return d(value);
  }
  template<typename Type>
  udResult getArray(udComponent *pThis, udSlice<Type> value) const
  {
    auto m = this->m;
    m.SetThis(pThis);
    FastDelegate<udResult(udSlice<Type>)> d;
    d.SetMemento(m);
    return d(value);
  }
protected:
  DelegateMemento m;
};
struct udSetter
{
public:
  udSetter(nullptr_t) {}
  template <class X, class Type>
  udSetter(udResult(X::*func)(Type))
  {
    m = MakeDelegate((X*)nullptr, func).GetMemento();
  }
  template<typename Type>
  udResult set(udComponent *pThis, Type value) const
  {
    auto m = this->m;
    m.SetThis(pThis);
    FastDelegate<udResult(Type)> d;
    d.SetMemento(m);
    return d(value);
  }
private:
  DelegateMemento m;
};

struct udPropertyDesc
{
  udString id;
  udString displayName;
  udString description;

  udPropertyType type;
  uint32_t arrayLength;

  uint32_t flags;

  udPropertyDisplayType displayType;

  udGetter getter;
  udSetter setter;
};

struct udComponentDesc
{
  const udComponentDesc *pSuperDesc;

  int udVersion;
  int pluginVersion;

  udComponentType type;

  udString id;          // an id for this component
  udString displayName; // display name
  udString description; // description

  // icon image...

//  this
  InitComponent *pInit;
  InitRender *pInitRender;
  CreateInstanceCallback *pCreateInstance;

  const udSlice<const udPropertyDesc> properties;
};


class udComponent : public udRefCounted
{
public:
  friend class udKernel;
  UD_COMPONENT(udComponent);

  const udComponentDesc* const pType;
  class udKernel* const pKernel;
  const udRCString uid;

  const udPropertyDesc *FindProperty(udString name);

  udResult SetProperty(udString property, udVariant value);

  udResult GetPropertyBool(udString property, bool &result);
  udResult GetPropertyInt(udString property, int64_t &result);
  udResult GetPropertyIntArray(udString property, udSlice<int64_t> result);
  udResult GetPropertyFloat(udString property, double &result);
  udResult GetPropertyFloatArray(udString property, udSlice<double> result);
  udResult GetPropertyString(udString property, udString &result);
  udResult GetPropertyStringArray(udString property, udSlice<udString> result);
  udResult GetPropertyComponent(udString property, udComponentRef &result);

  udResult SendMessage(udString target, udString message, udVariant data);
  udResult SendMessage(udComponent *pComponent, udString message, udVariant data) { return SendMessage(pComponent->uid, message, data); }

  // properties
  udResult getUid(udString &uid)
  {
    uid = this->uid;
    return udR_Success;
  }
  udResult setUid(udString uid)
  {
    return udR_Success;
  }


protected:
  udComponent(const udComponentDesc *_pType, udKernel *_pKernel, udRCString _uid, udInitParams initParams)
    : pType(_pType), pKernel(_pKernel), uid(_uid) {}
  virtual ~udComponent() {}

  virtual udResult ReceiveMessage(udString message, udString sender, udVariant data);

private:
  udComponent(const udComponent &) = delete;    // Still not sold on this
  void operator=(const udComponent &) = delete;
};


template<typename T>
inline udSharedPtr<T> component_cast(udComponentRef pComponent)
{
  if (!pComponent)
    return udSharedPtr<T>();
  const udComponentDesc *pDesc = pComponent->pType;
  while (pDesc)
  {
    if (pDesc->id.eq(T::descriptor.id))
      return static_pointer_cast<T>(pComponent);
    pDesc = pDesc->pSuperDesc;
  }
  return udSharedPtr<T>();
}

#endif // UDCOMPONENT_H
