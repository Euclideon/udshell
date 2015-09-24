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

namespace ep
{

SHARED_CLASS(GeomSource);
SHARED_CLASS(Node);

class GeomSource : public DataSource
{
public:
  EP_COMPONENT(GeomSource);

  epSlice<const epString> GetFileExtensions() const override
  {
    return extensions;
  }

protected:
  static const epArray<const epString> extensions;

  static udResult RegisterExtensions(Kernel *pKernel);

  GeomSource(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : DataSource(pType, pKernel, uid, initParams)
  {
    StreamRef ref = OpenStream(initParams["src"]);
    Create(ref);
  }

  void Create(StreamRef spSource);

  void ParseMaterials(const aiScene *pScene);
  void ParseMeshes(const aiScene *pScene);

  NodeRef ParseNode(const aiScene *pScene, aiNode *pNode, const aiMatrix4x4 *pParent, size_t &numMeshes, int depth = 0);
};

} // namespace ep

#endif // _EP_GEOMSOURCE_H
