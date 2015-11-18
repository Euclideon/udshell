#include "ep/cpp/rect.h"

namespace ep {

Variant epToVariant(const Rect& rect)
{
  Array<KeyValuePair> kvp(Reserve, 4);
  kvp.pushBack(KeyValuePair("x", rect.x));
  kvp.pushBack(KeyValuePair("y", rect.y));
  kvp.pushBack(KeyValuePair("width", rect.width));
  kvp.pushBack(KeyValuePair("height", rect.height));
  return std::move(kvp);
}

void epFromVariant(const Variant &variant, Rect *pArea)
{
  pArea->x = variant["x"].as<double>();
  pArea->y = variant["y"].as<double>();
  pArea->width = variant["width"].as<double>();
  pArea->height = variant["height"].as<double>();
}

} // namespace ep
