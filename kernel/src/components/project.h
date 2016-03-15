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
  EP_DECLARE_COMPONENT(Project, Component, EPKERNEL_PLUGINVERSION, "Contains the state of a project", 0)
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

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(ActiveActivity, "The currently active activity", nullptr, 0),
      EP_MAKE_PROPERTY_RO(Activities, "The activities contained in the project", nullptr, 0),
      EP_MAKE_PROPERTY(Src, "The URL for the Project file", nullptr, 0)
    };
  }

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(SaveProject, "Save Project to an XML file"),
      EP_MAKE_METHOD(AddActivity, "Add an Activity to the Project"),
      EP_MAKE_METHOD(RemoveActivity, "Remove an Activity from the Project"),
    };
  }
};

} //namespace ep

#endif // EP_PROJECT_H
