#include "components/nativepluginloader.h"
#include "ep/cpp/kernel.h"

#include "ep/cpp/plugin.h"
#include "ep/cpp/component/component.h"

#include "hal/library.h"

extern "C" {
  typedef bool (epPlugin_GetInfoProc)(ep::PluginInfo *pInfo);
  typedef bool (epPlugin_InitProc)(ep::Instance *pPlugin);
}

// ----- Everything we need! -----

namespace ep {

Slice<const String> NativePluginLoader::getSupportedExtensions() const
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

bool NativePluginLoader::loadPlugin(String filename)
{
  epLibrary libHandle;
  if (!epLibrary_Open(&libHandle, filename.toStringz()))
  {
    logError("Failed to load plugin '{0}': '{1}'", filename, epLibrary_GetLastError());

    // TODO: we return true to prevent reloading the plugin - this is only valid if there's a dependency issue
    // this should probably be reworked (and potentially throw?) once dependency info is defined somewhere
    return true;
  }

  epPlugin_GetInfoProc *pGetInfo = (epPlugin_GetInfoProc*)epLibrary_GetFunction(libHandle, "epPlugin_GetInfo");
  epPlugin_InitProc *pInit = (epPlugin_InitProc*)epLibrary_GetFunction(libHandle, "epPlugin_Init");

  // If there's no init function then assume it's not a plugin and ignore
  if (!pInit)
  {
    logDebug(2, "Detected library '{0}' is not a valid Platform Plugin. Ignoring...", filename);
    epLibrary_Close(libHandle);

    // TODO: we return true to prevent reloading the plugin in the case that this dll is invalid - this is only valid if there's a dependency issue
    // this should probably be reworked (and potentially throw?) once dependency info is defined somewhere
    return true;
  }

  // Check that the plugin is compatible
  // The first (pre-epPlugin_GetInfo) API version was 0.9.0
  PluginInfo info = { "0.9.0" };
  if (!pGetInfo || pGetInfo(&info))
  {
    if (!epVersionIsCompatible(info.apiVersion, EP_APIVERSION))
    {
      logWarning(2, "Failed to load plugin '{0}': Plugin version '{1}' is incompatible. Version '{2}' is required.", filename, info.apiVersion, EP_APIVERSION);
      epLibrary_Close(libHandle);

      // TODO: we return true to prevent reloading the plugin in the case that this dll is invalid - this is only valid if there's a dependency issue
      // this should probably be reworked (and potentially throw?) once dependency info is defined somewhere
      return true;
    }
  }

  bool bSuccess = false;

  // Try to initialise the plugin
  try
  {
    bSuccess = pInit(s_pInstance);
  }
  catch(std::exception &e)
  {
    logError("Unhandled exception from epPlugin_Init() while loading plugin {0}: {1}", filename, e.what());
  }
  catch(...)
  {
    logError("Unhandled C++ exception from epPlugin_Init() while loading plugin {0}!", filename);
  }

  if (!bSuccess)
  {
    logError("Failed to load plugin {0}!", filename);
    epLibrary_Close(libHandle);
  }

  // TODO: add plugin to plugin registry
  // TODO: plugin component types need to be associated with the plugin, so when the plugin unloads, the component types can be removed

  return bSuccess;
}

} // namespace ep
