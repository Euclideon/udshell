#pragma once
#ifndef _UD_SHADER_H
#define _UD_SHADER_H

#include "resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Shader);

class Shader : public Resource
{
public:

protected:
  template<typename T>
  friend class udSharedPtr;

private:
  Shader() : Resource(ResourceType::Shader) {};
  virtual ~Shader();

};

} // namespace ud

#endif // _UD_SHADER_H
