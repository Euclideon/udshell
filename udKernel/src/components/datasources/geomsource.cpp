#include "geomsource.h"
#include "components/nodes/node.h"
#include "components/resources/array.h"
#include "components/resources/metadata.h"
#include "components/resources/model.h"
#include "components/resources/material.h"
#include "hal/image.h"
#include "kernel.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <tuple>

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

static inline udString FromAIString(aiString name)
{
  return udString(name.C_Str(), name.length);
}

static inline udFloat4 CopyAIColor(const aiColor4D &m)
{
  return udFloat4::create(m.r, m.g, m.b, m.a);
}

static inline udDouble4x4 CopyAIMatrix(const aiMatrix4x4 &m)
{
  // TODO: OH NO! swizzle into UD space!!
  return udDouble4x4::create(m.a1, m.b1, m.c1, m.d1,
                             m.a2, m.b2, m.c2, m.d2,
                             m.a3, m.b3, m.c3, m.d3,
                             m.a4, m.b4, m.c4, m.d4);
}

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
//                         aiProcess_CalcTangentSpace         |
                         aiProcess_Triangulate              |
                         aiProcess_GenUVCoords              |
                         aiProcess_JoinIdenticalVertices    |
                         aiProcess_FindDegenerates          |
                         aiProcess_SortByPType              |
                         aiProcess_RemoveRedundantMaterials |
//                         aiProcess_ImproveCacheLocality     | // this needs to be configured for each target...
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

  // copy all the materials
  for (uint32_t i = 0; i<pScene->mNumMaterials; ++i)
  {
    aiMaterial &aiMat = *pScene->mMaterials[i];

    aiString _name;
    aiMat.Get(AI_MATKEY_NAME, _name);
    udDebugPrintf("Material %d: %s", i, _name.C_Str());

    udString name = FromAIString(_name);
    MaterialRef spMat = pKernel->CreateComponent<Material>({ { "name", name } });

    aiColor4D colour(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_DIFFUSE, colour);
    spMat->SetMaterialProperty(MaterialProperties::DiffuseColor, CopyAIColor(colour));

    colour = aiColor4D(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_AMBIENT, colour);
    spMat->SetMaterialProperty(MaterialProperties::AmbientColor, CopyAIColor(colour));

    colour = aiColor4D(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_EMISSIVE, colour);
    spMat->SetMaterialProperty(MaterialProperties::EmissiveColor, CopyAIColor(colour));

    colour = aiColor4D(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_SPECULAR, colour);
    spMat->SetMaterialProperty(MaterialProperties::SpecularColor, CopyAIColor(colour));

    // TODO: foreach texture type, get texture
    aiString texture;
    aiMat.Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture);
    udDebugPrintf("  Texture: %s", texture.C_Str());

    // TODO: try and load texture...
    //...

    // add resource
    udFixedString64 buffer = udFixedString64::format("material%d", i);
    resources.Insert(buffer, spMat);
  }

  // parse the scene
  if (pScene->mRootNode)
  {
    aiMatrix4x4 world;
    size_t numMeshes = 0;
    NodeRef spRoot = ParseNode(pScene, pScene->mRootNode, &world, numMeshes);
    resources.Insert("scene0", spRoot);
  }
}

NodeRef GeomSource::ParseNode(const aiScene *pScene, aiNode *pNode, const aiMatrix4x4 *pParent, size_t &numMeshes, int depth)
{
  aiNode &node = *pNode;
  const aiMatrix4x4 &parent = *pParent;

  udDebugPrintf("%*sNode: %s", depth, " ", node.mName.C_Str());

  aiMatrix4x4 &local = node.mTransformation;
  aiMatrix4x4 world = parent * local;

  udDebugPrintf("%*s  Local Position: %.2f,%.2f,%.2f", depth, " ", local.a4, local.b4, local.c4);
  udDebugPrintf("%*s  Local Orientation: [%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f]", depth, " ", local.a1, local.b1, local.c1, local.a2, local.b2, local.c2, local.a3, local.b3, local.c3);
  udDebugPrintf("%*s  World Position: %.2f,%.2f,%.2f", depth, " ", world.a4, world.b4, world.c4);
  udDebugPrintf("%*s  World Orientation: [%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f]", depth, " ", world.a1, world.b1, world.c1, world.a2, world.b2, world.c2, world.a3, world.b3, world.c3);

  // create bone from node
  NodeRef spNode = pKernel->CreateComponent<Node>({{ "name", FromAIString(node.mName) }});

//  if (node.mParent)
//    spNode->GetMetadata()->Insert("parent", FromAIString(node.mParent->mName));

  spNode->SetMatrix(CopyAIMatrix(local));

  // parse node mesh
  for (uint32_t i = 0; i<node.mNumMeshes; ++i)
  {
    aiMesh &mesh = *pScene->mMeshes[node.mMeshes[i]];

    udDebugPrintf("%*sMesh %d: %d (%s)", depth, " ", i, node.mMeshes[i], mesh.mName.C_Str());

    // create a model
    ModelRef spMesh = pKernel->CreateComponent<Model>({ { "name", FromAIString(mesh.mName) } });

    // get material
    udFixedString64 resName = udFixedString64::format("material%d", mesh.mMaterialIndex);
    ResourceRef *pspMat = resources.Get(resName);
    if (pspMat)
    {
      spMesh->SetMaterial(component_cast<Material>(*pspMat));
      udDebugPrintf("%*s  Material: %d (%s)", depth, " ", mesh.mMaterialIndex, (*pspMat)->GetName().toStringz());
    }

    // positions
    typedef std::tuple<float[3]> VertPos;
    udSlice<VertPos> verts((VertPos*)mesh.mVertices, mesh.mNumVertices);

    ArrayBufferRef spVerts = pKernel->CreateComponent<ArrayBuffer>();
    spVerts->AllocateFromData<VertPos>(verts);

    resName = udFixedString64::format("positions%d", numMeshes);
    resources.Insert(resName, spVerts);

    spMesh->SetPositions(spVerts);

    // normals
    if (mesh.HasNormals())
    {
      typedef std::tuple<float[3]> VertNorm;
      udSlice<VertNorm> normals((VertNorm*)mesh.mNormals, mesh.mNumVertices);

      ArrayBufferRef spNormals = pKernel->CreateComponent<ArrayBuffer>();
      spNormals->AllocateFromData<VertNorm>(normals);

      resName = udFixedString64::format("normals%d", numMeshes);
      resources.Insert(resName, spNormals);

      spMesh->SetNormals(spNormals);
    }

    // binormals & tangents
    if (mesh.HasTangentsAndBitangents())
    {
      typedef std::tuple<float[3], float[3]> VertBinTan;

      ArrayBufferRef spBinTan = pKernel->CreateComponent<ArrayBuffer>();
      spBinTan->Allocate<VertBinTan>(mesh.mNumVertices);

      VertBinTan *pBT = spBinTan->Map<VertBinTan>();
      for (uint32_t j = 0; j<mesh.mNumVertices; ++j)
      {
        std::get<0>(pBT[j])[0] = mesh.mBitangents[j].x;
        std::get<0>(pBT[j])[1] = mesh.mBitangents[j].y;
        std::get<0>(pBT[j])[2] = mesh.mBitangents[j].z;
        std::get<1>(pBT[j])[0] = mesh.mTangents[j].x;
        std::get<1>(pBT[j])[1] = mesh.mTangents[j].y;
        std::get<1>(pBT[j])[2] = mesh.mTangents[j].z;
      }
      spBinTan->Unmap();

      resName = udFixedString64::format("binormalstangents%d", numMeshes);
      resources.Insert(resName, spBinTan);

      spMesh->SetBinormalsAndTangents(spBinTan);
    }

    // UVs
    for (uint32_t t = 0; t<mesh.GetNumUVChannels(); ++t)
    {
      typedef std::tuple<float[3]> VertUV;
      udSlice<VertUV> uvs((VertUV*)mesh.mTextureCoords[t], mesh.mNumVertices);

      ArrayBufferRef spUVs = pKernel->CreateComponent<ArrayBuffer>();
      spUVs->AllocateFromData<VertUV>(uvs);

      resName = udFixedString64::format("uvs%d_%d", numMeshes, t);
      resources.Insert(resName, spUVs);

      spMesh->SetUVs(t, spUVs);
    }

    // Colors
    for (uint32_t t = 0; t<mesh.GetNumColorChannels(); ++t)
    {
      typedef std::tuple<float[4]> VertColor;
      udSlice<VertColor> colors((VertColor*)mesh.mColors[t], mesh.mNumVertices);

      ArrayBufferRef spColors = pKernel->CreateComponent<ArrayBuffer>();
      spColors->AllocateFromData<VertColor>(colors);

      resName = udFixedString64::format("colors%d_%d", numMeshes, t);
      resources.Insert(resName, spColors);

      spMesh->SetColors(t, spColors);
    }

    // indices (faces)
    ArrayBufferRef spIndices = pKernel->CreateComponent<ArrayBuffer>();
    spIndices->Allocate<uint32_t>(mesh.mNumFaces * 3);

    uint32_t *pIndices = spIndices->Map<uint32_t>();
    for (uint32_t j = 0; j<mesh.mNumFaces; ++j)
    {
      aiFace &f = mesh.mFaces[j];
      UDASSERT(f.mNumIndices == 3, "Prim is not a triangle!");
      *pIndices++ = f.mIndices[0];
      *pIndices++ = f.mIndices[1];
      *pIndices++ = f.mIndices[2];
    }
    spIndices->Unmap();

    resName = udFixedString64::format("indices%d", numMeshes);
    resources.Insert(resName, spIndices);

    spMesh->SetIndices(spIndices);

    // add mesh resource
    resName = udFixedString64::format("mesh%d", numMeshes++);
    resources.Insert(resName, spMesh);
  }

  // recurse children
  for (uint32_t i = 0; i<node.mNumChildren; ++i)
  {
    NodeRef spChild = ParseNode(pScene, node.mChildren[i], &world, numMeshes, depth + 2);
    if (spChild)
      spNode->AddChild(spChild);
  }

  return spNode;
}

} // namespace ud
