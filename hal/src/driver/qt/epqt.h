#pragma once
#if !defined(EPQT_H)
#define EPQT_H

#include "ep/c/platform.h"

#include <QtGlobal>

#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
#if defined(EP_COMPILER_VISUALC)
# pragma warning(disable:4718) // TODO: this can be removed once we are on Qt 5.6 -- QMap triggers warnings: recursive call has no side effects, deleting
#endif
#endif

#if defined(EP_COMPILER_GCC)
//# pragma GCC diagnostic ignored "-Wsomewarning"
#elif defined(EP_COMPILER_CLANG)
# if __clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 6)
#   pragma clang diagnostic ignored "-Winconsistent-missing-override"
# endif
#endif

#endif // EPQT_H
