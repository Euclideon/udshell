#pragma once
#ifndef EP_PROJECT_H
#define EP_PROJECT_H

#include "component.h"

namespace ep
{

PROTOTYPE_COMPONENT(Project);
SHARED_CLASS(Activity);
SHARED_CLASS(ResourceManager);

class Project : public Component
{
public:
  EP_COMPONENT(Project);

  void SaveProject();

  Slice<ActivityRef> GetActivities() const { return activities; }
  void AddActivity(ActivityRef activity) { activities.pushBack(activity); }
  void RemoveActivity(ActivityRef activity) { activities.remove(activities.findFirst(activity)); }

protected:
  Project(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

  uint32_t GetLineNumberFromByteIndex(Slice<char> buffer, size_t index) const;

  Variant SaveActivities();

  void ParseProject(Variant values);
  void ParseActivities(Variant values);
  void ParseActivity(String type, Variant values);

  Array<ActivityRef> activities;

  ResourceManagerRef spResourceManager;
  SharedString srcString;
};

} //namespace ep

#endif // EP_PROJECT_H
