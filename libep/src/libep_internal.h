#pragma once
#if !defined(EPLIBEP_INTERNAL_H)
#define EPLIBEP_INTERNAL_H

// This file exists for the explicit purpose of working around MSVC's ITERATOR_DEBUG_LEVEL
// and runtime library mismatch errors when linking a release library with a debug build.
#if defined(_MSC_VER)
#define _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH
#define _ALLOW_RUNTIME_LIBRARY_MISMATCH
#endif // _MSC_VER)

#endif // EPLIBEP_INTERNAL_H
