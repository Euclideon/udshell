#pragma once
#ifndef %{GUARD}
#define %{GUARD}

#include "ep/cpp/component/activity.h"
#include "ep/cpp/component/scene.h"

namespace %{Namespace} {

using namespace ep;

SHARED_CLASS(%{ActivityName});

class %{ActivityName} : public Activity
{
  EP_DECLARE_COMPONENT(%{Namespace}, %{ActivityName}, Activity, EPKERNEL_PLUGINVERSION, "%{ActivityDescription}", 0);

public:
  void activate() override final;
  void deactivate() override final;
  void update(double timeStep);
  Variant save() const override final;

protected:
  %{ActivityName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~%{ActivityName}() { deactivate(); }

private:
  Array<const PropertyInfo> getProperties() const;    // Register Properties
  Array<const MethodInfo> getMethods() const;         // Register Methods
  Array<const EventInfo> getEvents() const;           // Register Events
  Array<const StaticFuncInfo> getStaticFuncs() const; // Register Static Methods

  SceneRef spScene;
};

} //namespace %{Namespace}

#endif // %{GUARD}\
