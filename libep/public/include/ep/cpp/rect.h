#pragma once
#if !defined(EPRECT_HPP)
#define EPRECT_HPP

#include "ep/cpp/variant.h"

namespace ep {

  struct Rect
  {
    double x;
    double y;
    double width;
    double height;
  };

  Variant epToVariant(const Rect& area);
  void epFromVariant(const Variant &variant, Rect *pArea);

} // namespace ep

#endif // EPRECT_HPP

