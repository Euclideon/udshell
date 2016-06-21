#pragma once
#ifndef %{GUARD}
#define %{GUARD}

#include "ep/cpp/component/component.h"

namespace %{Namespace} {

using namespace ep;

class %{ProjectName} : public Component
{
  EP_DECLARE_COMPONENT(%{Namespace}, %{ProjectName}, Component, EPKERNEL_PLUGINVERSION, "%{PluginDescription}", 0);
public:

protected:
  %{ProjectName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~%{ProjectName}() { }

private:
  Array<const PropertyInfo> GetProperties() const;    // Register Properties
  Array<const MethodInfo> GetMethods() const;         // Register Methods
  Array<const EventInfo> GetEvents() const;           // Register Events
  Array<const StaticFuncInfo> GetStaticFuncs() const; // Register Static Methods
};

} //namespace %{Namespace}

#endif // %{GUARD}\
