#include "componentdesc.h"
#include "ep/c/componentdesc.h"

namespace ep
{

// HAX: just extern here, otherwise we need another header for this...
PropertyDesc* MakePluginPropertyDesc(const epPropertyDesc &prop);
MethodDesc* MakePluginMethodDesc(const epMethodDesc &method);
StaticFuncDesc* MakePluginStaticFuncDesc(const epStaticFuncDesc &func);
EventDesc* MakePluginEventDesc(const epEventDesc &ev);

void ComponentDesc::InitProps()
{
  const ComponentDesc *desc = this;
  while (desc)
  {
    if (desc->pExternalDesc)
    {
      for (size_t i = 0; i < desc->pExternalDesc->numProperties; ++i)
      {
        const epPropertyDesc &prop = desc->pExternalDesc->pProperties[i];
        if (!propertyTree.Get(prop.id))
        {
          PropertyDesc *p = MakePluginPropertyDesc(prop);
          propertyTree.Insert(prop.id, *p);
        }
      }
    }
    else
    {
      for (auto &p : desc->properties)
      {
        if (!propertyTree.Get(p.info.id))
          propertyTree.Insert(p.info.id, { p.info, &p.getter, &p.setter });
      }
    }
    desc = desc->pSuperDesc;
  }
}

void ComponentDesc::InitMethods()
{
  const ComponentDesc *desc = this;
  while (desc)
  {
    if (desc->pExternalDesc)
    {
      for (size_t i = 0; i < desc->pExternalDesc->numMethods; ++i)
      {
        const epMethodDesc &method = desc->pExternalDesc->pMethods[i];
        if (!methodTree.Get(method.id))
        {
          MethodDesc *m = MakePluginMethodDesc(method);
          methodTree.Insert(method.id, *m);
        }
      }
    }
    else
    {
      for (auto &m : desc->methods)
      {
        if (!methodTree.Get(m.info.id))
          methodTree.Insert(m.info.id, { m.info, &m.method });
      }
    }
    desc = desc->pSuperDesc;
  }
}

void ComponentDesc::InitStaticFuncs()
{
  const ComponentDesc *desc = this;
  while (desc)
  {
    if (desc->pExternalDesc)
    {
      for (size_t i = 0; i < desc->pExternalDesc->numStaticFuncs; ++i)
      {
        const epStaticFuncDesc &func = desc->pExternalDesc->pStaticFuncs[i];
        if (!staticFuncTree.Get(func.id))
        {
          StaticFuncDesc *f = MakePluginStaticFuncDesc(func);
          staticFuncTree.Insert(func.id, *f);
        }
      }
    }
    else
    {
      for (auto &m : desc->staticFuncs)
      {
        if (!staticFuncTree.Get(m.info.id))
          staticFuncTree.Insert(m.info.id, { m.info, &m.staticFunc });
      }
    }
    desc = desc->pSuperDesc;
  }
}

void ComponentDesc::InitEvents()
{
  const ComponentDesc *desc = this;
  while (desc)
  {
    if (desc->pExternalDesc)
    {
      for (size_t i = 0; i < desc->pExternalDesc->numEvents; ++i)
      {
        const epEventDesc &ev = desc->pExternalDesc->pEvents[i];
        if (!eventTree.Get(ev.id))
        {
          EventDesc *e = MakePluginEventDesc(ev);
          eventTree.Insert(ev.id, *e);
        }
      }
    }
    else
    {
      for (auto &e : desc->events)
      {
        if (!eventTree.Get(e.info.id))
          eventTree.Insert(e.info.id, { e.info, &e.ev });
      }
    }
    desc = desc->pSuperDesc;
  }
}

void ComponentDesc::BuildSearchTrees()
{
  if (propertyTree.Size() > 0) // all components have properties
    return;

  if (pSuperDesc)
    pSuperDesc->BuildSearchTrees();

  InitProps();
  InitMethods();
  InitEvents();
  InitStaticFuncs();
}

StaticFunc *ComponentDesc::GetStaticFunc(String _id) const
{
  const StaticFuncDesc *pFuncDesc;
  const ComponentDesc *pCompDesc = this;

  while(pCompDesc)
  {
    pFuncDesc = pCompDesc->staticFuncTree.Get(_id);
    if (pFuncDesc)
      return pFuncDesc->staticFunc;

    pCompDesc = pCompDesc->pSuperDesc;
  }

  return nullptr;
}

} // namespace ep
