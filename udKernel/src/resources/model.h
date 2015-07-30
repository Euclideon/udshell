#pragma once
#ifndef _UD_MODEL_H
#define _UD_MODEL_H

#include "resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Model);

class Model : public Resource
{
public:

protected:
  template<typename T>
  friend class udSharedPtr;

private:
  Model() : Resource(ResourceType::Model) {};
  virtual ~Model();
};

} // namespace ud

#endif // _UD_MODEL_H
