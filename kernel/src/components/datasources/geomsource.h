#pragma once
#if !defined(_EP_GEOMSOURCE_H)
#define _EP_GEOMSOURCE_H

#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/stream.h"

// prototypes to pacify C++
struct aiScene;
struct aiNode;
template<typename T> class aiMatrix4x4t;
typedef aiMatrix4x4t<float> aiMatrix4x4;

namespace ep
{

SHARED_CLASS(GeomSource);
SHARED_CLASS(Node);

class GeomSource : public DataSource
{
  EP_DECLARE_COMPONENT(GeomSource, DataSource, EPKERNEL_PLUGINVERSION, "Provides polygon geometry", 0)
public:

  Slice<const String> GetFileExtensions() const override
  {
    return extensions;
  }

protected:
  static const Array<const String> extensions;

  static void StaticInit(ep::Kernel *pKernel);

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

} // namespace ep

#endif // _EP_GEOMSOURCE_H
