#include "components/activities/activity.h"

namespace ep
{

static CPropertyDesc props[] =
{
  {
    {
      "ui", // id
      "UI", // displayName
      "The top level UI compoment for this activity", // description
    },
    &Activity::GetUI,  // getter
    nullptr,           // setter
  },
};

static CMethodDesc methods[] =
{
  {
    {
      "activate", // id
      "Set as the active activity", // description
    },
    &Activity::Activate, // method
  },
  {
    {
      "deactivate", // id
      "Unset as the active activity", // description
    },
    &Activity::Deactivate, // method
  },
};

ComponentDesc Activity::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "activity", // id
  "Activity", // displayName
  "Activity", // description

  Slice<CPropertyDesc>(props, EPARRAYSIZE(props)),   // properties
  Slice<CMethodDesc>(methods, EPARRAYSIZE(methods)), // methods
  nullptr,    // events
};

Activity::Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

} // namespace ep
