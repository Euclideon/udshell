#pragma once
#ifndef EP_PROJECT_H
#define EP_PROJECT_H

#include "component.h"

namespace kernel
{

PROTOTYPE_COMPONENT(Project);
SHARED_CLASS(Activity);
SHARED_CLASS(ResourceManager);

class Project : public Component
{
  EP_DECLARE_COMPONENT(Project, Component, EPKERNEL_PLUGINVERSION, "Contains the state of a project")
public:

  void SaveProject();

  Slice<ActivityRef> GetActivities() const { return activities; }
  void AddActivity(ActivityRef activity) { activities.pushBack(activity); }
  void RemoveActivity(ActivityRef activity) { activities.remove(activities.findFirst(activity)); }

protected:
  Project(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  uint32_t GetLineNumberFromByteIndex(Slice<char> buffer, size_t index) const;

  Variant SaveActivities();

  void ParseProject(Variant node);
  void ParseActivities(Variant node);
  void ParseActivity(Variant node);

  Array<ActivityRef> activities;

  ResourceManagerRef spResourceManager;
  SharedString srcString;

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(SaveProject, "Save Project to an XML file"),
    };
  }
};

} //namespace kernel

#endif // EP_PROJECT_H
