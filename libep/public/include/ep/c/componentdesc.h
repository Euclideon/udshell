#if !defined(_EP_COMPONENTDESC_H)
#define _EP_COMPONENTDESC_H

#include "ep/c/variant.h"
#include "ep/c/delegate.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct epComponent;

typedef epVariant(epGetter)(const epComponent *pBaseComponent, const void *pDerivedInstance);
typedef void(epSetter)(epComponent *pBaseComponent, void *pDerivedInstance, const epVariant *pValue);
typedef epVariant(epMethodCall)(const epComponent *pBaseComponent, const void *pDerivedInstance, const epVariant *pArgs, size_t numArgs);
typedef epVariant(epStaticCall)(const epVariant *pValue, size_t numArgs);
typedef epVariant(epSubscribe)(const epComponent *pBaseComponent, const void *pDerivedInstance, epVarDelegate *pDelegate);

struct epPropertyDesc
{
  const char *id;
  const char *displayName;
  const char *description;

  const char *displayType;
  uint32_t flags;

  epGetter *pGetter;
  epSetter *pSetter;

  void *pGetterMethod;
  void *pSetterMethod;
};
struct epMethodDesc
{
  const char *id;
  const char *description;

  epMethodCall *pCall;

  void *pMethod;
};
struct epEventDesc
{
  const char *id;
  const char *displayName;
  const char *description;

  epSubscribe *pSubscribe;
};
struct epStaticFuncDesc
{
  const char *id;
  const char *description;

  epStaticCall *pCall;
};

struct epComponentInfo
{
  int pluginVersion;

  const char *id;          // an id for this component
  const char *displayName; // display name
  const char *description; // description
};

struct epComponentDesc
{
  epComponentInfo info;

  const char *baseClass;
  const struct epComponentOverrides *pOverrides;

  size_t numProperties;
  const epPropertyDesc *pProperties;
  size_t numMethods;
  const epMethodDesc *pMethods;
  size_t numEvents;
  const epEventDesc *pEvents;
  size_t numStaticFuncs;
  const epStaticFuncDesc *pStaticFuncs;
};

#if defined(__cplusplus)
}
#endif

#endif
