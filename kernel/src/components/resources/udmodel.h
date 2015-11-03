#pragma once
#ifndef _EP_EPMODEL_H
#define _EP_EPMODEL_H

#include "udPlatform.h"
#include "udOctree.h"

#include "components/resources/resource.h"
#include "ep/cpp/sharedptr.h"
#include "udRender.h"
#include "kernel.h"

namespace ep
{

SHARED_CLASS(UDModel);

class UDModel : public Resource
{
public:
  EP_COMPONENT(UDModel);

  udOctree *GetOctreePtr() const { return pOctree; }
  friend class UDDataSource;
protected:
  UDModel(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~UDModel();

  udOctree *pOctree = nullptr;
};


struct NodeRenderModel : public udRenderModel
{
  typedef uint32_t(SimpleVoxelDlgt)(uint32_t color);

  udDouble4x4 matrix;
  udRenderClipArea clipArea;
  Delegate<SimpleVoxelDlgt> simpleVoxelDel;

  static unsigned VoxelShaderFunc(udRenderModel *pRenderModel, udNodeIndex nodeIndex, udRenderNodeInfo *epUnusedParam(pNodeInfo))
  {
    NodeRenderModel *pNodeModel = static_cast<NodeRenderModel*>(pRenderModel);
    udOctree *pOctree = pRenderModel->pOctree;
    uint32_t color = pOctree->pGetNodeColor(pOctree, nodeIndex);
    // TODO : either wrap this in a critical section or create Lua states for each thread
    color = pNodeModel->simpleVoxelDel(color);
    return color;
  }
};

} // namespace ep

#endif // _EP_EPMODEL_H
