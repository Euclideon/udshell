#pragma once
#if !defined(_EP_DYNAMICCOMPONENT_H)
#define _EP_DYNAMICCOMPONENT_H

#include "ep/cpp/component/component.h"

namespace ep {

class DynamicComponent : public Component
{
  EP_DECLARE_COMPONENT(ep, DynamicComponent, Component, EPKERNEL_PLUGINVERSION, "Base class for dynamic components", 0)

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

  PropertyDesc* cachePropertyDesc(String _name, String description, const VarDelegate &getter, const VarDelegate &setter) const
  {
    PropertyDesc *pDesc = propertyCache.get(_name);
    if (pDesc)
    {
      // update the delegates to the one we just got
      pDesc->setter.data = setter.getMemento();
      pDesc->getter.data = getter.getMemento();
    }
    else
    {
      // build a property descriptor
      struct Shim
      {
        Variant call(Slice<const Variant> args, const RefCounted &data)
        {
          return ((VarDelegate&)data)(args);
        }
      };

      pDesc = &propertyCache.insert(_name, PropertyDesc({ _name, _name, description },
        MethodShim(&Shim::call, getter.getMemento()), MethodShim(&Shim::call, setter.getMemento())));
    }
    return pDesc;
  }

  MethodDesc* cacheMethodDesc(String _name, String description, const VarDelegate &method) const
  {
    MethodDesc *pDesc = methodCache.get(_name);
    if (pDesc)
    {
      // update the delegate to the one we just got
      pDesc->method.data = method.getMemento();
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
      pDesc = &methodCache.insert(_name, MethodDesc({ _name, description, nullptr }, MethodShim(&Shim::call, method.getMemento())));
    }
    return pDesc;
  }

  virtual void attachToGlue(Component *pGlue, Variant::VarMap initParams) { pThis = pGlue; }

  Component *pThis;

  mutable AVLTree<SharedString, PropertyDesc> propertyCache;
  mutable AVLTree<SharedString, MethodDesc> methodCache;
};

}

#endif // _EP_DYNAMICCOMPONENT_H
