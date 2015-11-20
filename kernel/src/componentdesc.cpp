#include "componentdesc.h"

namespace kernel
{

ComponentDesc::ComponentDesc(const ep::ComponentDesc &desc)
  : ep::ComponentDesc(desc)
{
  // build search trees
  const ep::ComponentDesc *pDesc = this;
  while (pDesc)
  {
    for (auto &p : pDesc->properties)
    {
      if (!propertyTree.Get(p.id))
        propertyTree.Insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
    }
    for (auto &m : pDesc->methods)
    {
      if (!methodTree.Get(m.id))
        methodTree.Insert(m.id, { m, m.pMethod });
    }
    for (auto &f : pDesc->staticFuncs)
    {
      if (!staticFuncTree.Get(f.id))
        staticFuncTree.Insert(f.id, { f, (void*)f.pCall });
    }
    for (auto &e : pDesc->events)
    {
      if (!eventTree.Get(e.id))
        eventTree.Insert(e.id, { e, e.pSubscribe });
    }
    pDesc = pDesc->pSuperDesc;
  }
}

const StaticFuncShim* ComponentDesc::GetStaticFunc(String _id) const
{
  const StaticFuncDesc *pFuncDesc;
  const ComponentDesc *pCompDesc = this;

  while(pCompDesc)
  {
    pFuncDesc = pCompDesc->staticFuncTree.Get(_id);
    if (pFuncDesc)
      return &pFuncDesc->staticFunc;
    pCompDesc = (const ComponentDesc*)pCompDesc->pSuperDesc;
  }

  return nullptr;
}

} // namespace kernel
