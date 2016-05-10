#include "components/nativepluginloader.h"
#include "ep/cpp/kernel.h"

#include "ep/cpp/plugin.h"
#include "ep/cpp/component/component.h"

#if defined(EP_LINUX)
# include <dlfcn.h>
#endif

extern "C" {
  typedef bool (epPlugin_InitProc)(ep::Instance *pPlugin);
}

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
#if defined(EP_ARCH_X86)
  const char *pFuncName = "_epPlugin_Init";
#else
  const char *pFuncName = "epPlugin_Init";
#endif

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
  {
    LogError("Unable to load plugin '{0}' - error code '{1}'", filename, (uint32_t)GetLastError());

    // TODO: we return true to prevent reloading the plugin - this is only valid if there's a dependency issue
    // this should probably be reworked (and potentially throw?) once dependency info is defined somewhere
    return true;
  }

  epPlugin_InitProc *pInit = (epPlugin_InitProc*)GetProcAddress(hDll, pFuncName);
  if (!pInit)
  {
    FreeLibrary(hDll);

    // TODO: we return true to prevent reloading the plugin - this is only valid if there's a dependency issue
    // this should probably be reworked (and potentially throw?) once dependency info is defined somewhere
    return true;
  }

  bool bSuccess = pInit(s_pInstance);
  if (!bSuccess)
    FreeLibrary(hDll);

  // TODO: add plugin to plugin registry
  // TODO: plugin component types need to be associated with the plugin, so when the plugin unloads, the component types can be removed

  return bSuccess;

#elif defined(EP_LINUX)

  void *hSo = dlopen(filename.toStringz(), RTLD_NOW);
  if (hSo == NULL)
  {
    LogError("Unable to load plugin '{0}' - error '{1}'", filename, dlerror());

    // TODO: we return true to prevent reloading the plugin - this is only valid if there's a dependency issue
    // this should probably be reworked (and potentially throw?) once dependency info is defined somewhere
    return true;
  }

  epPlugin_InitProc *pInit = (epPlugin_InitProc*)dlsym(hSo, pFuncName);
  if (!pInit)
  {
    dlclose(hSo);

    // TODO: we return true to prevent reloading the plugin - this is only valid if there's a dependency issue
    // this should probably be reworked (and potentially throw?) once dependency info is defined somewhere
    return true;
  }

  bool bSuccess = pInit(s_pInstance);
  if (!bSuccess)
    dlclose(hSo);

  // TODO: add plugin to plugin registry
  // TODO: plugin component types need to be associated with the plugin, so when the plugin unloads, the component types can be removed

  return bSuccess;

#else

  epUnused(pFuncName);

  LogError("Platform has no shared-library support!");
  return false;

#endif
}

} // namespace ep
