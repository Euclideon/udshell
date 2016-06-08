#pragma once
#if !defined(_EP_IPROJECT_HPP)
#define _EP_IPROJECT_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/input.h"

namespace ep
{

SHARED_CLASS(Project);
SHARED_CLASS(Activity);
SHARED_CLASS(ResourceManager);

class IProject
{
public:
  virtual void SaveProject() = 0;

  virtual Slice<ActivityRef> GetActivities() const = 0;
  virtual void AddActivity(ActivityRef activity) = 0;
  virtual void RemoveActivity(ActivityRef activity) = 0;

  virtual void SetSrc(String src) = 0;
  virtual String GetSrc() const = 0;

  virtual ActivityRef GetActiveActivity() const = 0;
  virtual void SetActiveActivity(ActivityRef activity) = 0;
};

} //namespace ep

#endif // _EP_IPROJECT_HPP
