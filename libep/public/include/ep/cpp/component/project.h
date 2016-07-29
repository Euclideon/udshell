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

    void saveProject() { pImpl->SaveProject(); }

    virtual Slice<ActivityRef> getActivities() const { return pImpl->GetActivities(); }
    virtual void addActivity(ActivityRef activity) { pImpl->AddActivity(activity); }
    virtual void removeActivity(ActivityRef activity) { pImpl->RemoveActivity(activity); }

    virtual void setSrc(String src) { pImpl->SetSrc(src); }
    virtual String getSrc() const { return pImpl->GetSrc(); }

    virtual ActivityRef getActiveActivity() const { return pImpl->GetActiveActivity(); }
    virtual void setActiveActivity(ActivityRef activity) { pImpl->SetActiveActivity(activity); }

  protected:
    Project(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
      : Resource(pType, pKernel, uid, initParams)
    {
      pImpl = createImpl(initParams);
    }

  private:
    Array<const PropertyInfo> getProperties() const;
    Array<const MethodInfo> getMethods() const;
  };

} //namespace ep

#endif // EP_PROJECT_H
