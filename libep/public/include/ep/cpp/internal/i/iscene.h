#pragma once
#if !defined(_EP_ISCENE_HPP)
#define _EP_ISCENE_HPP

#include "ep/cpp/component/component.h"
#include "ep/c/input.h"

namespace ep {

struct Bookmark;
SHARED_CLASS(Node);
SHARED_CLASS(RenderableScene);
SHARED_CLASS(Camera);

class IScene
{
public:
  virtual bool InputEvent(const epInputEvent &ev) = 0;
  virtual void Update(double timeDelta) = 0;
  virtual Variant Save() const = 0;

  // TODO: Consider creating an event for when bookmarks are successfully created
  virtual void AddBookmarkFromCamera(String bmName, CameraRef camera) = 0;
  virtual void AddBookmark(String bmName, const Bookmark &bm) = 0;
  virtual void RemoveBookmark(String bmName) = 0;
  virtual void RenameBookmark(String oldName, String newName) = 0;
  virtual const Bookmark *FindBookmark(String bmName) const = 0;
  virtual void LoadBookmarks(Variant::VarMap bookmarks) = 0;
  virtual Variant SaveBookmarks() const = 0;

  using BookmarkMap = SharedMap<AVLTree<SharedString, Bookmark>>;
  virtual const BookmarkMap &GetBookmarkMap() const = 0;

  virtual NodeRef GetRootNode() const = 0;

  virtual RenderableSceneRef GetRenderScene() = 0;

  virtual void MakeDirty() = 0;

protected:
  virtual Variant FindBookmark_Internal(String bmName) const = 0;
};

};

#endif // EPSCENE_H
