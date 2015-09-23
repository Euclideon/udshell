#include "geomsource.h"
#include "components/nodes/node.h"
#include "components/nodes/geomnode.h"
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
const udFixedSlice<const udString> GeomSource::extensions = {
  // mesh formats
  ".fbx", // Autodesk FBX
  ".dae", // Collada
  ".blend", // Blender 3D
  ".3ds", // 3DS Max 3DS
  ".ase", // 3DS Max ASE
  ".obj", // Wavefront Object
  ".ifc", // Industry Foundation Classes(IFC/Step)
  ".xgl", ".zgl", // XGL
  ".ply", // Stanford Polygon Library
  ".dxf", // AutoCAD DXF (*limited)
  ".lwo", // LightWave
  ".lws", // LightWave Scene
  ".lxo", // Modo
  ".stl", // Stereolithography
  ".x", // DirectX X
  ".ac", // AC3D
  ".ms3d", // Milkshape 3D
  ".cob", ".scn", //TrueSpace (*limited)

  // mocap formats
  ".bvh", // Biovision BVH
  ".csm", // CharacterStudio Motion (*limited)
};

ComponentDesc GeomSource::descriptor =
{
  &DataSource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "geomsource", // id
  "Geometry Source", // displayName
  "Provides polygon geometry", // description

  nullptr,            // properties
  nullptr,            // methods
  nullptr,            // events
  nullptr,            // static functions
  &RegisterExtensions // init
};

static inline udString FromAIString(const aiString &name)
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
  void *pBuffer = udAlloc((size_t)len);

  // read file from source
  udSlice<void> buf(pBuffer, (size_t)len);
  buf = spSource->Read(buf);
  UDASSERT((int64_t)buf.length == len, "!");

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

  const aiScene *pScene = importer.ReadFileFromMemory(buf.ptr, buf.length, options, pExtension);
  if (!pScene)
    return; // TODO: some sort of error?

  // marse materials
  ParseMaterials(pScene);

  // marse meshes
  ParseMeshes(pScene);

  // parse the scene
  if (pScene->mRootNode)
  {
    aiMatrix4x4 world;
    size_t numMeshes = 0;
    NodeRef spRoot = ParseNode(pScene, pScene->mRootNode, &world, numMeshes);
    resources.Insert("scene0", spRoot);
  }
}

void GeomSource::ParseMaterials(const aiScene *pScene)
{
  // copy all the materials
  for (uint32_t i = 0; i<pScene->mNumMaterials; ++i)
  {
    aiMaterial &aiMat = *pScene->mMaterials[i];

    aiString _name;
    aiMat.Get(AI_MATKEY_NAME, _name);
    LogDebug(4, "Material {0}: \"{1}\"", i, FromAIString(_name));

    udString name = FromAIString(_name);
    MaterialRef spMat = pKernel->CreateComponent<Material>({ { "name", name } });

    aiColor4D color(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_DIFFUSE, color);
    spMat->SetMaterialProperty("diffuse", CopyAIColor(color));

    color = aiColor4D(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_AMBIENT, color);
    spMat->SetMaterialProperty("ambient", CopyAIColor(color));

    color = aiColor4D(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_EMISSIVE, color);
    spMat->SetMaterialProperty("emissive", CopyAIColor(color));

    color = aiColor4D(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_SPECULAR, color);
    spMat->SetMaterialProperty("specular", CopyAIColor(color));

    // TODO: foreach texture type...
    aiString texture;
    aiMat.Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture);
    if (texture.length > 0)
    {
      LogDebug(4, "  Texture: {0}", FromAIString(texture));

      // TODO: try and load texture...
      //...
    }

    // add resource
    resources.Insert(udSharedString::concat("material", i), spMat);
  }

  udFree(pBuffer);
}

void GeomSource::ParseMeshes(const aiScene *pScene)
{
  // copy all the meshes
  for (uint32_t i = 0; i<pScene->mNumMeshes; ++i)
  {
    aiMesh &mesh = *pScene->mMeshes[i];

    LogDebug(4, "Mesh {0}: {1}", i, FromAIString(mesh.mName));

    // create a model
    ModelRef spMesh = pKernel->CreateComponent<Model>({ { "name", FromAIString(mesh.mName) } });

    // get material
    ResourceRef *pspMat = resources.Get(udSharedString::concat("material", mesh.mMaterialIndex));
    if (pspMat)
    {
      spMesh->SetMaterial(component_cast<Material>(*pspMat));
      LogDebug(4, "  Material: {0} ({1})", mesh.mMaterialIndex, (*pspMat)->GetName());
    }
    else
    {
      // unknown material!
      LogWarning(2, "  Material: Unknown material!");
    }

    // positions
    typedef std::tuple<float[3]> VertPos;
    udSlice<VertPos> verts((VertPos*)mesh.mVertices, mesh.mNumVertices);

    ArrayBufferRef spVerts = pKernel->CreateComponent<ArrayBuffer>();
    spVerts->AllocateFromData<VertPos>(verts);

    resources.Insert(udSharedString::concat("positions", i), spVerts);

    spMesh->SetVertexArray(spVerts, { "a_position" });

    // normals
    if (mesh.HasNormals())
    {
      typedef std::tuple<float[3]> VertNorm;
      udSlice<VertNorm> normals((VertNorm*)mesh.mNormals, mesh.mNumVertices);

      ArrayBufferRef spNormals = pKernel->CreateComponent<ArrayBuffer>();
      spNormals->AllocateFromData<VertNorm>(normals);

      resources.Insert(udSharedString::concat("normals", i), spNormals);

      spMesh->SetVertexArray(spNormals, { "a_normal" });
    }

    // binormals & tangents
    if (mesh.HasTangentsAndBitangents())
    {
      typedef std::tuple<float[3], float[3]> VertBinTan;

      ArrayBufferRef spBinTan = pKernel->CreateComponent<ArrayBuffer>();
      spBinTan->Allocate<VertBinTan>(mesh.mNumVertices);

      udSlice<VertBinTan> bt = spBinTan->Map<VertBinTan>();
      for (uint32_t j = 0; j<mesh.mNumVertices; ++j)
      {
        std::get<0>(bt.ptr[j])[0] = mesh.mBitangents[j].x;
        std::get<0>(bt.ptr[j])[1] = mesh.mBitangents[j].y;
        std::get<0>(bt.ptr[j])[2] = mesh.mBitangents[j].z;
        std::get<1>(bt.ptr[j])[0] = mesh.mTangents[j].x;
        std::get<1>(bt.ptr[j])[1] = mesh.mTangents[j].y;
        std::get<1>(bt.ptr[j])[2] = mesh.mTangents[j].z;
      }
      spBinTan->Unmap();

      resources.Insert(udSharedString::concat("binormalstangents", i), spBinTan);

      spMesh->SetVertexArray(spBinTan, { "a_binormal", "a_tangent" });
    }

    // UVs
    for (uint32_t t = 0; t<mesh.GetNumUVChannels(); ++t)
    {
      typedef std::tuple<float[3]> VertUV;
      udSlice<VertUV> uvs((VertUV*)mesh.mTextureCoords[t], mesh.mNumVertices);

      ArrayBufferRef spUVs = pKernel->CreateComponent<ArrayBuffer>();
      spUVs->AllocateFromData<VertUV>(uvs);

      resources.Insert(udSharedString::concat("uvs", i, "_", t), spUVs);

      spMesh->SetVertexArray(spUVs, { udSharedString::concat("a_uv", t) });
    }

    // Colors
    for (uint32_t c = 0; c<mesh.GetNumColorChannels(); ++c)
    {
      typedef std::tuple<float[4]> VertColor;
      udSlice<VertColor> colors((VertColor*)mesh.mColors[c], mesh.mNumVertices);

      ArrayBufferRef spColors = pKernel->CreateComponent<ArrayBuffer>();
      spColors->AllocateFromData<VertColor>(colors);

      resources.Insert(udSharedString::concat("colors", i, "_", c), spColors);

      spMesh->SetVertexArray(spColors, { udSharedString::concat("a_color", c) });
    }

    // indices (faces)
    ArrayBufferRef spIndices = pKernel->CreateComponent<ArrayBuffer>();
    spIndices->Allocate<uint32_t>(mesh.mNumFaces * 3);

    udSlice<uint32_t> indices = spIndices->Map<uint32_t>();
    uint32_t *pIndices = indices.ptr;
    for (uint32_t j = 0; j<mesh.mNumFaces; ++j)
    {
      aiFace &f = mesh.mFaces[j];
      UDASSERT(f.mNumIndices == 3, "Prim is not a triangle!");
      *pIndices++ = f.mIndices[0];
      *pIndices++ = f.mIndices[1];
      *pIndices++ = f.mIndices[2];
    }
    UDASSERT(pIndices - indices.ptr == (ptrdiff_t)indices.length, "Wrong number of indices?!");
    spIndices->Unmap();

    resources.Insert(udSharedString::concat("indices", i), spIndices);

    spMesh->SetIndexArray(spIndices);

    // add mesh resource
    resources.Insert(udSharedString::concat("mesh", i), spMesh);
  }
}

NodeRef GeomSource::ParseNode(const aiScene *pScene, aiNode *pNode, const aiMatrix4x4 *pParent, size_t &numMeshes, int depth)
{
  aiNode &node = *pNode;
  const aiMatrix4x4 &parent = *pParent;

  LogDebug(4, "{1,*0}Node: {2}", depth, "", FromAIString(node.mName));

  aiMatrix4x4 &local = node.mTransformation;
  aiMatrix4x4 world = parent * local;

  LogDebug(4, "{1,*0}  Local Position: %.2f,%.2f,%.2f", depth, "", local.a4, local.b4, local.c4);
  LogDebug(4, "{1,*0}  Local Orientation: [%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f]", depth, "", local.a1, local.b1, local.c1, local.a2, local.b2, local.c2, local.a3, local.b3, local.c3);
  LogDebug(4, "{1,*0}  World Position: %.2f,%.2f,%.2f", depth, "", world.a4, world.b4, world.c4);
  LogDebug(4, "{1,*0}  World Orientation: [%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f]", depth, "", world.a1, world.b1, world.c1, world.a2, world.b2, world.c2, world.a3, world.b3, world.c3);

  // create bone from node
  NodeRef spNode = pKernel->CreateComponent<Node>({{ "name", FromAIString(node.mName) }});

//  if (node.mParent)
//    spNode->GetMetadata()->Insert("parent", FromAIString(node.mParent->mName));

  spNode->SetMatrix(CopyAIMatrix(local));

  // parse node mesh
  for (uint32_t i = 0; i<node.mNumMeshes; ++i)
  {
    ResourceRef *pspMesh = resources.Get(udSharedString::concat("mesh", node.mMeshes[i]));
    if (pspMesh)
    {
      // create geom node
      GeomNodeRef spGeomNode = pKernel->CreateComponent<GeomNode>();
      spGeomNode->SetModel(shared_pointer_cast<Model>(*pspMesh));

      // add geom node to world node (we could collapse this if there is only one mesh...)
      spNode->AddChild(spGeomNode);

      LogDebug(4, "{1,*0}  Mesh {2}: {3} ({4})", depth, "", i, node.mMeshes[i], (*pspMesh)->name);
    }
    else
    {
      // unknown mesh!
      LogWarning(2, "{1,*0}  Mesh {2}: {3} Unknown mesh!!", depth, "", i, node.mMeshes[i]);
    }
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

udResult GeomSource::RegisterExtensions(Kernel *pKernel)
{
  return pKernel->RegisterExtensions(&descriptor, extensions);
}

} // namespace ud
