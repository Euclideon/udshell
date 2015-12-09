#include "components/activities/activity.h"

namespace ep
{

Activity::Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

} // namespace ep
