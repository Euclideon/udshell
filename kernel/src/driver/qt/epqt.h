#pragma once
#if !defined(EPQT_H)
#define EPQT_H

#include "ep/c/platform.h"

#if defined(EP_COMPILER_VISUALC)
//# pragma warning(disable:4127) // conditional expression is constant
#elif defined(EP_COMPILER_GCC)
//# pragma GCC diagnostic ignored "-Wsomewarning"
#elif defined(EP_COMPILER_CLANG)
# if __clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 6)
#   pragma clang diagnostic ignored "-Winconsistent-missing-override"
# endif
#endif

#endif // EPQT_H
