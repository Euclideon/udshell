#pragma once
#ifndef EP_VIEWER_H
#define EP_VIEWER_H

#include "ep/cpp/component/activity.h"
#include "ep/cpp/input.h"

namespace ep {

#define PLUGIN_VER 100

SHARED_CLASS(UIComponent);
SHARED_CLASS(Viewer);
SHARED_CLASS(UDModel);
SHARED_CLASS(UDNode);
SHARED_CLASS(SimpleCamera);
SHARED_CLASS(Scene);
SHARED_CLASS(View);
SHARED_CLASS(Node);

class Viewer : public Activity
{
  EP_DECLARE_COMPONENT(ep, Viewer, Activity, EPKERNEL_PLUGINVERSION, "View UD data files", 0);
public:

  void activate() override;
  void deactivate() override;
  void update(double timeStep);
  Variant save() const override;

  // TODO: Remove Temp hacks!
  SimpleCameraRef getSimpleCamera() const { return spCamera; }
  ViewRef getView() const { return spView; }

  static MutableString<260> getFileNameFromPath(String path);

protected:
  Viewer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  void createPlatformLogo();
  bool inputHook(ep::InputEvent ev);

  ~Viewer() { deactivate(); }

  static void staticInit(ep::Kernel *pKernel);
  // TODO: Bug EP-66
  //void ToggleResourcesPanel();
  void toggleBookmarksPanel();
  void createBookmark();

  void onResourceDropped(String resourceUID, int x, int y);
  void addSceneNodeAtViewPosition(UDNodeRef node, int x, int y);

  // Static functions for CommandManager callbacks
  static void staticToggleBookmarksPanel(Variant::VarMap params);
  static void staticCreateBookmark(Variant::VarMap params);
  // TODO: Bug EP-66
  //static void StaticToggleResourcesPanel(Variant::VarMap params);

  UDModelRef spModel;
  SceneRef spScene;
  SimpleCameraRef spCamera;
  ViewRef spView;
  UIComponentRef spUIBookmarks;
  NodeRef spImageNode;

private:
  Array<const PropertyInfo> getProperties() const;
};

} //namespace ep

#endif // EP_VIEWER_H
