#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/kernel.h"

namespace ep
{

  Array<const PropertyInfo> UDModel::GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(StartingRoot, "Normally zero, optionally set the starting root number (used with ForceSingleRoot flag)", nullptr, 0),
      EP_MAKE_PROPERTY(RenderClipRect, "Clipping Rect of the Screen", nullptr, 0),
      EP_MAKE_PROPERTY(RenderFlags, "UD Rendering Flags", nullptr, 0),
      EP_MAKE_PROPERTY_RO(DataSource, "Data Source for UD Model", nullptr, 0),
      EP_MAKE_PROPERTY_RO(UDScale, "Internal Scale of the Model", nullptr, 0),
      EP_MAKE_PROPERTY_RO(UDMatrix, "UD Matrix", nullptr, 0),
      EP_MAKE_PROPERTY_RO(BoundingVolume, "The Bouning Volume", nullptr, 0),
      EP_MAKE_PROPERTY_EXPLICIT("VoxelShader", "Voxel Shader", EP_MAKE_GETTER(GetVoxelVarDelegate), EP_MAKE_SETTER(SetVoxelVarDelegate), nullptr, 0)
    };
  }
}
