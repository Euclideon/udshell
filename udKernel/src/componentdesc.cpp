#include "componentdesc.h"

namespace ud
{

void ComponentDesc::BuildSearchTree()
{
  if (propertyTree.Size() > 0) // all components have properties
    return;

  if (pSuperDesc)
    pSuperDesc->BuildSearchTree();

  // count and assign indices
  size_t numProps = pSuperDesc ? pSuperDesc->propertyTree.Size() : 0;
  size_t numMethods = pSuperDesc ? pSuperDesc->methodTree.Size() : 0;
  size_t numEvents = pSuperDesc ? pSuperDesc->eventTree.Size() : 0;

  for (size_t i = 0; i<properties.length; ++i)
    properties[i].index = (uint32_t)(numProps + i);
  for (size_t i = 0; i<methods.length; ++i)
    methods[i].index = (uint32_t)(numMethods + i);
  for (size_t i = 0; i<events.length; ++i)
    events[i].index = (uint32_t)(numEvents + i);

  numProps += properties.length;
  numMethods += methods.length;
  numEvents += events.length;

  // build the search trees
  ComponentDesc *pDesc = this;
  while (pDesc)
  {
    for (auto &p : pDesc->properties)
    {
      if (!propertyTree.Get(p.info.id))
        propertyTree.Insert(p.info.id, &p);
    }
    for (auto &m : pDesc->methods)
    {
      if (!methodTree.Get(m.info.id))
        methodTree.Insert(m.info.id, &m);
    }
    for (auto &e : pDesc->events)
    {
      if (!eventTree.Get(e.info.id))
        eventTree.Insert(e.info.id, &e);
    }
    pDesc = pDesc->pSuperDesc;
  }
}

} // namespace ud
