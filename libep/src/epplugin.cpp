#include "ep/cpp/plugin.h"
#include "ep/cpp/filesystem.h"
#include "ep/cpp/variant.h"

namespace ep {

Instance *s_pInstance = nullptr;

// TODO: should move this to some other cpp?
Directory::Directory(String searchPattern)
{
  pDirectoryHandle = s_pInstance->Find(searchPattern, nullptr, &fd);
}
Directory::~Directory()
{
  if (pDirectoryHandle)
    s_pInstance->Find(nullptr, pDirectoryHandle, nullptr);
}
FindData Directory::PopFront()
{
  FindData t = fd;
  pDirectoryHandle = s_pInstance->Find(nullptr, pDirectoryHandle, &fd);
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
  return ep::s_pInstance->Alloc(size, flags, pFile, line);
}

void epPlugin_Free(void *pMem)
{
  ep::s_pInstance->Free(pMem);
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
