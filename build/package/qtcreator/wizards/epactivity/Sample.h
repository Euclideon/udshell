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
  void Activate() override final;
  void Deactivate() override final;
  void Update(double timeStep);
  Variant Save() const override final;

protected:
  %{ActivityName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~%{ActivityName}() { Deactivate(); }
  
private:
  Array<const PropertyInfo> GetProperties() const;    // Register Properties
  Array<const MethodInfo> GetMethods() const;         // Register Methods
  Array<const EventInfo> GetEvents() const;           // Register Events
  Array<const StaticFuncInfo> GetStaticFuncs() const; // Register Static Methods
  
  SceneRef spScene;
};

} //namespace %{Namespace}

#endif // %{GUARD}\
