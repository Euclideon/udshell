#pragma once
#if !defined(_EP_ISCENE_HPP)
#define _EP_ISCENE_HPP

#include "ep/cpp/component/component.h"
#include "ep/c/input.h"

namespace ep {

struct Bookmark;
SHARED_CLASS(Node);
SHARED_CLASS(RenderScene);
SHARED_CLASS(Camera);

class IScene
{
public:
  virtual bool InputEvent(const epInputEvent &ev) = 0;
  virtual void Update(double timeDelta) = 0;
  virtual Variant Save() const = 0;

  // TODO: Consider creating an event for when bookmarks are successfully created
  virtual void AddBookMarkFromCamera(String bmName, CameraRef camera) = 0;
  virtual void AddBookMark(String bmName, const Bookmark &bm) = 0;
  virtual void RemoveBookMark(String bmName) = 0;
  virtual void RenameBookMark(String oldName, String newName) = 0;
  virtual const Bookmark *FindBookMark(String bmName) const = 0;
  virtual void LoadBookMarks(Variant::VarMap bookmarks) = 0;
  virtual Variant SaveBookMarks() const = 0;

  using BookmarkMap = SharedMap<AVLTree<SharedString, Bookmark>>;
  virtual const BookmarkMap &GetBookmarkMap() const = 0;

  virtual NodeRef GetRootNode() const = 0;

  virtual RenderSceneRef GetRenderScene() = 0;

  // TODO: HACK: fix this api!
  virtual epResult SetRenderModels(struct udRenderModel models[], size_t numModels) = 0;
  virtual const udRenderModel* GetRenderModels(size_t *pNumModels) const = 0;

  virtual void MakeDirty() = 0;

protected:
  virtual Variant FindBookMark_Internal(String bmName) const = 0;
};

};

#endif // EPSCENE_H
