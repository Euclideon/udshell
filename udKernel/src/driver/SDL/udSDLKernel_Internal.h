#pragma once
#ifndef UDSDLKERNEL_INTERNAL_H
#define UDSDLKERNEL_INTERNAL_H

#include "kernel.h"

namespace ud
{
  class SDLKernel : public Kernel
  {
  public:
    udResult DoInit(Kernel *pKernel) { return Kernel::DoInit(pKernel); }
  };

} // namespace ud

#endif  // UDSDLKERNEL_INTERNAL_H
