#pragma once
#ifndef EPSCENE_H
#define EPSCENE_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/iscene.h"
#include "ep/c/input.h"

namespace ep {

struct Bookmark
{
  Double3 position;
  Double3 ypr;
};

inline Variant epToVariant(const Bookmark &bm)
{
  Variant::VarMap map;
  map.Insert("position", bm.position);
  map.Insert("orientation", bm.ypr);
  return std::move(map);
}

inline void epFromVariant(const Variant &v, Bookmark *pBM)
{
  pBM->position = v["position"].as<Double3>();
  pBM->ypr = v["orientation"].as<Double3>();
}

SHARED_CLASS(RenderScene);
SHARED_CLASS(View);
SHARED_CLASS(Scene);
SHARED_CLASS(Camera);

class Scene : public Component, public IScene
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Scene, IScene, Component, EPKERNEL_PLUGINVERSION, "Scene desc...")

public:
  bool InputEvent(const epInputEvent &ev) override { return pImpl->InputEvent(ev); }
  void Update(double timeDelta) override { pImpl->Update(timeDelta); }
  Variant Save() const override { return pImpl->Save(); }

  // TODO: Consider creating an event for when bookmarks are successfully created
  void AddBookMarkFromCamera(String bmName, CameraRef camera) override { pImpl->AddBookMarkFromCamera(bmName, camera); }
  void AddBookMark(String bmName, const Bookmark &bm) override { pImpl->AddBookMark(bmName, bm); }
  void RemoveBookMark(String bmName) override { pImpl->RemoveBookMark(bmName); }
  void RenameBookMark(String oldName, String newName) override { pImpl->RenameBookMark(oldName, newName); }
  const Bookmark *FindBookMark(String bmName) const override { return pImpl->FindBookMark(bmName); }
  void LoadBookMarks(Variant::VarMap bookmarks) override { pImpl->LoadBookMarks(bookmarks); }
  Variant SaveBookMarks() const override { return pImpl->SaveBookMarks(); }

  const BookmarkMap &GetBookmarkMap() const override { return pImpl->GetBookmarkMap(); }

  NodeRef GetRootNode() const override { return pImpl->GetRootNode(); }

  RenderSceneRef GetRenderScene() override { return pImpl->GetRenderScene(); }

  // TODO: HACK: fix this api!
  epResult SetRenderModels(struct udRenderModel models[], size_t numModels) override { return pImpl->SetRenderModels(models, numModels); }
  const udRenderModel* GetRenderModels(size_t *pNumModels) const override { return pImpl->GetRenderModels(pNumModels); }

  void MakeDirty() override { pImpl->MakeDirty(); }

  Event<> Dirty;

protected:
  Scene(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  Variant FindBookMark_Internal(String bmName) const override { return pImpl->FindBookMark_Internal(bmName); }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(RootNode, "Scene root node", nullptr, 0),
      EP_MAKE_PROPERTY_RO(BookmarkMap, "Bookmark map", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(MakeDirty, "Force a dirty signal"),
      EP_MAKE_METHOD(AddBookMarkFromCamera, "Add a BookMark from Camera"),
      EP_MAKE_METHOD(AddBookMark, "Add a BookMark"),
      EP_MAKE_METHOD(RemoveBookMark, "Remove a BookMark"),
      EP_MAKE_METHOD(RenameBookMark, "Rename a BookMark"),
      EP_MAKE_METHOD_EXPLICIT("FindBookMark", FindBookMark_Internal, "Find a BookMark"),
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
