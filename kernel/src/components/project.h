#pragma once
#ifndef EP_PROJECT_H
#define EP_PROJECT_H

#include "ep/cpp/component/component.h"

namespace ep
{

SHARED_CLASS(Project);
SHARED_CLASS(Activity);
SHARED_CLASS(ResourceManager);

class Project : public Component
{
  EP_DECLARE_COMPONENT(ep, Project, Component, EPKERNEL_PLUGINVERSION, "Contains the state of a project", 0)
public:

  void SaveProject();

  Slice<ActivityRef> GetActivities() const { return activities; }
  void AddActivity(ActivityRef activity) { activities.pushBack(activity); }
  void RemoveActivity(ActivityRef activity) { activities.remove(activities.findFirst(activity)); }

  void SetSrc(String src) { srcString = src; }
  String GetSrc() const { return srcString; }

  ActivityRef GetActiveActivity() const { return spActiveActivity; }
  void SetActiveActivity(ActivityRef activity) { spActiveActivity = activity; }

protected:
  Project(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  Variant SaveActivities();

  void ParseProject(Variant node);
  void ParseActivities(Variant node);
  void ParseActivity(Variant node);

  Array<ActivityRef> activities;

  ResourceManagerRef spResourceManager;
  SharedString srcString = nullptr;
  ActivityRef spActiveActivity = nullptr;

  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} //namespace ep

#endif // EP_PROJECT_H
