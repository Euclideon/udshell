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

class Scene : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Scene, IScene, Resource, EPKERNEL_PLUGINVERSION, "Scene desc...", 0)

public:
  virtual bool InputEvent(const epInputEvent &ev) { return pImpl->InputEvent(ev); }
  virtual void Update(double timeDelta) { pImpl->Update(timeDelta); }

  Variant Save() const override { return pImpl->Save(); }

  // TODO: Consider creating an event for when bookmarks are successfully created
  void AddBookmarkFromCamera(String bmName, CameraRef camera) { pImpl->AddBookmarkFromCamera(bmName, camera); }
  void AddBookmark(String bmName, const Bookmark &bm) { pImpl->AddBookmark(bmName, bm); }
  void RemoveBookmark(String bmName) { pImpl->RemoveBookmark(bmName); }
  void RenameBookmark(String oldName, String newName) { pImpl->RenameBookmark(oldName, newName); }
  const Bookmark *FindBookmark(String bmName) const { return pImpl->FindBookmark(bmName); }
  void LoadBookmarks(Variant::VarMap bookmarks) { pImpl->LoadBookmarks(bookmarks); }
  Variant SaveBookmarks() const { return pImpl->SaveBookmarks(); }

  const BookmarkMap &GetBookmarkMap() const { return pImpl->GetBookmarkMap(); }

  NodeRef GetRootNode() const { return pImpl->GetRootNode(); }

  RenderableSceneRef GetRenderScene() { return pImpl->GetRenderScene(); }

  void MakeDirty() { pImpl->MakeDirty(); }

  Event<> Dirty;

protected:
  Scene(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  Variant FindBookmark_Internal(String bmName) const { return pImpl->FindBookmark_Internal(bmName); }

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
  Array<const EventInfo> GetEvents() const;
};

};

#endif // EPSCENE_H
