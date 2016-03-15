#pragma once
#if !defined(_EP_DYNAMICCOMPONENT_H)
#define _EP_DYNAMICCOMPONENT_H

#include "ep/cpp/component/component.h"

namespace ep {

class DynamicComponent : public Component
{
  EP_DECLARE_COMPONENT(DynamicComponent, Component, EPKERNEL_PLUGINVERSION, "Base class for dynamic components", 0)

protected:
  friend class KernelImpl;

  DynamicComponent(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams)
    : Component(_pType, _pKernel, _uid, initParams)
  {
    pThis = this;
  }
  ~DynamicComponent()
  {
  }

  MethodDesc* CacheMethodDesc(String _name, String description, const VarDelegate &method) const
  {
    MethodDesc *pDesc = methodCache.Get(_name);
    if (pDesc)
    {
      // update the delegate to the one we just got
      pDesc->method.data = method.GetMemento();
    }
    else
    {
      // build a method descriptor
      struct Shim
      {
        Variant call(Slice<const Variant> args, const RefCounted &data)
        {
          return ((VarDelegate&)data)(args);
        }
      };
      pDesc = &methodCache.Insert(_name, MethodDesc({ _name, description, nullptr }, MethodShim(&Shim::call, method.GetMemento())));
    }
    return pDesc;
  }

  Component *pThis;

  mutable AVLTree<SharedString, PropertyDesc> propertyCache;
  mutable AVLTree<SharedString, MethodDesc> methodCache;
};

}

#endif // _EP_DYNAMICCOMPONENT_H
