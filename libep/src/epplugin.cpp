#include "libep_internal.h"
#include "ep/cpp/plugin.h"
#include "ep/cpp/filesystem.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/hashmap.h"

#include "ep/cpp/component/resource/material.h"

namespace ep {

namespace internal {

RefCounted* getStaticImpl(String name)
{
  return (*(HashMap<SharedString, UniquePtr<RefCounted>>*)s_pInstance->pStaticImplRegistry)[name].get();
}

void addStaticImpl(SharedString name, UniquePtr<RefCounted> upImpl)
{
  ((HashMap<SharedString, UniquePtr<RefCounted>>*)s_pInstance->pStaticImplRegistry)->insert(name, std::move(upImpl));
}

} // namespace internal

Instance *s_pInstance = nullptr;

const char * const Material::s_shaderNames[] = {
  "vertexShader",
  "pixelShader",
  "geometryShader",
  "tesselationControlShader",
  "tesselationEvaluationShader",
  "computeShader"
};

// TODO: should move this to some other cpp?
Directory::Directory(String searchPattern)
{
  pDirectoryHandle = s_pInstance->pFind(searchPattern, nullptr, &fd);
}
Directory::~Directory()
{
  if (pDirectoryHandle)
    s_pInstance->pFind(nullptr, pDirectoryHandle, nullptr);
}
FindData Directory::popFront()
{
  FindData t = fd;
  pDirectoryHandle = s_pInstance->pFind(nullptr, pDirectoryHandle, &fd);
  return t;
}

Variant epToVariant(const FindData& fd)
{
  return Variant::VarMap{
    { "filename", fd.filename },
    { "path", fd.path },
    { "filesize", fd.fileSize },
    { "attributes", fd.attributes },
    { "writetime", fd.writeTime },
    { "accesstime", fd.accessTime }
  };
}

} // namespace ep

extern "C" {

bool epPluginAttach() epweak;

#if defined(EP_COMPILER_VISUALC)
bool epPluginAttachWeak()
{
  return false;
}
# if defined(EP_ARCH_X86)
#   pragma comment(linker, "/alternatename:_epPluginAttach=_epPluginAttachWeak")
# else
#   pragma comment(linker, "/alternatename:epPluginAttach=epPluginAttachWeak")
# endif
#else
bool epPluginAttach()
{
  return false;
}
#endif

EP_EXPORT void epPlugin_GetInfo(ep::PluginInfo *pInfo)
{
  pInfo->apiVersion = EP_APIVERSION;
}

EP_EXPORT bool epPlugin_Init(ep::Instance *pPlugin)
{
  ep::s_pInstance = pPlugin;
  return epPluginAttach();
}

epKernel *epPlugin_GetKernel()
{
  return (epKernel*)ep::s_pInstance->pKernelInstance;
}

void* epPlugin_Alloc(size_t size, epAllocationFlags flags, const char *pFile, int line)
{
  return ep::s_pInstance->pAlloc(size, flags, pFile, line);
}

void epPlugin_Free(void *pMem)
{
  ep::s_pInstance->pFree(pMem);
}

}

#if 0 //defined(EP_WINDOWS) // TODO: why does the CRT hog this? we might need to do things here...
BOOLEAN WINAPI DllMain(IN HINSTANCE epUnusedParam(hDllHandle), IN DWORD nReason, IN LPVOID epUnusedParam(Reserved))
{
  BOOLEAN bSuccess = TRUE;

  // Perform global initialization.
  switch (nReason)
  {
    case DLL_PROCESS_ATTACH:
      // For optimization.
//      DisableThreadLibraryCalls(hDllHandle);
      break;

    case DLL_PROCESS_DETACH:
      break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }

  return bSuccess;
}
#endif
