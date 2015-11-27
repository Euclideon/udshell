#include "components/activities/activity.h"

namespace kernel
{

Activity::Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

} // namespace kernel
