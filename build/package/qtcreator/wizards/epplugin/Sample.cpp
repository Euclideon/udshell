#include "%{ProjectName}.h"

using namespace ep;

namespace %{Namespace} {

Array<const PropertyInfo> %{ProjectName}::GetProperties() const
{
  return { };
}

Array<const ep::MethodInfo> %{ProjectName}::GetMethods() const
{
  return { };
}


Array<const EventInfo> %{ProjectName}::GetEvents() const
{
  return { };
}

Array<const StaticFuncInfo> %{ProjectName}::GetStaticFuncs() const
{
  return { };
}

%{ProjectName}::%{ProjectName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

extern "C" bool epPluginAttach()
{
  Kernel::GetInstance()->RegisterComponentType<%{ProjectName}>();
  return true;
}

} // namespace %{Namespace}
