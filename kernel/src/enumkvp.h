#pragma once
#ifndef EPENUMKVP_H
#define EPENUMKVP_H

#include "ep/cpp/platform.h"

namespace ep {

struct EnumKVP
{
  EnumKVP(String key, int64_t v) : key(key), value(v) {}

  String key;
  int64_t value;
};
#define EnumKVP(e) EnumKVP( #e, (int64_t)e )

} // namespace ep

#endif // EPENUMKVP_H
