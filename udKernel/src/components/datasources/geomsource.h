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
    return {
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
  }

protected:
  GeomSource(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : DataSource(pType, pKernel, uid, initParams)
  {}

  void Create(StreamRef spSource) override;

  NodeRef ParseNode(const aiScene *pScene, aiNode *pNode, const aiMatrix4x4 *pParent, size_t &numMeshes, int depth = 0);
};

} // namespace ud

#endif // _UD_GEOMSOURCE_H
