#pragma once
#ifndef EP_PROJECT_H
#define EP_PROJECT_H

#include "ep/cpp/internal/i/iproject.h"

#include "ep/cpp/component/resource/resource.h"

namespace ep
{

  SHARED_CLASS(Project);
  SHARED_CLASS(Activity);
  SHARED_CLASS(ResourceManager);

  class Project : public Resource
  {
    EP_DECLARE_COMPONENT_WITH_IMPL(ep, Project, IProject, Component, EPKERNEL_PLUGINVERSION, "Contains the state of a project", 0)
  public:

    void SaveProject() { pImpl->SaveProject(); }

    virtual Slice<ActivityRef> GetActivities() const { return pImpl->GetActivities(); }
    virtual void AddActivity(ActivityRef activity) { pImpl->AddActivity(activity); }
    virtual void RemoveActivity(ActivityRef activity) { pImpl->RemoveActivity(activity); }

    virtual void SetSrc(String src) { pImpl->SetSrc(src); }
    virtual String GetSrc() const { return pImpl->GetSrc(); }

    virtual ActivityRef GetActiveActivity() const { return pImpl->GetActiveActivity(); }
    virtual void SetActiveActivity(ActivityRef activity) { pImpl->SetActiveActivity(activity); }

  protected:
    Project(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
      : Resource(pType, pKernel, uid, initParams)
    {
      pImpl = CreateImpl(initParams);
    }

  private:
    Array<const PropertyInfo> GetProperties() const;
    Array<const MethodInfo> GetMethods() const;
  };

} //namespace ep

#endif // EP_PROJECT_H
