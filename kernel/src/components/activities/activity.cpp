#include "components/activities/activity.h"

namespace kernel
{

Activity::Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

} // namespace kernel
