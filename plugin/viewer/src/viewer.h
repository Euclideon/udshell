#pragma once
#ifndef EP_VIEWER_H
#define EP_VIEWER_H

#include "ep/cpp/component/activity.h"

namespace ep {

#define PLUGIN_VER 100

SHARED_CLASS(UIComponent);
SHARED_CLASS(Viewer);
SHARED_CLASS(UDModel);
SHARED_CLASS(SimpleCamera);
SHARED_CLASS(Scene);
SHARED_CLASS(View);

class Viewer : public Activity
{
  EP_DECLARE_COMPONENT(Viewer, Activity, EPKERNEL_PLUGINVERSION, "Viewer desc...")
public:

  void Activate() override;
  void Deactivate() override;
  void Update(double timeStep);
  Variant Save() const override;

  // TODO: Remove Temp hacks!
  SimpleCameraRef GetSimpleCamera() const { return spCamera; }
  ViewRef GetView() const { return spView; }

protected:
  Viewer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~Viewer() { Deactivate(); }

  static void StaticInit(ep::Kernel *pKernel);
  void ToggleBookmarksPanel();
  void CreateBookmark();

  // Static functions for CommandManager callbacks
  static void StaticToggleBookmarksPanel(Variant::VarMap params);
  static void StaticCreateBookmark(Variant::VarMap params);

  UDModelRef spModel;
  SceneRef spScene;
  SimpleCameraRef spCamera;
  ViewRef spView;
  UIComponentRef spUIBookmarks;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(SimpleCamera, "The Viewer's SimpleCamera Component", nullptr, 0),
      EP_MAKE_PROPERTY_RO(View, "The Viewer's View Component", nullptr, 0),
    };
  }
};

} //namespace ep

#endif // EP_VIEWER_H

