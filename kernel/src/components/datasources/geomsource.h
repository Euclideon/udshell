#pragma once
#if !defined(_EP_GEOMSOURCE_H)
#define _EP_GEOMSOURCE_H

#include "components/datasource.h"
#include "components/resources/array.h"
#include "components/stream.h"

// prototypes to pacify C++
struct aiScene;
struct aiNode;
template<typename T> class aiMatrix4x4t;
typedef aiMatrix4x4t<float> aiMatrix4x4;

namespace kernel
{

SHARED_CLASS(GeomSource);
SHARED_CLASS(Node);

class GeomSource : public DataSource
{
  EP_DECLARE_COMPONENT(GeomSource, DataSource, EPKERNEL_PLUGINVERSION, "Provides polygon geometry")
public:

  Slice<const String> GetFileExtensions() const override
  {
    return extensions;
  }

protected:
  static const Array<const String> extensions;

  static epResult StaticInit(ep::Kernel *pKernel);

  GeomSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : DataSource(pType, pKernel, uid, initParams)
  {
    StreamRef ref = OpenStream(*initParams.Get("src"));
    Create(ref);
  }

  void Create(StreamRef spSource);

  void ParseMaterials(const aiScene *pScene);
  void ParseMeshes(const aiScene *pScene);

  NodeRef ParseNode(const aiScene *pScene, aiNode *pNode, const aiMatrix4x4 *pParent, size_t &numMeshes, int depth = 0);
};

} // namespace kernel

#endif // _EP_GEOMSOURCE_H
