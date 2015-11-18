#include "ep/cpp/boundingvolume.h"

namespace ep {

Variant epToVariant(const BoundingVolume &volume)
{
  Array<KeyValuePair> kvp(Reserve, 6);
  kvp.pushBack(KeyValuePair("minx", volume.min.x));
  kvp.pushBack(KeyValuePair("miny", volume.min.y));
  kvp.pushBack(KeyValuePair("minz", volume.min.z));
  kvp.pushBack(KeyValuePair("maxx", volume.max.x));
  kvp.pushBack(KeyValuePair("maxy", volume.max.y));
  kvp.pushBack(KeyValuePair("maxz", volume.max.z));
  return std::move(kvp);
}

void epFromVariant(const Variant &variant, BoundingVolume *pVolume)
{
  pVolume->min.x = variant["minx"].as<double>();
  pVolume->min.y = variant["miny"].as<double>();
  pVolume->min.z = variant["minz"].as<double>();
  pVolume->max.x = variant["maxx"].as<double>();
  pVolume->max.y = variant["maxy"].as<double>();
  pVolume->max.z = variant["maxz"].as<double>();
}

} // namespace ep
