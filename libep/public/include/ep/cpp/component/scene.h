#pragma once
#ifndef EPSCENE_H
#define EPSCENE_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/iscene.h"
#include "ep/cpp/input.h"

namespace ep {

struct Bookmark
{
  Double3 position;
  Double3 ypr;
};

inline Variant epToVariant(const Bookmark &bm)
{
  Variant::VarMap map;
  map.insert("position", bm.position);
  map.insert("orientation", bm.ypr);
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
  virtual bool inputEvent(const ep::InputEvent &ev) { return pImpl->InputEvent(ev); }
  virtual void update(double timeDelta) { pImpl->Update(timeDelta); }

  Variant save() const override { return pImpl->Save(); }

  // TODO: Consider creating an event for when bookmarks are successfully created
  void addBookmarkFromCamera(String bmName, CameraRef camera) { pImpl->AddBookmarkFromCamera(bmName, camera); }
  void addBookmark(String bmName, const Bookmark &bm) { pImpl->AddBookmark(bmName, bm); }
  void removeBookmark(String bmName) { pImpl->RemoveBookmark(bmName); }
  void renameBookmark(String oldName, String newName) { pImpl->RenameBookmark(oldName, newName); }
  const Bookmark *findBookmark(String bmName) const { return pImpl->FindBookmark(bmName); }
  void loadBookmarks(Variant::VarMap bookmarks) { pImpl->LoadBookmarks(bookmarks); }
  Variant saveBookmarks() const { return pImpl->SaveBookmarks(); }

  const BookmarkMap &getBookmarkMap() const { return pImpl->GetBookmarkMap(); }

  //! Gets the root node of the scene graph.
  //! \return The scenes root node.
  NodeRef getRootNode() const { return pImpl->GetRootNode(); }

  RenderableSceneRef getRenderScene() { return pImpl->GetRenderScene(); }

  void makeDirty() { pImpl->MakeDirty(); }

  Event<> dirty;

protected:
  Scene(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  Variant findBookmarkInternal(String bmName) const { return pImpl->FindBookmark_Internal(bmName); }

private:
  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
  Array<const EventInfo> getEvents() const;
};

};

#endif // EPSCENE_H
