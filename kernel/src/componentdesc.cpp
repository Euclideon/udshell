#include "componentdesc.h"

namespace ep
{

void ComponentDesc::InitProps()
{
  ComponentDesc *desc = this;
  while (desc)
  {
    for (auto &p : desc->properties)
    {
      if (!propertyTree.Get(p.info.id))
        propertyTree.Insert(p.info.id, { p.info, &p.getter, &p.setter });
    }
    desc = desc->pSuperDesc;
  }
}

void ComponentDesc::InitMethods()
{
  ComponentDesc *desc = this;
  while (desc)
  {
    for (auto &m : desc->methods)
    {
      if (!methodTree.Get(m.info.id))
        methodTree.Insert(m.info.id, { m.info, &m.method });
    }
    desc = desc->pSuperDesc;
  }
}

void ComponentDesc::InitStaticFuncs()
{
  ComponentDesc *desc = this;
  while (desc)
  {
    for (auto &m : desc->staticFuncs)
    {
      if (!staticFuncTree.Get(m.info.id))
        staticFuncTree.Insert(m.info.id, { m.info, &m.staticFunc });
    }
    desc = desc->pSuperDesc;
  }
}

void ComponentDesc::InitEvents()
{
  ComponentDesc *desc = this;
  while (desc)
  {
    for (auto &e : desc->events)
    {
      if (!eventTree.Get(e.info.id))
        eventTree.Insert(e.info.id, { e.info, &e.ev });
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

StaticFunc *ComponentDesc::GetStaticFunc(epString id) const
{
  const StaticFuncDesc *pFuncDesc;
  const ComponentDesc *pCompDesc = this;

  while(pCompDesc)
  {
    pFuncDesc = pCompDesc->staticFuncTree.Get(id);
    if (pFuncDesc)
      return pFuncDesc->staticFunc;

    pCompDesc = pCompDesc->pSuperDesc;
  }

  return nullptr;
}

} // namespace ep
