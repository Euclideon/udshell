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

SHARED_CLASS(RenderableScene);
SHARED_CLASS(View);
SHARED_CLASS(Scene);
SHARED_CLASS(Camera);

class Scene : public Component, public IScene
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Scene, IScene, Component, EPKERNEL_PLUGINVERSION, "Scene desc...", 0)

public:
  bool InputEvent(const epInputEvent &ev) override { return pImpl->InputEvent(ev); }
  void Update(double timeDelta) override { pImpl->Update(timeDelta); }
  Variant Save() const override { return pImpl->Save(); }

  // TODO: Consider creating an event for when bookmarks are successfully created
  void AddBookmarkFromCamera(String bmName, CameraRef camera) override final { pImpl->AddBookmarkFromCamera(bmName, camera); }
  void AddBookmark(String bmName, const Bookmark &bm) override final { pImpl->AddBookmark(bmName, bm); }
  void RemoveBookmark(String bmName) override final { pImpl->RemoveBookmark(bmName); }
  void RenameBookmark(String oldName, String newName) override final { pImpl->RenameBookmark(oldName, newName); }
  const Bookmark *FindBookmark(String bmName) const override final { return pImpl->FindBookmark(bmName); }
  void LoadBookmarks(Variant::VarMap bookmarks) override final { pImpl->LoadBookmarks(bookmarks); }
  Variant SaveBookmarks() const override final { return pImpl->SaveBookmarks(); }

  const BookmarkMap &GetBookmarkMap() const override { return pImpl->GetBookmarkMap(); }

  NodeRef GetRootNode() const override { return pImpl->GetRootNode(); }

  RenderableSceneRef GetRenderScene() override { return pImpl->GetRenderScene(); }

  void MakeDirty() override { pImpl->MakeDirty(); }

  Event<> Dirty;

protected:
  Scene(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  Variant FindBookmark_Internal(String bmName) const override { return pImpl->FindBookmark_Internal(bmName); }

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
      EP_MAKE_METHOD(AddBookmarkFromCamera, "Add a Bookmark from Camera"),
      EP_MAKE_METHOD(AddBookmark, "Add a Bookmark"),
      EP_MAKE_METHOD(RemoveBookmark, "Remove a Bookmark"),
      EP_MAKE_METHOD(RenameBookmark, "Rename a Bookmark"),
      EP_MAKE_METHOD_EXPLICIT("FindBookmark", FindBookmark_Internal, "Find a Bookmark"),
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
