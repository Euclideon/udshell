#pragma once
#ifndef UDCOMPONENT_H
#define UDCOMPONENT_H

#include "udPlatform.h"
#include "udHashMap.h"
#include "udString.h"
#include "udHelpers.h"

#include <stdio.h>

#if defined(SendMessage)
# undef SendMessage
#endif

enum { UDSHELL_APIVERSION = 100 };
enum { UDSHELL_PLUGINVERSION = UDSHELL_APIVERSION };


// TODO: remove this!
#if UDPLATFORM_WINDOWS
#pragma warning(disable: 4100)
#endif //

class udKernel;
class udComponent;
struct udComponentDesc;


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
  String
};

enum udPropertyFlags : uint32_t
{
  udPF_NoRead = 1 << 0,
  udPF_NoWrite = 1 << 1,
  udPF_OnlyInit = 1 << 2,
};

enum class udPropertyDisplayType : uint32_t
{
  Default
};

struct udPropertyDesc
{
  const char *id;
  const char *displayName;
  const char *description;

  udPropertyType type;
  uint32_t arrayLength;

  uint32_t flags;

  udPropertyDisplayType displayType;
};

struct udComponentDesc
{
  int udVersion;
  int pluginVersion;

  udComponentType type;

  const char *id;          // an id for this component
  const char *parentId;    // the id of the parent class
  const char *displayName; // display name
  const char *description; // description

  // icon image...

//  this
  InitComponent *pInit;
  InitRender *pInitRender;
  CreateInstanceCallback *pCreateInstance;

  const udPropertyDesc *pProperties;
  size_t numProperties;
};


class udComponent
{
public:
  friend class udKernel;

  const udComponentDesc * const pType;
  class udKernel * const pKernel;
  const udRCString uid;

  virtual udResult SetProperty(udString property, udString value) { return udR_Success; }
  virtual udResult GetProperty(udString property, udSlice<char> *pBuffer) { return udR_Success; }

  virtual udResult ReceiveMessage(udString message, udString sender, udString data);

  udResult SendMessage(udString target, udString message, udString data);
  udResult SendMessage(udComponent *pComponent, udString message, udString data) { return SendMessage(pComponent->uid, message, data); }

  // convenience overloads
  udResult SetProperty(udString property, int64_t value);
  udResult GetProperty(udString property, int64_t *pValue);
  udResult SetProperty(udString property, double value);
  udResult GetProperty(udString property, double *pValue);
  udResult SetProperty(udString property, udComponent *pComponent);
  udResult GetProperty(udString property, udComponent **ppComponent);

  static const udComponentDesc descriptor;

protected:
  udComponent(const udComponentDesc *_pType, udKernel *_pKernel, udRCString _uid, udInitParams initParams)
    : pType(_pType), pKernel(_pKernel), uid(_uid) {}
  virtual ~udComponent() {}

private:
  udComponent(const udComponent &) = delete;    // Still not sold on this
  void operator=(const udComponent &) = delete;
};

template<typename T>
inline T* component_cast(udComponent *pComponent)
{
  // TODO: check that pComponent is derived from T...
  return (T*)pComponent;
}

#endif // UDCOMPONENT_H
