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

  ArrayBufferRef GetPositionBuffer() const { return spPositions; }

  void SetMaterial(MaterialRef spMaterial) { this->spMaterial = spMaterial; }

  void SetPositions(ArrayBufferRef spPositions) { this->spPositions = spPositions; }
  void SetNormals(ArrayBufferRef spNormals) { this->spNormals = spNormals; }
  void SetBinormalsAndTangents(ArrayBufferRef spBinormalsAndTangents) { this->spBinormalsTangents = spBinormalsAndTangents; }
  void SetUVs(size_t uvChannel, ArrayBufferRef spUVs) { this->spUVs[uvChannel] = spUVs; }
  void SetColors(size_t colorChannel, ArrayBufferRef spColors) { this->spColors[colorChannel] = spColors; }

  void SetIndices(ArrayBufferRef spIndices) { this->spIndices = spIndices; }

protected:
  Model(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  ArrayBufferRef spPositions = nullptr;
  ArrayBufferRef spNormals = nullptr;
  ArrayBufferRef spBinormalsTangents = nullptr;
  ArrayBufferRef spUVs[8];
  ArrayBufferRef spColors[8];
  ArrayBufferRef spIndices = nullptr;
  MaterialRef spMaterial = nullptr;
};

} // namespace ud

#endif // _UD_MODEL_H
