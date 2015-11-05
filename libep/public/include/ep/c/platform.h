#if !defined(_EP_PLATFORM_H)
#define _EP_PLATFORM_H

/*
  We'll define a few #define's to tell us about the build target:

  Compilers:
    EP_COMPILER_VISUALC
    EP_COMPILER_GCC
    EP_COMPILER_CLANG

  Assembler Syntax:
    EP_ASM_INTEL
    EP_ASM_ATNT

  Architectures:
    EP_ARCH_X86
    EP_ARCH_X64
    EP_ARCH_ITANIUM
    EP_ARCH_PPC
    EP_ARCH_MIPS
    EP_ARCH_SH4
    EP_ARCH_SPU
    EP_ARCH_ARM
    EP_ARCH_68K
    EP_ARCH_LLVM_IR

  Endian:
    EP_ENDIAN_LITTLE
    EP_ENDIAN_BIG

  Data word:
    EP_32BIT
    EP_64BIT

  Platform:
    EP_WINDOWS
    EP_LINUX
    EP_OSX
    EP_IPHONE
    EP_ANDROID
    EP_NACL
    EP_WEB
*/

// detect compiler
#if defined(__clang__)
# define EP_COMPILER_CLANG
#elif defined(_MSC_VER)
# define EP_COMPILER_VISUALC
#elif defined(__GNUC__)
# define EP_COMPILER_GCC
# define EP_GCC_VERSION (GNUC__ * 10000 + __GNUC_MINOR__ * 100  + __GNUC_PATCHLEVEL__)
#else
# error "Unrecognised compiler"
#endif

// detect C++11
#if __cplusplus >= 201103L || _MSC_VER >= 1700
# define EP_CPP11 1
#else
# define EP_CPP11 0
#endif


// detect architecture/platform
#if defined(_WIN32)
# define EP_WINDOWS

# if _M_IA64 || __IA64__ || __ia64__
#   define EP_ARCH_ITANIUM
# elif defined(_M_X64) || defined(_M_AMD64) || __X86_64__ || defined(__x86_64__)
#   define EP_ARCH_X64
# elif defined(_M_IX86) || defined(__i386) || defined(__i386__)
#   define EP_ARCH_X86
# else
#   error "Couldn't detect target architecture"
# endif

  // TODO: remove this from here!
  // we shouldn't have to parse windows.h for every file
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <Windows.h>
# include <Intrin.h>

# if defined(SendMessage)
#   undef SendMessage
# endif

# if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x501
#   undef _WIN32_WINNT
# endif
# if !defined(_WIN32_WINNT)
#   define _WIN32_WINNT 0x501   // This specifies WinXP or later - this makes a bunch of modern WIN32 API's available
# endif

#elif defined(__APPLE__)
# include <TargetConditionals.h>
# if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#   define EP_IPHONE

#   if TARGET_IPHONE_SIMULATOR
#     define EP_ARCH_X86
#   else
#     define EP_ARCH_ARM
#     define EP_32BIT
#     define EP_ENDIAN_LITTLE
#   endif

#   define USE_GLES
# elif TARGET_OS_MAC
#   define EP_OSX

#   if defined(__LP64__) || defined(_LP64)
#     define EP_64BIT
#   else
#     define EP_32BIT
#   endif

#   if defined(__x86_64__)
#     define EP_ARCH_X64
#   elif defined(__i386__)
#     define EP_ARCH_X86
#   elif defined(__ppc) || defined(__powerpc__) || defined(__PowerPC__) || defined(__PPC__) || defined(__ppc__) || defined(__ppc64__)
#     define EP_ARCH_PPC
#   else
#     error "Couldn't detect target architecture"
#   endif
# else
#   error "Unknown Apple product?"
# endif

#elif defined(ANDROID_NDK) || defined(__ANDROID__) || defined(ANDROID)
# define EP_ANDROID

# if defined(__arm__)
#   define EP_ARCH_ARM
#   define EP_32BIT
#   define EP_ENDIAN_LITTLE
# elif defined(__i386) || defined(__i386__)
#   define EP_ARCH_X86
# elif defined(_mips) || defined(__mips) || defined(__mips__)
#   define EP_ARCH_MIPS
#   define EP_32BIT
# else
    // TODO: x86_64, AArch64, MIPS64, have all been added in recent NDK's
#   error "Couldn't detect target architecture"
# endif

# define USE_GLES

#elif defined(EMSCRIPTEN)
# define EP_WEB

# define EP_ARCH_LLVM_IR
# define EP_32BIT
# define EP_ENDIAN_LITTLE

# define USE_GLES

#elif defined(_NACL)
# define EP_NACL

// TODO: check for pnacl...

# if defined(__x86_64__)
#   define EP_ARCH_X64
# elif defined(__i386__)
#   define EP_ARCH_X86
# else
    // TODO: what about arm?
#   error "Unknown architecture?!"
# endif

# define USE_GLES

#elif defined(__linux) || defined(__linux__) || defined(__gnu_linux__) || linux == 1
# define EP_LINUX

# include <memory.h>
# include <stdarg.h>

# if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
#   define EP_ARCH_X64
# elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
#   define EP_ARCH_X86
# else
    // hardly an error, linux could run on anything!
#   error "Couldn't detect target architecture!"
# endif

#elif defined(__SH4__) || defined(__SH4_SINGLE_ONLY__)
# define EP_ARCH_SH4
#elif defined(__arm__)
# define EP_ARCH_ARM
#elif defined(_MIPS_ARCH) || defined(_mips) || defined(__mips) || defined(__mips__) || defined(__MIPSEL__) || defined(_MIPSEL) || defined(__MIPSEL)
# define EP_ARCH_MIPS
#elif defined(__ppc64__)
# define EP_ARCH_PPC
# define EP_64BIT
#elif defined(__ppc) || defined(__powerpc__) || defined(__PowerPC__) || defined(__PPC__) || defined(__ppc__)
# define EP_ARCH_PPC
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
# define EP_ARCH_X64
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
# define EP_ARCH_X86
#else
# error "Couldn't detect target architecture!"
#endif

// if the architecture or platform didn't specify a data word size, try and detect one
#if !defined(EP_32BIT) && !defined(EP_64BIT)
# if defined(EP_ARCH_X86) || defined(EP_ARCH_SH4)
#   define EP_32BIT
# elif defined(EP_ARCH_X64) || defined(EP_ARCH_ITANIUM)
#   define EP_64BIT
# else
#   error "Unknown word length!"
# endif
#endif

// select architecture endian
#if !defined(EP_ENDIAN_BIG) && !defined(EP_ENDIAN_LITTLE)
# if defined(EP_ARCH_PPC) || defined(EP_ARCH_SPU) || defined(EP_ARCH_68K)
#   define EP_ENDIAN_BIG
# elif defined(EP_ARCH_X86) || defined(EP_ARCH_X64) || defined(EP_ARCH_ITANIUM) || defined(EP_ARCH_SH4)
#   define EP_ENDIAN_LITTLE
# else
#   error "Unable to detect target endian!"
# endif
#endif

// if the architecture didn't specify a platform, try and detect one
// TODO: this
//#if !defined(EP_PLATFORM)
//# error "Unknown platform!"
//#endif

// select asm format
#if defined(EP_COMPILER_VISUALC)
# define EP_ASM_INTEL
#else
# define EP_ASM_ATNT
#endif

// compiler wrangling
#if defined(EP_COMPILER_GCC)
# define __cdecl __attribute__((__cdecl__))
# define _cdecl __attribute__((__cdecl__))
#endif

// silence warnings
#if defined(EP_COMPILER_VISUALC)
# pragma warning(disable:4127) // conditional expression is constant
# pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
# pragma warning(disable:4458) // declaration of 'name' hides class member   --- TODO: remove this! but we need to decide on naming convention...
# pragma warning(disable: 4100) // unreferenced formal parameter
# pragma warning(disable: 4714) // marked as __forceinline not inlined
//# pragma warning(disable:4100) // disable 'unreferenced formal parameter'
//# pragma warning(disable:4996) // disable depreciated warnings
//# pragma warning(disable:4190) // disable C-linkage returning UDT (user data type)
#elif defined(EP_COMPILER_GCC)
# pragma GCC diagnostic ignored "-Wtype-limits"       // disable (char < 256) complaints  TODO: REMOVE ME!!!
# pragma GCC diagnostic ignored "-Wstrict-aliasing"   // disable strict-aliasing complaint
# if EP_GCC_VERSION < 50101
#   pragma GCC diagnostic ignored "-Wmissing-field-initializers" // missing initializer
# endif // GCC_VERSION < 50101
//# pragma GCC diagnostic ignored "-Wformat-security"   // stop complaining about variable format strings
//# pragma GCC diagnostic ignored "-Wunused-parameter"  // stop complaining about unused parameters
//# pragma GCC diagnostic ignored "-Wmissing-field-initializers" // stop complaining about missing field initialisers
#elif defined(EP_COMPILER_CLANG)
//# pragma clang diagnostic ignored "-Wreturn-type-c-linkage" // disable C-linkage returning UDT (user data type)
//# pragma clang diagnostic ignored "-Wformat-security"       // stop complaining about variable format strings
#endif

#if defined(EP_COMPILER_VISUALC)
# define EP_EXPORT __declspec(dllexport)
# define EP_EXPORT_VARIABLE __declspec(dllexport)
# if defined(_WINDLL)
#   define EP_SHAREDLIB 1
# endif
#else
# define EP_EXPORT
# define EP_EXPORT_VARIABLE
#endif

#if EP_SHAREDLIB
# define EP_API extern "C" EP_EXPORT
#else
# define EP_API extern "C"
#endif

#if !defined(NULL) // In case stdlib.h hasn't been included
#define NULL 0
#endif

#if defined(EP_COMPILER_CLANG) || defined(EP_COMPILER_GCC)
# if !EP_CPP11 && !defined(nullptr)
#   define nullptr NULL
# endif
#endif

#if EP_CPP11 && !defined(EP_COMPILER_VISUALC)
# include <cstddef>
using std::nullptr_t;
#endif

template<typename T>
struct epTheTypeIs;


// helper macros

#if defined(_DEBUG)
# define EP_DEBUG 1
# define EP_RELEASE 0
#else
# define EP_DEBUG 0
# define EP_RELEASE 1
#endif

#if EP_DEBUG
# define EPASSUME(condition) EPASSERT((condition), "Bad assumption: "##condition)
# define EPUNREACHABLE EPASSERT(false, "Shouldn't be here!")
#endif

#if defined(EP_COMPILER_VISUALC)
# define epalign_begin(n) __declspec(align(n))
# define epalign_end(n)
# define epdepricated_begin() __declspec(deprecated)
# define epdepricated_end(message)
# define epconst
# define eppure
# define epnothrow __declspec(nothrow)
# define eppacked
# define epforceinline __forceinline
# define epweak
# define epprintf_func(formatarg, vararg)
# define epprintf_method(formatarg, vararg)
# if !EP_DEBUG
#   define EPASSUME(condition) __assume(condition)
#   define EPUNREACHABLE __assume(0)
# endif
# define EPFMT_SIZE_T "%Iu"
# define EPFMT_SSIZE_T "%Id"
# define EPFMT_PTRDIFF_T "%Id"
#elif defined(EP_COMPILER_GCC) || defined(EP_COMPILER_CLANG)
  // TODO: use #if __has_builtin()/__has_attribute() to test if these are available?
# define epalign_begin(n)
# define epalign_end(n) __attribute__((aligned(n)))
# define epdepricated_begin()
# define epdepricated_end(message) __attribute__((deprecated(message)))
# define epconst __attribute__((const))
# define eppure __attribute__((pure))
# define epnothrow __attribute__((nothrow))
# define eppacked __attribute__((packed))
# define epforceinline inline __attribute__((always_inline))
# define epweak __attribute__((weak))
# define epprintf_func(formatarg, vararg) __attribute__((format(printf, formatarg, vararg)))
# define epprintf_method(formatarg, vararg) __attribute__((format(printf, formatarg + 1, vararg + 1)))
# if !EP_DEBUG
    // TODO: use #if __has_builtin(__builtin_unreachable) ??
#   define EPASSUME(condition) if(!(condition)) { __builtin_unreachable(); }
#   define EPUNREACHABLE __builtin_unreachable()
# endif
# define EPFMT_SIZE_T "%zu"
# define EPFMT_SSIZE_T "%zd"
# define EPFMT_PTRDIFF_T "%zd"
#else
# define epalign_begin(n)
# define epalign_end(n)
# define epdepricated_begin()
# define epdepricated_end(message)
# define epconst
# define eppure
# define epnothrow
# define eppacked
# define epforceinline inline
# define epweak
# define epprintf_func(formatarg, vararg)
# define epprintf_method(formatarg, vararg)
# if !EP_DEBUG
#   define EPASSUME(condition)
#   define EPUNREACHABLE
# endif
# if defined(EP_64BIT)
#   define EPFMT_SIZE_T "%llu"
#   define EPFMT_SSIZE_T "%lld"
#   define EPFMT_PTRDIFF_T "%lld"
# else
#   define EPFMT_SIZE_T "%u"
#   define EPFMT_SSIZE_T "%d"
#   define EPFMT_PTRDIFF_T "%d"
# endif
#endif

#define epUnused(x) (void)x

#if defined(__GNUC__)
# define epUnusedParam(x) x __attribute__((unused))
#else
# define epUnusedParam(x)
#endif

#define EPARRAYSIZE(_array) ( sizeof(_array) / sizeof((_array)[0]) )


// dependent headers
#include <stdint.h>
#include <stdlib.h>
#include <new>

//#include "ep/c/error.h"
#include "ep/c/string.h"


// Outputs a string to debug console
extern "C" {
void epDebugWrite(const char *pString);
void epDebugPrintf(const char *format, ...) epprintf_func(1, 2);
}


// debug stuff...
#if defined(__GNUC__)
# if EP_DEBUG
#   include <signal.h>
#   define __debugbreak() raise(SIGTRAP)
#   define DebugBreak() raise(SIGTRAP)
# else
#   define __debugbreak()
#   define DebugBreak()
# endif
#endif

#if EP_DEBUG

# define EPTRACE_ON     0     // Set to 1 to enable, set to 2 for printf on entry/exit of every function
# define EPASSERT_ON    1
# define EPRELASSERT_ON 1

#elif EP_RELEASE

#if !defined(EPTRACE_ON)
# define EPTRACE_ON     0
# endif
# define EPASSERT_ON    0
# define EPRELASSERT_ON 1

#endif

#if EPTRACE_ON
# define EPTRACE() udTrace __udtrace##__LINE__(__FUNCTION__, EPTRACE_ON)
# define EPTRACE_LINE() udTrace::Message("Line %d\n", __LINE__)
# define EPTRACE_SCOPE(id) udTrace __udtrace##__LINE__(id, EPTRACE_ON)
# define EPTRACE_MESSAGE(format,...) udTrace::Message(format,__VA_ARGS__)
# define EPTRACE_VARIABLE(var) udTrace_Variable(#var, var, __LINE__)
# define EPTRACE_MEMORY(var,length) udTrace_Memory(#var, var, length, __LINE__)
#else
# define EPTRACE()
# define EPTRACE_LINE()
# define EPTRACE_SCOPE(id)
# define EPTRACE_MESSAGE(format,...)
# define EPTRACE_VARIABLE(var)
# define EPTRACE_MEMORY(var,length)
#endif

// TODO: Make assertion system handle pop-up window where possible
#if EPASSERT_ON

extern "C" void epAssertFailed(epString condition, epString message, epString file, int line);

    // C assert just has a fixed message...
# define EPASSERT(condition, message) do { if (!(condition)) { epAssertFailed(#condition, message, __FILE__, __LINE__); DebugBreak(); } } while (0)
# define IF_EPASSERT(x) x

# if EPRELASSERT_ON
#   define EPRELASSERT(condition, message) do { if (!(condition)) { epAssertFailed(#condition, message, __FILE__, __LINE__); DebugBreak(); } } while (0)
#   define IF_EPRELASSERT(x) x
# else
#   define EPRELASSERT(condition, message) // TODO: Make platform-specific __assume(condition)
#   define IF_EPRELASSERT(x)
# endif

#else // EPASSERT_ON

# define EPASSERT(condition, ...) // TODO: Make platform-specific __assume(condition)
# define IF_EPASSERT(x)
# define EPRELASSERT(condition, ...) // TODO: Make platform-specific __assume(condition)
# define IF_EPRELASSERT(x)

#endif // EPASSERT_ON

#if EP_CPP11
# define EP_STATICASSERT(a_condition, a_error) static_assert(a_condition, #a_error)
#else
# if EP_WINDOWS
#   define EP_STATICASSERT(a_condition, a_error) typedef char EP_STATICASSERT##a_error[(a_condition)?1:-1]
# else
#   define EP_STATICASSERT(a_condition, a_error) typedef char EP_STATICASSERT##a_error[(a_condition)?1:-1] __attribute__ ((unused))
# endif
#endif


// HACK: rejig these
#define __MEMORY_DEBUG__  0

#if __MEMORY_DEBUG__
# define IF_MEMORY_DEBUG(x,y) ,x,y
#else
# define IF_MEMORY_DEBUG(x,y)
#endif //  __MEMORY_DEBUG__

extern "C" {
enum epAllocationFlags
{
  epAF_None = 0,
  epAF_Zero = 1
};

void *_epAlloc(size_t size, epAllocationFlags flags = epAF_None IF_MEMORY_DEBUG(const char * pFile = __FILE__, int  line = __LINE__));
#define epAlloc(size) _epAlloc(size, epAF_None IF_MEMORY_DEBUG(__FILE__, __LINE__))

void *_epAllocAligned(size_t size, size_t alignment, epAllocationFlags flags IF_MEMORY_DEBUG(const char * pFile = __FILE__, int  line = __LINE__));
#define epAllocAligned(size, alignment, flags) _epAllocAligned(size, alignment, flags IF_MEMORY_DEBUG(__FILE__, __LINE__))

#define epAllocFlags(size, flags) _epAlloc(size, flags IF_MEMORY_DEBUG(__FILE__, __LINE__))
#define epAllocType(type, count, flags) (type*)_epAlloc(sizeof(type) * (count), flags IF_MEMORY_DEBUG(__FILE__, __LINE__))

void _epFree(void *pMemory IF_MEMORY_DEBUG(const char * pFile = __FILE__, int  line = __LINE__));
#define epFree(pMemory) _epFree(pMemory IF_MEMORY_DEBUG(__FILE__, __LINE__))
}


#endif // _EP_PLATFORM_H
