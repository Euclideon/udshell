#include "shaderimpl.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> Shader::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(Code, "The code string to be compiled", nullptr, 0),
  };
}

} // namespace ep
