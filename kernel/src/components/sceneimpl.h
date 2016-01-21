#pragma once
#ifndef EPSCENEIMPL_H
#define EPSCENEIMPL_H

#include "udRender.h"

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/node.h"
#include "ep/cpp/component/scene.h"
#include "ep/cpp/internal/i/iscene.h"

// TODO: Move this once the Scene Impl refactor is done.
namespace ep {

SHARED_CLASS(RenderableScene);
SHARED_CLASS(View);
SHARED_CLASS(Camera);

class SceneImpl : public BaseImpl<Scene, IScene>
{
public:
  SceneImpl(Component *pInstance, Variant::VarMap initParams);

  virtual bool InputEvent(const epInputEvent &ev) override final;
  virtual void Update(double timeDelta) override final;
  Variant Save() const override final;

  // TODO: Consider creating an event for when bookmarks are successfully created
  void AddBookmarkFromCamera(String bmName, CameraRef camera) override final;
  void AddBookmark(String bmName, const Bookmark &bm) override final;
  void RemoveBookmark(String bmName) override final;
  void RenameBookmark(String oldName, String newName) override final;
  const Bookmark *FindBookmark(String bmName) const  override final { return bookmarks.Get(bmName); }
  void LoadBookmarks(Variant::VarMap bookmarks) override final;
  Variant SaveBookmarks() const override final;
  const BookmarkMap &GetBookmarkMap() const override final { return bookmarks; }

  NodeRef GetRootNode() const override final { return rootNode; }

  RenderableSceneRef GetRenderScene() override final;

  static RenderableSceneRef Convert(RenderScene &renderScene);

  void MakeDirty() override final
  {
    bDirty = true;
    pInstance->Dirty.Signal();
  }

protected:
  Variant FindBookmark_Internal(String bmName) const override final { auto *pBM = bookmarks.Get(bmName); return pBM ? Variant(*pBM) : Variant(nullptr); }

  double timeStep = 0.0;
  NodeRef rootNode = nullptr;
  RenderSceneRef spCache = nullptr;
  bool bDirty = true; // becomes dirty when scene changes
  udRenderModel renderModels[16];
  size_t numRenderModels = 0;
  BookmarkMap bookmarks;
};

}

#endif // EPSCENEIMPL_H
