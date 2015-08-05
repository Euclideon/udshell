#pragma once
#ifndef _UD_UDMODEL_H
#define _UD_UDMODEL_H

#include "udPlatform.h"
#include "udOctree.h"

#include "components/resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"
#include "udRender.h"
#include "udlua.h"
#include "kernel.h"

namespace ud
{

SHARED_CLASS(UDModel);

class UDModel : public Resource
{
public:
  UD_COMPONENT(UDModel);

  udResult Load(udString name, bool useStreamer);

  udOctree *GetOctreePtr() const { return pOctree; }

protected:
  UDModel(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~UDModel();

  udOctree *pOctree = nullptr;
};


struct NodeRenderModel : public udRenderModel
{
  typedef uint32_t(SimpleVoxelDlgt)(uint32_t color);

  udDouble4x4 matrix;
  udRenderClipArea clipArea;
  udDelegate<SimpleVoxelDlgt> simpleVoxelDel;

  static unsigned VoxelShaderFunc(udRenderModel *pRenderModel, udNodeIndex nodeIndex)
  {
    NodeRenderModel *pNodeModel = static_cast<NodeRenderModel*>(pRenderModel);
    udOctree *pOctree = pRenderModel->pOctree;
    uint32_t color = pOctree->pGetNodeColor(pOctree, nodeIndex);
    // TODO : either wrap this in a critical section  or create Lua states for each thread
    color = pNodeModel->simpleVoxelDel(color);
    return color;
  }
};

} // namespace ud

#endif // _UD_UDMODEL_H
