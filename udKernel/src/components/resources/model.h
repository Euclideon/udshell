#pragma once
#ifndef _UD_MODEL_H
#define _UD_MODEL_H

#include "components/resources/resource.h"
#include "components/resources/array.h"
#include "components/resources/material.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Model);

class Model : public Resource
{
public:
  UD_COMPONENT(Model);

  UD_ENUM(ArrayType,
            Indices,
            Positions,
            Normals,
            BinormalsAndTangents,
            UV0, UV1, UV2, UV3, UV4, UV5, UV6, UV7,
            Color0, Color1, Color2, Color3, Color4, Color5, Color6, Color7,
            Max
          );

  ArrayBufferRef GetArray(ArrayType array) const { return spArrays[array]; }
  void SetArray(ArrayType array, ArrayBufferRef spArray) { spArrays[array] = spArray; }

  void SetMaterial(MaterialRef spMaterial) { this->spMaterial = spMaterial; }

  ArrayBufferRef GetPositionBuffer() const { return spArrays[ArrayType::Positions]; }

  void SetPositions(ArrayBufferRef spPositions) { spArrays[ArrayType::Positions] = spPositions; }
  void SetNormals(ArrayBufferRef spNormals) { spArrays[ArrayType::Normals] = spNormals; }
  void SetBinormalsAndTangents(ArrayBufferRef spBinormalsAndTangents) { spArrays[ArrayType::BinormalsAndTangents] = spBinormalsAndTangents; }
  void SetUVs(size_t uvChannel, ArrayBufferRef spUVs) { spArrays[ArrayType::UV0 + uvChannel] = spUVs; }
  void SetColors(size_t colorChannel, ArrayBufferRef spColors) { spArrays[ArrayType::Color0 + colorChannel] = spColors; }

  void SetIndices(ArrayBufferRef spIndices) { spArrays[ArrayType::Indices] = spIndices; }

protected:
  Model(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  ArrayBufferRef spArrays[ArrayType::Max];
  MaterialRef spMaterial;
};

} // namespace ud

#endif // _UD_MODEL_H
