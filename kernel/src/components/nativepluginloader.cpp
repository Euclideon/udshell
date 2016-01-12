#include "components/nativepluginloader.h"
#include "kernel.h"

#include "ep/c/plugin.h"
#include "ep/c/internal/kernel_inl.h"
#include "ep/cpp/component/component.h"

extern "C" {
  typedef bool (epPlugin_InitProc)(epPluginInstance *pPlugin);
}

namespace kernel {

// HAX: Kernel::GetPluginInterface() defined here to save a whole bunch of extern's and header pollution
epPluginInstance *Kernel::GetPluginInterface()
{
  if (!pPluginInstance)
  {
    pPluginInstance = new epPluginInstance;

    pPluginInstance->apiVersion = EPKERNEL_APIVERSION;

    pPluginInstance->pKernelInstance = (epKernel*)this;

    pPluginInstance->Alloc = [](size_t size) -> void*
    {
      return epAlloc(size);
    },
      pPluginInstance->AllocAligned = [](size_t size, size_t alignment) -> void*
    {
      return epAllocAligned(size, alignment, epAF_None);
    },
      pPluginInstance->Free = [](void *pMem) -> void
    {
      epFree(pMem);
    },

      pPluginInstance->AssertFailed = [](epString condition, epString message, epString file, int line) -> void
    {
#if EPASSERT_ON
      epAssertFailed(condition, message, file, line);
#endif
    },

      pPluginInstance->DestroyComponent = [](epComponent *pInstance) -> void
    {
      // NOTE: this was called when an RC reached zero...
      Component *pC = (Component*)pInstance;
      pC->DecRef(); //       and then dec it with the internal function which actually performs the cleanup
    };
  }
  return pPluginInstance;
}

} // namespace kernel

// ----- Everything we need! -----

namespace ep {

Slice<const String> NativePluginLoader::GetSupportedExtensions() const
{
#if defined(EP_WINDOWS)
  static Array<const String> s_ext = { ".dll" };
#elif defined(EP_OSX) || defined(EP_IPHONE)
  static Array<const String> s_ext = { ".dylib" };
#else
  static Array<const String> s_ext = { ".so" };
#endif
  return s_ext;
}

bool NativePluginLoader::LoadPlugin(String filename)
{
#if defined(EP_WINDOWS)
  // Convert UTF-8 to UTF-16 -- TODO use UD helper functions or add some to hal?
  int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename.ptr, (int)filename.length, nullptr, 0);
  wchar_t *widePath = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename.ptr, (int)filename.length, widePath, len) == 0)
    return false;
  widePath[len] = 0;

  // try and load library
  HMODULE hDll = LoadLibraryW(widePath);
  if (hDll == NULL)
    return false;
#if defined(EP_ARCH_X86)
  const char *pFuncName = "_epPlugin_Init";
#else
  const char *pFuncName = "epPlugin_Init";
#endif
  epPlugin_InitProc *pInit = (epPlugin_InitProc*)GetProcAddress(hDll, pFuncName);
  if (!pInit)
  {
    FreeLibrary(hDll);
    return false;
  }
#else
  epPlugin_InitProc *pInit = nullptr;

  EPASSERT(false, "Not yet supported!");

  if (!pInit)
    return false;
#endif

  bool bSuccess = pInit(((kernel::Kernel&)GetKernel()).GetPluginInterface());

  if (!bSuccess)
  {
#if defined(EP_WINDOWS)
    FreeLibrary(hDll);
#endif
  }

  // TODO: add plugin to plugin registry
  // TODO: plugin component types need to be associated with the plugin, so when the plugin unloads, the component types can be removed

  return bSuccess;
}

} // namespace ep
