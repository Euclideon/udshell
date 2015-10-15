#if !defined(_EP_COMPONENTDESC_H)
#define _EP_COMPONENTDESC_H

#include "ep/c/variant.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct epComponent;
struct epVarDelegate;

typedef epVariant(epGetter)(const epComponent *pBaseComponent, const void *pDerivedInstance);
typedef void(epSetter)(epComponent *pBaseComponent, void *pDerivedInstance, const epVariant *pValue);
typedef epVariant(epMethodCall)(const epComponent *pBaseComponent, const void *pDerivedInstance, const epVariant *pArgs, size_t numArgs);
typedef epVariant(epStaticCall)(const epVariant *pValue, size_t numArgs);
typedef epVariant(epSubscribe)(const epComponent *pBaseComponent, const void *pDerivedInstance, epVarDelegate *pDelegate);

// C delegate api
typedef epVariant(epVarMethod)(void *pThis, const epVariant *pArgs, size_t numArgs);

size_t epVarDelegate_Acquire(epVarDelegate *pDelegate);
size_t epVarDelegate_Release(epVarDelegate *pDelegate);
epVariant epVarDelegate_Call(epVarDelegate *pDelegate, const epVariant *pArgs, size_t numArgs);
epVarDelegate* epVarDelegate_Make(void *pThis, epVarMethod *pFunc);


struct epPropertyDesc
{
  const char *id;
  const char *displayName;
  const char *description;

  const char *displayType;
  uint32_t flags;

  epGetter *pGetter;
  epSetter *pSetter;
};
struct epMethodDesc
{
  const char *id;
  const char *description;

  epMethodCall *pCall;
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

struct epComponentDesc
{
  int pluginVersion;

  const char *id;          // an id for this component
  const char *displayName; // display name
  const char *description; // description

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
