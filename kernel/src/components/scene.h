#pragma once
#ifndef EPSCENE_H
#define EPSCENE_H

#include "udRender.h"

#include "ep/cpp/component.h"
#include "nodes/node.h"

// TODO: Move this once the Scene Impl refactor is done.
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

class Scene : public Component
{
  friend class View;
  EP_DECLARE_COMPONENT(Scene, Component, EPKERNEL_PLUGINVERSION, "Scene desc...")
public:

  virtual bool InputEvent(const epInputEvent &ev);
  virtual void Update(double timeDelta);
  Variant Save() const override;

  // TODO: Consider creating an event for when bookmarks are successfully created
  void AddBookMarkFromCamera(String bmName, CameraRef camera);
  void AddBookMark(String bmName, const Bookmark &bm);
  void RemoveBookMark(String bmName);
  void RenameBookMark(String oldName, String newName);
  const Bookmark *FindBookMark(String bmName) const { return bookmarks.Get(bmName); }
  void LoadBookMarks(Variant::VarMap bookmarks);
  Variant SaveBookMarks() const;

  using BookmarkMap = SharedMap<AVLTree<SharedString, Bookmark>>;
  const BookmarkMap &GetBookmarkMap() const { return bookmarks; }

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

  BookmarkMap bookmarks;

  Scene(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~Scene();

  Variant FindBookMark_Internal(String bmName) const { auto *pBM = bookmarks.Get(bmName); return pBM ? Variant(*pBM) : Variant(nullptr); }

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
