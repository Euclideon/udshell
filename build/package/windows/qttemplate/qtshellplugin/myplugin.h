#pragma once
#ifndef EP_MYPLUGIN_H
#define EP_MYPLUGIN_H

#include "ep/cpp/component/activity.h"
#include "ep/cpp/input.h"

namespace ep {

#define PLUGIN_VER 100

  SHARED_CLASS(UIComponent);
  SHARED_CLASS(MyPlugin);
  SHARED_CLASS(UDModel);
  SHARED_CLASS(UDNode);
  SHARED_CLASS(SimpleCamera);
  SHARED_CLASS(Scene);
  SHARED_CLASS(View);
  SHARED_CLASS(Node);

  class MyPlugin : public Activity
  {
    EP_DECLARE_COMPONENT(ep, MyPlugin, Activity, EPKERNEL_PLUGINVERSION, "Describe your plugin", 0)
  public:

  protected:
    MyPlugin(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
    
  private:
    Array<const PropertyInfo> GetProperties() const;
  };

} //namespace ep

#endif // EP_MYPLUGIN_H
