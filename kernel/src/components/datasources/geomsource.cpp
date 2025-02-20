#include "geomsource.h"
#include "ep/cpp/component/node/node.h"
#include "ep/cpp/component/node/udnode.h"
#include "ep/cpp/component/node/geomnode.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/resource/metadata.h"
#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/component/file.h"
#include "components/datasources/udsource.h"
#include "ep/cpp/kernel.h"
#include "hal/image.h"

#include "assimp/Importer.hpp"
#include "assimp/Exporter.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "ep/cpp/stringof.h"

#include <tuple>

namespace ep
{
const Array<const String> GeomSource::extensions = {
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

static inline String fromAIString(const aiString &name)
{
  return String(name.C_Str(), name.length);
}

static inline Float4 copyAIColor(const aiColor4D &m)
{
  return Float4::create(m.r, m.g, m.b, m.a);
}

static inline Double4x4 copyAIMatrix(const aiMatrix4x4 &m)
{
  // TODO: OH NO! swizzle into UD space!!
  return Double4x4::create(m.a1, m.b1, m.c1, m.d1,
                           m.a2, m.b2, m.c2, m.d2,
                           m.a3, m.b3, m.c3, m.d3,
                           m.a4, m.b4, m.c4, m.d4);
}

void GeomSource::create(StreamRef spSource)
{
  const char *pExtension = nullptr; // TODO: get the file extension to help the assimp!

  // allocate for file
  int64_t len = spSource->length();
  void *pBuffer = epAlloc((size_t)len);

  // read file from source
  Slice<void> buf(pBuffer, (size_t)len);
  buf = spSource->read(buf);
  EPASSERT((int64_t)buf.length == len, "!");

  Assimp::Importer importer;

  unsigned int options = //aiProcess_ConvertToLeftHanded      |
                         //aiProcess_FlipWindingOrder         |
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

  // parse materials
  parseMaterials(pScene);

  // parse meshes
  parseMeshes(pScene);

  // parse xrefs
  parseXRefs(pScene);

  // parse the scene
  if (pScene->mRootNode)
  {
    aiMatrix4x4 world;
    size_t numMeshes = 0; // TODO this is not being used
    NodeRef spRoot = parseNode(pScene, pScene->mRootNode, &world, numMeshes);

    // TODO: Massive hack !!!! Swizzle the matrix into ud space
    const Double4x4 &mat = spRoot->getMatrix();
    spRoot->setMatrix(Double4x4::create(mat.axis.x, mat.axis.z, -mat.axis.y, mat.axis.t));

    setResource("scene0", spRoot);
  }

  epFree(pBuffer);
}

bool GeomSource::write(const aiScene *pScene)
{
  Assimp::Exporter exporter;
  const aiExportDataBlob *pBlob;

  pBlob = exporter.ExportToBlob(pScene, "collada", aiProcess_ConvertToLeftHanded);
  if(!pBlob)
  {
    logDebug(1, "Unable to export Assimp scene to collada");
    return false;
  }

  try
  {
    StreamRef spFile = getKernel().createComponent<File>({ { "path", getURL() }, { "flags", FileOpenFlags::Create | FileOpenFlags::Write | FileOpenFlags::Text } });
    spFile->write(Slice<void>(pBlob->data, pBlob->size));
  }
  catch (EPException &)
  {
    logWarning(1, "Failed to open collada file for writing: \"{0}\"", getURL());
    exporter.FreeBlob();
    return false;
  }

  return true;
}

void GeomSource::parseMaterials(const aiScene *pScene)
{
  // copy all the materials
  for (uint32_t i = 0; i<pScene->mNumMaterials; ++i)
  {
    aiMaterial &aiMat = *pScene->mMaterials[i];

    aiString aiName;
    aiMat.Get(AI_MATKEY_NAME, aiName);
    logDebug(4, "Material {0}: \"{1}\"", i, fromAIString(aiName));

    String _name = fromAIString(aiName);
    MaterialRef spMat = getKernel().createComponent<Material>({ { "name", _name } });

    aiColor4D color(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_DIFFUSE, color);
    spMat->setMaterialProperty("diffuse", copyAIColor(color));

    color = aiColor4D(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_AMBIENT, color);
    spMat->setMaterialProperty("ambient", copyAIColor(color));

    color = aiColor4D(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_EMISSIVE, color);
    spMat->setMaterialProperty("emissive", copyAIColor(color));

    color = aiColor4D(1.f, 1.f, 1.f, 1.f);
    aiMat.Get(AI_MATKEY_COLOR_SPECULAR, color);
    spMat->setMaterialProperty("specular", copyAIColor(color));

    // TODO: foreach texture type...
    aiString texture;
    aiMat.Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture);
    if (texture.length > 0)
    {
      logDebug(4, "  Texture: {0}", fromAIString(texture));

      // TODO: try and load texture...
      //...
    }

    // add resource
    setResource(SharedString::concat("material", i), spMat);
  }
}

struct BinTan
{
  Float3 binormal;
  Float3 tangent;
};

static const ElementInfo binTanInfo[] = { { sizeof(Float3), { 3 }, ElementInfoFlags::Float },
                                          { sizeof(Float3), { 3 }, ElementInfoFlags::Float } };
} // ep

template<> // TODO: Make this not in the gobal namespace!!!!!!!
ep::SharedString stringof<ep::BinTan>()
{
  return ep::ElementInfo::buildTypeString(ep::binTanInfo);
}

namespace ep {

void GeomSource::parseMeshes(const aiScene *pScene)
{
  // copy all the meshes
  for (uint32_t i = 0; i<pScene->mNumMeshes; ++i)
  {
    aiMesh &mesh = *pScene->mMeshes[i];

    logDebug(4, "Mesh {0}: {1}", i, fromAIString(mesh.mName));

    // create a model
    ModelRef spMesh = getKernel().createComponent<Model>({ { "name", fromAIString(mesh.mName) } });

    // get material
    ResourceRef spMat = getResource(SharedString::concat("material", mesh.mMaterialIndex));
    if (spMat)
    {
      spMesh->setMaterial(component_cast<Material>(spMat));
      logDebug(4, "  Material: {0} ({1})", mesh.mMaterialIndex, spMat->getName());
    }
    else
    {
      // unknown material!
      logWarning(2, "  Material: Unknown material!");
    }

    // positions
    Slice<Float3> verts((Float3*)mesh.mVertices, mesh.mNumVertices);
    ArrayBufferRef spVerts = getKernel().createComponent<ArrayBuffer>();
    spVerts->allocateFromData<Float3>(verts);
    spVerts->getMetadata()->insertAt("a_position", "attributeInfo", 0, "name");

    setResource(SharedString::concat("positions", i), spVerts);

    spMesh->addVertexArray(spVerts);

    // normals
    if (mesh.HasNormals())
    {

      Slice<Float3> normals((Float3*)mesh.mNormals, mesh.mNumVertices);
      ArrayBufferRef spNormals = getKernel().createComponent<ArrayBuffer>();
      spNormals->allocateFromData<Float3>(normals);
      spNormals->getMetadata()->insertAt("a_normal", "attributeInfo", 0, "name");

      setResource(SharedString::concat("normals", i), spNormals);

      spMesh->addVertexArray(spNormals);
    }

    // binormals & tangents
    if (mesh.HasTangentsAndBitangents())
    {
      ArrayBufferRef spBinTan = getKernel().createComponent<ArrayBuffer>();
      spBinTan->allocate<BinTan>(mesh.mNumVertices);
      spBinTan->getMetadata()->insertAt("a_binormal", "attributeInfo", 0, "name");
      spBinTan->getMetadata()->insertAt("a_tangent", "attributeInfo", 1, "name");

      Slice<BinTan> bt = spBinTan->map<BinTan>();
      for (uint32_t j = 0; j<mesh.mNumVertices; ++j)
      {
        bt[j].binormal.x = mesh.mBitangents[j].x;
        bt[j].binormal.y = mesh.mBitangents[j].y;
        bt[j].binormal.z = mesh.mBitangents[j].z;
        bt[j].tangent.x = mesh.mTangents[j].x;
        bt[j].tangent.y = mesh.mTangents[j].y;
        bt[j].tangent.z = mesh.mTangents[j].z;
      }
      spBinTan->unmap();

      setResource(SharedString::concat("binormalstangents", i), spBinTan);

      spMesh->addVertexArray(spBinTan);
    }

    // UVs
    for (uint32_t t = 0; t<mesh.GetNumUVChannels(); ++t)
    {
      Slice<Float3> uvs((Float3*)mesh.mTextureCoords[t], mesh.mNumVertices);

      ArrayBufferRef spUVs = getKernel().createComponent<ArrayBuffer>();
      spUVs->allocateFromData<Float3>(uvs);
      spUVs->getMetadata()->insertAt(SharedString::concat("a_uv", t), "attributeInfo", t, "name");

      setResource(SharedString::concat("uvs", i, "_", t), spUVs);

      spMesh->addVertexArray(spUVs);
    }

    // Colors
    for (uint32_t c = 0; c<mesh.GetNumColorChannels(); ++c)
    {
      Slice<Float4> colors((Float4*)mesh.mColors[c], mesh.mNumVertices);

      ArrayBufferRef spColors = getKernel().createComponent<ArrayBuffer>();
      spColors->allocateFromData<Float4>(colors);
      spColors->getMetadata()->insertAt(SharedString::concat("a_color", c), "attributeInfo", c, "name");

      setResource(SharedString::concat("colors", i, "_", c), spColors);

      spMesh->addVertexArray(spColors);
    }

    // indices (faces)
    ArrayBufferRef spIndices = getKernel().createComponent<ArrayBuffer>();
    spIndices->allocate<uint32_t>(mesh.mNumFaces * 3);

    Slice<uint32_t> indices = spIndices->map<uint32_t>();
    uint32_t *pIndices = indices.ptr;
    for (uint32_t j = 0; j<mesh.mNumFaces; ++j)
    {
      aiFace &f = mesh.mFaces[j];
      EPASSERT(f.mNumIndices == 3, "Prim is not a triangle!");
      *pIndices++ = f.mIndices[0];
      *pIndices++ = f.mIndices[1];
      *pIndices++ = f.mIndices[2];
    }
    EPASSERT(pIndices - indices.ptr == (ptrdiff_t)indices.length, "Wrong number of indices?!");
    spIndices->unmap();

    setResource(SharedString::concat("indices", i), spIndices);

    spMesh->setIndexArray(spIndices);

    // add mesh resource
    setResource(SharedString::concat("mesh", i), spMesh);
  }
}

GeomSource::XRefType GeomSource::getXRefType(String url)
{
  String ext = url.getRightAtLast(".", true);
  Slice<const String> udDSExts = UDSource::staticGetFileExtensions();
  for (const String &udExt : udDSExts)
  {
    if (ext.eqIC(udExt))
      return XRefType::UDModel;
  }

  return XRefType::Unknown;
}

void GeomSource::parseXRefs(const aiScene *pScene)
{
  for (uint32_t i = 0; i<pScene->mNumXRefs; ++i)
  {
    aiXRef &xref = *pScene->mXRefs[i];
    String refName = String(fromAIString(xref.mName));
    String url = fromAIString(xref.mUrl);

    logDebug(4, "XRef {0}: {1} - \"{2}\"", i, refName, url);

    XRefType type = getXRefType(url);

    if (type == XRefType::UDModel)
    {
      // Load the UDDataSource
      UDSourceRef spModelDS;
      epscope(fail) { if (!spModelDS) pKernel->logError("GeomSource -- Failed to load UDModel \"{0}\"", url); };
      spModelDS = getKernel().createComponent<UDSource>({ {"name", refName }, {"src", url}, {"useStreamer", true} });

      UDModelRef spUDModel;
      if (spModelDS->getNumResources() > 0)
        spUDModel = spModelDS->getResourceAs<UDModel>(0);

      // add UDModel resource
      setResource(SharedString::concat("udmodel", i), spUDModel);
    }
    else
      logWarning(2, "GeomSource -- Unsupported XRef type \"{0}\"", url);
  }
}

NodeRef GeomSource::parseNode(const aiScene *pScene, aiNode *pNode, const aiMatrix4x4 *pParent, size_t &numMeshes, int depth)
{
  aiNode &node = *pNode;
  const aiMatrix4x4 &parent = *pParent;

  logDebug(4, "{1,*0}Node: {2}", depth, "", fromAIString(node.mName));

  aiMatrix4x4 &local = node.mTransformation;
  aiMatrix4x4 world = parent * local;

  logDebug(4, "{1,*0}  Local Position: %.2f,%.2f,%.2f", depth, "", local.a4, local.b4, local.c4);
  logDebug(4, "{1,*0}  Local Orientation: [%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f]", depth, "", local.a1, local.b1, local.c1, local.a2, local.b2, local.c2, local.a3, local.b3, local.c3);
  logDebug(4, "{1,*0}  World Position: %.2f,%.2f,%.2f", depth, "", world.a4, world.b4, world.c4);
  logDebug(4, "{1,*0}  World Orientation: [%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f],[%.1f,%.1f,%.1f]", depth, "", world.a1, world.b1, world.c1, world.a2, world.b2, world.c2, world.a3, world.b3, world.c3);

  // create bone from node
  NodeRef spNode = getKernel().createComponent<Node>({{ "name", fromAIString(node.mName) }});

//  if (node.mParent)
//    spNode->GetMetadata()->Insert("parent", fromAIString(node.mParent->mName));

  spNode->setMatrix(copyAIMatrix(local));

  // parse node mesh
  for (uint32_t i = 0; i<node.mNumMeshes; ++i)
  {
    ResourceRef spMesh = getResource(SharedString::concat("mesh", node.mMeshes[i]));
    if (spMesh)
    {
      // create geom node
      GeomNodeRef spGeomNode = getKernel().createComponent<GeomNode>();
      spGeomNode->setModel(shared_pointer_cast<Model>(spMesh));

      // add geom node to world node (we could collapse this if there is only one mesh...)
      spNode->addChild(spGeomNode);

      logDebug(4, "{1,*0}  Mesh {2}: {3} ({4})", depth, "", i, node.mMeshes[i], spMesh->getName());
    }
    else
    {
      // unknown mesh!
      logWarning(2, "{1,*0}  Mesh {2}: {3} Unknown mesh!!", depth, "", i, node.mMeshes[i]);
    }
  }

  // parse xrefs
  for (uint32_t i = 0; i<node.mNumXRefs; ++i)
  {
    aiXRef &xref = *pScene->mXRefs[node.mXRefs[i]];
    String url = fromAIString(xref.mUrl);

    XRefType type = getXRefType(url);

    if (type == XRefType::UDModel)
    {
      ResourceRef spUDModel = getResource(SharedString::concat("udmodel", node.mXRefs[i]));
      if (spUDModel)
      {
        // create UDNode
        UDNodeRef spUDNode = getKernel().createComponent<UDNode>();
        spUDNode->setUDModel(component_cast<UDModel>(spUDModel));

        // add UDNode to world node (we could collapse this if there is only one model...)
        spNode->addChild(spUDNode);

        logDebug(4, "{1,*0}  UDModel {2}: {3} ({4})", depth, "", i, node.mXRefs[i], spUDModel->getName());
      }
      else
        logWarning(2, "GeomSource -- Node references UDModel which does not exist");
    }
    else
      logWarning(2, "GeomSource -- Unsupported XRef type \"{0}\" in node", url);
  }

  // recurse children
  for (uint32_t i = 0; i<node.mNumChildren; ++i)
  {
    NodeRef spChild = parseNode(pScene, node.mChildren[i], &world, numMeshes, depth + 2);
    if (spChild)
      spNode->addChild(spChild);
  }

  return spNode;
}

void GeomSource::staticInit(ep::Kernel *pKernel)
{
  pKernel->registerExtensions(pKernel->getComponentDesc(componentID()), extensions);
}

} // namespace ep
