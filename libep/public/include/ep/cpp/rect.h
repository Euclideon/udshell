#pragma once
#if !defined(EPRECT_HPP)
#define EPRECT_HPP

#include "ep/cpp/variant.h"

namespace ep {

  template<typename T>
  struct Dimensions
  {
    T width;
    T height;
  };

  template<typename T>
  struct Rect
  {
    T x;
    T y;
    T width;
    T height;
  };

  template<typename T>
  inline Variant epToVariant(Dimensions<T> v)
  {
    return Variant::VarMap{ KeyValuePair{ "width", v.width }, KeyValuePair{ "height", v.height } };
  }
  template<typename T>
  inline Variant epToVariant(Rect<T> v)
  {
    return Variant::VarMap{ KeyValuePair{ "x", v.x }, KeyValuePair{ "y", v.y },
                            KeyValuePair{ "width", v.width }, KeyValuePair{ "height", v.height } };
  }

  template<typename T>
  inline void epFromVariant(const Variant &v, Dimensions<T> *pV)
  {
    pV->width = v["width"].as<T>();
    pV->height = v["height"].as<T>();
  }
  template<typename T>
  inline void epFromVariant(const Variant &v, Rect<T> *pV)
  {
    pV->x = v["x"].as<T>();
    pV->y = v["y"].as<T>();
    pV->width = v["width"].as<T>();
    pV->height = v["height"].as<T>();
  }

} // namespace ep

#endif // EPRECT_HPP

