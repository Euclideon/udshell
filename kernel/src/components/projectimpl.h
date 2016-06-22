#pragma once
#ifndef EPPROJECTIMPL_H
#define EPPROJECTIMPL_H

#include "ep/cpp/component/project.h"
#include "ep/cpp/internal/i/iproject.h"

#include "ep/cpp/component/resource/resource.h"

namespace ep
{

  SHARED_CLASS(Project);
  SHARED_CLASS(Activity);
  SHARED_CLASS(ResourceManager);

  class ProjectImpl : public BaseImpl<Project, IProject>
  {
  public:
    ProjectImpl(Component *pInstance, Variant::VarMap initParams);

    void SaveProject() override final;

    Slice<ActivityRef> GetActivities() const override final { return activities; }
    void AddActivity(ActivityRef activity) override final { activities.pushBack(activity); }
    void RemoveActivity(ActivityRef activity) override final { activities.remove(activities.findFirst(activity)); }

    void SetSrc(String src) override final { srcString = src; }
    String GetSrc() const override final { return srcString; }

    ActivityRef GetActiveActivity() const override final { return spActiveActivity; }
    void SetActiveActivity(ActivityRef activity) override final { spActiveActivity = activity; }

  protected:
    Variant SaveActivities();

    void SetVars(String path);

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

#endif // EPPROJECTIMPL_H
