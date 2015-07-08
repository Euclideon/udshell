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


struct udGetter
{
public:
  udGetter(nullptr_t) {}
  template <class X, class Type>
  udGetter(udResult(X::*func)(Type) const)
  {
    m = MakeDelegate((X*)nullptr, func).GetMemento();
  }
  template<typename Type>
  udResult get(const udComponent *pThis, Type &value) const
  {
    auto m = this->m;
    m.SetThis((void*)pThis);
    FastDelegate<udResult(Type&)> d;
    d.SetMemento(m);
    return d(value);
  }
  template<typename Type>
  udResult getArray(const udComponent *pThis, udSlice<Type> value) const
  {
    auto m = this->m;
    m.SetThis((void*)pThis);
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


typedef udResult (InitComponent)();
typedef udResult (InitRender)();
typedef udComponent *(CreateInstanceCallback)(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams);


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

  bool IsType(udString type) const;
  template<typename T>
  bool IsType() const { return IsType(T::descriptor.id); }

  const udPropertyDesc *FindProperty(udString name) const;

  udResult SetProperty(udString property, udVariant value);

  udResult GetPropertyBool(udString property, bool &result) const;
  udResult GetPropertyInt(udString property, int64_t &result) const;
  udResult GetPropertyIntArray(udString property, udSlice<int64_t> result) const;
  udResult GetPropertyFloat(udString property, double &result) const;
  udResult GetPropertyFloatArray(udString property, udSlice<double> result) const;
  udResult GetPropertyString(udString property, udString &result) const;
  udResult GetPropertyStringArray(udString property, udSlice<udString> result) const;
  udResult GetPropertyComponent(udString property, udComponentRef &result) const;

  udResult SendMessage(udString target, udString message, udVariant data);
  udResult SendMessage(udComponent *pComponent, udString message, udVariant data) { return SendMessage(pComponent->uid, message, data); }

  // properties
  udResult getUid(udString &uid) const { uid = this->uid; return udR_Success; }
  udResult getType(udString &type) const { type = this->descriptor.id; return udR_Success; }
  udResult getDisplayName(udString &displayName) const { displayName = this->descriptor.displayName; return udR_Success; }
  udResult getDescription(udString &description) const { description = this->descriptor.description; return udR_Success; }

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
