#include "%{ActivityHdr}"

using namespace ep;

namespace %{Namespace} {

Array<const PropertyInfo> %{ActivityName}::getProperties() const
{
  return { };
}

Array<const ep::MethodInfo> %{ActivityName}::getMethods() const
{
  return { };
}


Array<const EventInfo> %{ActivityName}::getEvents() const
{
  return { };
}

Array<const StaticFuncInfo> %{ActivityName}::getStaticFuncs() const
{
  return { };
}

%{ActivityName}::%{ActivityName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

extern "C" bool epPluginAttach()
{
  Kernel::getInstance()->registerComponentType<%{ActivityName}>();
  return true;
}

} // namespace %{Namespace}
