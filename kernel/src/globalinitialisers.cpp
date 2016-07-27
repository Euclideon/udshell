#if defined(_MSC_VER)
#pragma warning(disable : 4073) // initializers put in library initialization area
#pragma init_seg(lib)
#endif // _MSC_VER

#include "ep/cpp/platform.h"
#include "ep/cpp/plugin.h"
#include "ep/cpp/filesystem.h"
#include "hal/directory.h"
#include "kernelimpl.h"

#if defined(EP_LINUX)
#define EP_INIT_PRIORITY(x) __attribute__ ((init_priority (x)))
#else
#define EP_INIT_PRIORITY(x)
#endif

extern "C" {
  void epInternalInit();
}

namespace ep {
namespace internal {

  // HACK !!! (GCC and Clang)
  // For the implementation of epInternalInit defined in this file to override
  // the weak version in epplatform.cpp at least one symbol from this file must
  // be referenced externally.  This has been implemented in kernelimpl.cpp inside
  // createInstance().
  void *GetStaticImplRegistry()
  {
    if (!KernelImpl::s_pStaticImplRegistry)
      KernelImpl::s_pStaticImplRegistry = epNew(KernelImpl::StaticImplRegistryMap);
    return KernelImpl::s_pStaticImplRegistry;
  }

  static void *GetVarAVLAllocator()
  {
    if (!KernelImpl::s_pVarAVLAllocator)
      KernelImpl::s_pVarAVLAllocator = epNew(KernelImpl::VarAVLTreeAllocator);
    return KernelImpl::s_pVarAVLAllocator;
  }

  static void *GetWeakRefRegistry()
  {
    if (!KernelImpl::s_pWeakRefRegistry)
      KernelImpl::s_pWeakRefRegistry = epNew(KernelImpl::WeakRefRegistryMap, 65536);
    return KernelImpl::s_pWeakRefRegistry;
  }

  static void Destroy()
  {
    epDelete(KernelImpl::s_pVarAVLAllocator);
    epDelete(KernelImpl::s_pStaticImplRegistry);
    epDelete(KernelImpl::s_pWeakRefRegistry);
  }

  static ErrorSystem* GetErrorSystem()
  {
    static ErrorSystem errorSystem;
    return &errorSystem;
  }

struct GlobalInstanceInitializer
{
  GlobalInstanceInitializer();

  ~GlobalInstanceInitializer()
  {
    Destroy();
  }
};

static GlobalInstanceInitializer EP_INIT_PRIORITY(101) globalInstanceInitializer;

static void TranslateFindData(const EPFindData &fd, FindData *pFD)
{
  pFD->filename = (const char*)fd.pFilename;
  pFD->path = (const char*)fd.pSystemPath;
  pFD->attributes = ((fd.attributes & EPFA_Directory) ? FileAttributes::Directory : 0) |
    ((fd.attributes & EPFA_SymLink) ? FileAttributes::SymLink : 0) |
    ((fd.attributes & EPFA_Hidden) ? FileAttributes::Hidden : 0) |
    ((fd.attributes & EPFA_ReadOnly) ? FileAttributes::ReadOnly : 0);
  pFD->fileSize = fd.fileSize;
  pFD->accessTime.ticks = fd.accessTime.ticks;
  pFD->writeTime.ticks = fd.writeTime.ticks;
}

static void *_Alloc(size_t size, epAllocationFlags flags, const char * pFile, int line)
{
#if defined(EP_COMPILER_VISUALC)
# if __EP_MEMORY_DEBUG__
  void *pMemory = (flags & epAF_Zero) ? _recalloc_dbg(nullptr, 1, size, _NORMAL_BLOCK, pFile, line) : _malloc_dbg(size, _NORMAL_BLOCK, pFile, line);
# else
  void *pMemory = (flags & epAF_Zero) ? _recalloc(nullptr, 1, size) : malloc(size);
# endif // __EP_MEMORY_DEBUG__
#else // defined(EP_COMPILER_VISUALC)
  epUnused(pFile);
  epUnused(line);
  void *pMemory = (flags & epAF_Zero) ? calloc(1, size) : malloc(size);
#endif
  return pMemory;
}

static void _Free(void *pMemory)
{
  if (pMemory)
    free(pMemory);
}

static void _Assert(String condition, String message, String file, int line)
{
   IF_EPASSERT(AssertFailed(condition, message, file, line);)
}

static void _DestroyComponent(Component *pInstance)
{
  pInstance->DecRef();
}

static void *_Find (String pattern, void *pHandle, void *pData)
{
  EPFind *pFind = (EPFind*)pHandle;
  FindData *pFD = (FindData*)pData;
  if (pattern && !pFind && pFD)
  {
    EPFind *find = epNew(EPFind);
    EPFindData fd;
    if (HalDirectory_FindFirst(find, pattern.toStringz(), &fd))
    {
      internal::TranslateFindData(fd, pFD);
      return find;
    }
    else
    {
      epDelete(find);
      return nullptr;
    }
  }
  else if (!pattern && pFind && pFD)
  {
    EPFindData fd;
    if (HalDirectory_FindNext(pFind, &fd))
    {
      internal::TranslateFindData(fd, pFD);
      return pHandle;
    }
    else
    {
      HalDirectory_FindClose(pFind);
      epDelete(pFind);
      return nullptr;
    }
  }
  else if (!pattern && pFind && !pFD)
  {
    HalDirectory_FindClose(pFind);
    epDelete(pFind);
    return nullptr;
  }
  else
    EPTHROW_ERROR(Result::InvalidArgument, "Bad call");
}

static Instance s_instance =
{
  EP_APIVERSION,  // apiVersion;
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  internal::_Alloc,
  internal::_Free,
  internal::_Assert,
  internal::_DestroyComponent,
  internal::_Find
};

GlobalInstanceInitializer::GlobalInstanceInitializer()
{
  epInternalInit();
  ep::internal::s_instance.pErrorSystem = ep::internal::GetErrorSystem();
  ep::internal::s_instance.pStaticImplRegistry = ep::internal::GetStaticImplRegistry();
  ep::internal::s_instance.pTreeAllocator = ep::internal::GetVarAVLAllocator(),
  ep::internal::s_instance.pWeakRegistry = ep::internal::GetWeakRefRegistry();
}

} // namespace internal
} //namespace ep

extern "C" {

void epInternalInit()
{
  if (!ep::s_pInstance)
    ep::s_pInstance = &ep::internal::s_instance;
}

} // extern "C"

