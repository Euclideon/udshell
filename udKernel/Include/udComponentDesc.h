#pragma once
#ifndef UDCOMPONENTDESC_H
#define UDCOMPONENTDESC_H

#include "udPlatform.h"
#include "udString.h"
#include "udSharedPtr.h"
#include "udVariant.h"

#include "3rdparty\FastDelegate.h"
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
  class Name; \
  typedef udSharedPtr<Name> Name##Ref;


class udKernel;
class udComponent;
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
  struct Partial
  {
    static udVariant shimFunc(const udMethod * const pSetter, udComponent *pThis, udSlice<udVariant> value);
  };
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

  udPropertyType type;
  uint32_t arrayLength;
  const udSlice<const udEnumKVP> kvp;
};

struct udPropertyDesc
{
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
  udString id;
  udString displayName;
  udString description;

  udMethod method;

  udTypeDesc result;
  const udSlice<const udTypeDesc> args;
};


// component description
enum { UDSHELL_APIVERSION = 100 };
enum { UDSHELL_PLUGINVERSION = UDSHELL_APIVERSION };

typedef udResult(InitComponent)();
typedef udResult(InitRender)();
typedef udComponent *(CreateInstanceCallback)(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams);

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
  const udSlice<const udMethodDesc> methods;
};


#include "udComponentDesc.inl"

#endif // UDCOMPONENTDESC_H
