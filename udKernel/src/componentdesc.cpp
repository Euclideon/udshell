#include "componentdesc.h"

namespace ud
{

void ComponentDesc::BuildSearchTree()
{
  if (pPropertyTree) // all components have properties
    return;

  if (pSuperDesc)
    pSuperDesc->BuildSearchTree();

  // count and assign indices
  size_t numProps = pSuperDesc && pSuperDesc->pPropertyTree ? pSuperDesc->pPropertyTree->Size() : 0;
  size_t numMethods = pSuperDesc && pSuperDesc->pMethodTree ? pSuperDesc->pMethodTree->Size() : 0;
  size_t numEvents = pSuperDesc && pSuperDesc->pEventTree ? pSuperDesc->pEventTree->Size() : 0;

  for (size_t i = 0; i<properties.length; ++i)
    properties[i].index = (uint32_t)(numProps + i);
  for (size_t i = 0; i<methods.length; ++i)
    methods[i].index = (uint32_t)(numMethods + i);
  for (size_t i = 0; i<events.length; ++i)
    events[i].index = (uint32_t)(numEvents + i);

  numProps += properties.length;
  numMethods += methods.length;
  numEvents += events.length;

  // allocate the search trees
  if (numProps)
  {
    pPropertyTree = udNew(udMap<PropertyNode>, numProps, [](const PropertyNode &a, const PropertyNode &b) {
      return a.id.cmp(b.id);
    });
  }
  if (numMethods)
  {
    pMethodTree = udNew(udMap<MethodNode>, numMethods, [](const MethodNode &a, const MethodNode &b) {
      return a.id.cmp(b.id);
    });
  }
  if (numEvents)
  {
    pEventTree = udNew(udMap<EventNode>, numEvents, [](const EventNode &a, const EventNode &b) {
      return a.id.cmp(b.id);
    });
  }

  // build the search trees
  ComponentDesc *pDesc = this;
  while (pDesc)
  {
    for (size_t i = 0; i<pDesc->properties.length; ++i)
      pPropertyTree->Insert(PropertyNode(&pDesc->properties[i]));
    for (size_t i = 0; i<pDesc->methods.length; ++i)
      pMethodTree->Insert(MethodNode(&pDesc->methods[i]));
    for (size_t i = 0; i<pDesc->events.length; ++i)
      pEventTree->Insert(EventNode(&pDesc->events[i]));
    pDesc = pDesc->pSuperDesc;
  }
}

} // namespace ud
