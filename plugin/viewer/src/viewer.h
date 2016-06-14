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

  void Activate() override;
  void Deactivate() override;
  void Update(double timeStep);
  Variant Save() const override;

  // TODO: Remove Temp hacks!
  SimpleCameraRef GetSimpleCamera() const { return spCamera; }
  ViewRef GetView() const { return spView; }

  static MutableString<260> GetFileNameFromPath(String path);

protected:
  Viewer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  void CreatePlatformLogo();
  bool InputHook(ep::InputEvent ev);

  ~Viewer() { Deactivate(); }

  static void StaticInit(ep::Kernel *pKernel);
  // TODO: Bug EP-66
  //void ToggleResourcesPanel();
  void ToggleBookmarksPanel();
  void CreateBookmark();

  void OnResourceDropped(String resourceUID, int x, int y);
  void AddSceneNodeAtViewPosition(UDNodeRef node, int x, int y);

  // Static functions for CommandManager callbacks
  static void StaticToggleBookmarksPanel(Variant::VarMap params);
  static void StaticCreateBookmark(Variant::VarMap params);
  // TODO: Bug EP-66
  //static void StaticToggleResourcesPanel(Variant::VarMap params);

  UDModelRef spModel;
  SceneRef spScene;
  SimpleCameraRef spCamera;
  ViewRef spView;
  UIComponentRef spUIBookmarks;
  NodeRef spImageNode;

private:
  Array<const PropertyInfo> GetProperties() const;
};

} //namespace ep

#endif // EP_VIEWER_H
