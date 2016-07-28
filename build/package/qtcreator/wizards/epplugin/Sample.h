#pragma once
#ifndef %{GUARD}
#define %{GUARD}

#include "ep/cpp/component/component.h"

namespace %{Namespace} {

using namespace ep;

SHARED_CLASS(%{ActivityName});

class %{ActivityName} : public Component
{
  EP_DECLARE_COMPONENT(%{Namespace}, %{ActivityName}, Component, EPKERNEL_PLUGINVERSION, "%{ActivityDescription}", 0);
public:

protected:
  %{ActivityName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~%{ActivityName}() { }

private:
  Array<const PropertyInfo> getProperties() const;    // Register Properties
  Array<const MethodInfo> getMethods() const;         // Register Methods
  Array<const EventInfo> getEvents() const;           // Register Events
  Array<const StaticFuncInfo> getStaticFuncs() const; // Register Static Methods
};

} //namespace %{Namespace}

#endif // %{GUARD}\
