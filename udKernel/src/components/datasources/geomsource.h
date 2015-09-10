#pragma once
#if !defined(_UD_GEOMSOURCE_H)
#define _UD_GEOMSOURCE_H

#include "components/datasource.h"
#include "components/resources/array.h"
#include "components/stream.h"

// prototypes to pacify C++
struct aiScene;
struct aiNode;
template<typename T> class aiMatrix4x4t;
typedef aiMatrix4x4t<float> aiMatrix4x4;

namespace ud
{

SHARED_CLASS(GeomSource);
SHARED_CLASS(Node);

class GeomSource : public DataSource
{
public:
  UD_COMPONENT(GeomSource);

  udSlice<const udString> GetFileExtensions() const override
  {
    return extensions;
  }

protected:
  static const udFixedSlice<const udString> extensions;

  static udResult RegisterExtensions(Kernel *pKernel);

  GeomSource(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : DataSource(pType, pKernel, uid, initParams)
  {
    StreamRef ref = OpenStream(initParams["src"]);
    Create(ref);
  }

  void Create(StreamRef spSource);

  NodeRef ParseNode(const aiScene *pScene, aiNode *pNode, const aiMatrix4x4 *pParent, size_t &numMeshes, int depth = 0);
};

} // namespace ud

#endif // _UD_GEOMSOURCE_H
