#include "geomsource.h"
#include "components/resources/array.h"
#include "hal/image.h"
#include "kernel.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace ud
{

ComponentDesc GeomSource::descriptor =
{
  &DataSource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "geomsource", // id
  "Geometry Source", // displayName
  "Provides polygon geometry", // description
};

void GeomSource::Create(StreamRef spSource)
{
  const char *pExtension = nullptr; // TODO: get the file extension to help the assimp!

  // allocate for file
  int64_t len = spSource->Length();
  void *pBuffer = udAlloc(len);

  // read file from source
  size_t read = spSource->Read(pBuffer, len);
  UDASSERT((int64_t)read == len, "!");

  Assimp::Importer importer;

  unsigned int options = aiProcess_ConvertToLeftHanded      |
                         aiProcess_FlipWindingOrder         |
//                       aiProcess_CalcTangentSpace         |
                         aiProcess_Triangulate              |
                         aiProcess_GenUVCoords              |
                         aiProcess_JoinIdenticalVertices    |
                         aiProcess_FindDegenerates          |
                         aiProcess_SortByPType              |
                         aiProcess_RemoveRedundantMaterials |
//                       aiProcess_ImproveCacheLocality     | // this needs to be configured for each target...
                         0;

  // if we want lines and points, but don't want degenerate triangles (some formats use degenerate triangles to emulate lines/points):
//  importer.SetPropertyInteger(AI_CONFIG_PP_FD_REMOVE, 1); // remove degenerate triangles
  // of we don't want lines and points at all:
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE); // remove all lines and points

  // set the post-transform cache size...
//  importer.SetPropertyInteger(AI_CONFIG_PP_ICL_PTCACHE_SIZE, postTransformCacheSize?);

  const aiScene *pScene = importer.ReadFileFromMemory(pBuffer, read, options, pExtension);
  if (!pScene)
    return; // TODO: some sort of error?

  //...
}

} // namespace ud
