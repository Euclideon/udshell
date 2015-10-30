#pragma once
#ifndef EP_VIEWER_H
#define EP_VIEWER_H

#include "components/activities/activity.h"

namespace ep
{
PROTOTYPE_COMPONENT(Viewer);
SHARED_CLASS(UDNode);
SHARED_CLASS(View);
SHARED_CLASS(SimpleCamera);
SHARED_CLASS(Scene);

class Viewer : public Activity
{
public:
  EP_COMPONENT(Viewer);

  void Activate() override;
  void Deactivate() override;
  void Update(double timeStep);

protected:
  Viewer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  ~Viewer() { Deactivate(); }

  // TODO: Remove this when Unsubscribe can be called with some kind of subscription identifier
  Delegate<void(double)> updateFunc;

  SceneRef spScene;
  SimpleCameraRef spCamera;
};

} //namespace ep
#endif // EP_VIEWER_H
