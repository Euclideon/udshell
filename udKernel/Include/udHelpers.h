#pragma once
#ifndef UDHELPERS_H
#define UDHELPERS_H

#include "udString.h"
#include "udSharedPtr.h"

struct udKeyValuePair;
class udComponent;
typedef udSharedPtr<udComponent> udComponentRef;

struct udVariant
{
public:
  enum class Type
  {
    Null,
    Bool,
    Int,
    Float,
    String,
    Array,
    Component
  };

  udVariant() : t(Type::Null) {}
  udVariant(bool b) : t(Type::Bool), b(b) {}
  udVariant(int64_t i) : t(Type::Int), i(i) {}
  udVariant(double f) : t(Type::Float), f(f) {}
  udVariant(udString _s) : t(Type::String) { s.ptr = _s.ptr; s.length = _s.length; }
  udVariant(udSlice<udVariant> a) : t(Type::Array) { s.ptr = a.ptr; s.length = a.length; }
  udVariant(udComponentRef &_c) : t(Type::Component) { c = _c.ptr(); }

  Type type() const { return t; }

  udRCString stringify() const
  {
    if (t == Type::String) // HACK: return if is string...
      return asString();
    // TODO: ...
    return nullptr;
  }

  bool asBool() const
  {
    switch (t)
    {
      case Type::Null:
        return false;
      case Type::Bool:
        return b;
      case Type::Int:
        return !!i;
      case Type::Float:
        return f != 0;
      case Type::String:
      {
        udString s((const char*)s.ptr, s.length);
        if (s.eqi("true"))
          return true;
        else if (s.eqi("false"))
          return false;
        return !s.empty();
      }
      default:
        UDASSERT(t == Type::Array, "Wrong type!");
        return false;
    }
  }
  int64_t asInt() const
  {
    switch (t)
    {
      case Type::Null:
        return 0;
      case Type::Bool:
        return b ? 1 : 0;
      case Type::Int:
        return i;
      case Type::Float:
        return (int64_t)f;
      case Type::String:
        return udString((const char*)s.ptr, s.length).parseInt();
      default:
        UDASSERT(t == Type::Array, "Wrong type!");
        return 0;
    }
  }
  double asFloat() const
  {
    switch (t)
    {
      case Type::Null:
        return 0.0;
      case Type::Bool:
        return b ? 1.0 : 0.0;
      case Type::Int:
        return (double)i;
      case Type::Float:
        return f;
      case Type::String:
        return udString((const char*)s.ptr, s.length).parseFloat();
      default:
        UDASSERT(t == Type::Array, "Wrong type!");
        return 0.0;
    }
  }
  udString asString() const
  {
    // TODO: it would be nice to be able to string-ify other types
    // ...but we don't have any output buffer
    switch (t)
    {
      case Type::Null:
        return udString();
      case Type::String:
        return udString((const char*)s.ptr, s.length);
      default:
        UDASSERT(t == Type::String, "Wrong type!");
        return udString();
    }
  }
  udSlice<udVariant> asArray() const
  {
    switch (t)
    {
      case Type::Null:
        return udSlice<udVariant>();
      default:
        UDASSERT(t == Type::String, "Wrong type!");
        return udSlice<udVariant>();
    }
  }
  udComponentRef asComponent() const
  {
    switch (t)
    {
      case Type::Component:
        return udComponentRef(c);
      default:
        UDASSERT(t == Type::Component, "Wrong type!");
        return udComponentRef();
    }
  }

private:
  Type t;
  union
  {
    struct
    {
      size_t length;
      const void *ptr;
    } s;
    int64_t i;
    double f;
    bool b;
    udComponent *c;
  };
};

struct udKeyValuePair
{
  udKeyValuePair(udString key, udVariant value) : key(key), value(value) {}

  udString key;
  udVariant value;
};
typedef udSlice<udKeyValuePair> udInitParams;


udRCSlice<udKeyValuePair> udParseCommandLine(const char *pCommandLine);
udRCSlice<udKeyValuePair> udParseCommandLine(int argc, char *argv[]);
udRCSlice<udKeyValuePair> udParseCommandLine(uint32_t argc, const char* argn[], const char* argv[]);

#endif // UDHELPERS_H
