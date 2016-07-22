#include "%{ActivityHdr}"

using namespace ep;

namespace %{Namespace} {

Array<const PropertyInfo> %{ActivityName}::GetProperties() const
{
  return { };
}

Array<const ep::MethodInfo> %{ActivityName}::GetMethods() const
{
  return { };
}


Array<const EventInfo> %{ActivityName}::GetEvents() const
{
  return { };
}

Array<const StaticFuncInfo> %{ActivityName}::GetStaticFuncs() const
{
  return { };
}

%{ActivityName}::%{ActivityName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

extern "C" bool epPluginAttach()
{
  Kernel::GetInstance()->RegisterComponentType<%{ActivityName}>();
  return true;
}

} // namespace %{Namespace}
