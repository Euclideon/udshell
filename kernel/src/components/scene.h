#pragma once
#ifndef EPSCENE_H
#define EPSCENE_H

#include "udRender.h"

#include "component.h"
#include "nodes/node.h"

namespace kernel
{

SHARED_CLASS(RenderScene);

PROTOTYPE_COMPONENT(View);
PROTOTYPE_COMPONENT(Scene);

class Scene : public Component
{
  friend class View;
  EP_DECLARE_COMPONENT(Scene, Component, EPKERNEL_PLUGINVERSION, "Scene desc...")
public:

  virtual bool InputEvent(const epInputEvent &ev);
  virtual void Update(double timeDelta);

  NodeRef GetRootNode() const { return rootNode; }

  RenderSceneRef GetRenderScene();

  Event<> Dirty;

  // TODO: HACK: fix this api!
  epResult SetRenderModels(struct udRenderModel models[], size_t numModels);
  const udRenderModel* GetRenderModels(size_t *pNumModels) const { if (pNumModels) { *pNumModels = numRenderModels; } return renderModels; }

  void MakeDirty()
  {
    bDirty = true;
    Dirty.Signal();
  }

protected:
  double timeStep = 0.0;

  NodeRef rootNode = nullptr;

  RenderSceneRef spCache = nullptr;
  bool bDirty = true; // becomes dirty when scene changes

  udRenderModel renderModels[16];
  size_t numRenderModels = 0;

  Scene(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  virtual ~Scene();

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(RootNode, "Scene root node", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(MakeDirty, "Force a dirty signal"),
    };
  }
  static Array<const EventInfo> GetEvents()
  {
    return{
      EP_MAKE_EVENT(Dirty, "Scene dirty event"),
    };
  }
};

};

#endif // EPSCENE_H
