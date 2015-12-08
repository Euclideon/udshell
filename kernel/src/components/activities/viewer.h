#pragma once
#ifndef EP_VIEWER_H
#define EP_VIEWER_H

#include "components/activities/activity.h"

namespace kernel {

PROTOTYPE_COMPONENT(Viewer);
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

protected:
  Viewer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~Viewer() { Deactivate(); }

  // TODO: Tidy this up once bookmarks ui is created.
  void BookmarkCurrentCamera();
  void JumpToBookmark();

  // TODO: Remove this when Unsubscribe can be called with some kind of subscription identifier
  Delegate<void(double)> updateFunc;

  UDModelRef spModel;
  SceneRef spScene;
  SimpleCameraRef spCamera;
  ViewRef spView;
};

} //namespace kernel

#endif // EP_VIEWER_H
