#pragma once
#if !defined(EPBOUNDINGVOLUME_HPP)
#define EPBOUNDINGVOLUME_HPP

#include "ep/cpp/variant.h"
#include "ep/cpp/math.h"

namespace ep {

  struct BoundingVolume
  {
    Double3 min;
    Double3 max;
  };

  Variant epToVariant(const BoundingVolume &volume);
  void epFromVariant(const Variant &variant, BoundingVolume *pVolume);

} // namespace ep

#endif // EPBOUNDINGVOLUME_HPP

