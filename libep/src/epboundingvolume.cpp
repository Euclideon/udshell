#include "libep_internal.h"
#include "ep/cpp/boundingvolume.h"

namespace ep {

Variant epToVariant(const BoundingVolume &volume)
{
  Array<KeyValuePair, 2> kvp(Reserve, 2);
  kvp.pushBack(KeyValuePair("min", volume.min));
  kvp.pushBack(KeyValuePair("max", volume.max));
  return std::move(kvp);
}

void epFromVariant(const Variant &variant, BoundingVolume *pVolume)
{
  pVolume->min = variant["min"].as<Double3>();
  pVolume->max = variant["max"].as<Double3>();
}

} // namespace ep
