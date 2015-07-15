#pragma once
#ifndef UDCOMPONENT_H
#define UDCOMPONENT_H

#include "udComponentDesc.h"
#include "udHashMap.h"

#include <stdio.h>

#if defined(SendMessage)
# undef SendMessage
#endif


#define UD_COMPONENT(Name) \
  static const udComponentDesc descriptor; \
  typedef udSharedPtr<Name> Ref;


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

  void SetProperty(udString property, const udVariant value);
  udVariant GetProperty(udString property) const;

  udResult SendMessage(udString target, udString message, udVariant data);
  udResult SendMessage(udComponent *pComponent, udString message, udVariant data) { return SendMessage(pComponent->uid, message, data); }

  // properties
  udString GetUid() const { return uid; }
  udString GetType() const { return descriptor.id; }
  udString GetDisplayName() const { return descriptor.displayName; }
  udString GetDescription() const { return descriptor.description; }

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
