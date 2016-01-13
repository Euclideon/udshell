#include "components/nativepluginloader.h"
#include "kernel.h"

#include "ep/cpp/plugin.h"
#include "ep/cpp/component/component.h"

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

  bool bSuccess = pInit(s_pInstance);

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
