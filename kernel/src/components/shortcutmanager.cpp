#include "components/shortcutmanager.h"
#include "kernel.h"

namespace ep
{
static CMethodDesc methods[] =
{
  {
    {
      "setshortcutstring", // id
      "Set the shortcut string for the given operation id", // description
    },
    &ShortcutManager::SetShortcutString, // method
  },
  // TODO - uncomment this once epVariant can be constructed from a epSharedString
  /*{
    {
      "getshortcutstring", // id
      "Get the shortcut string for the given operation id", // description
    },
    &ShortcutManager::GetShortcutString, // method
  },*/
  {
    {
      "registershortcut", // id
      "register an operation and assign it a shortcut string", // description
    },
    &ShortcutManager::RegisterShortcut, // method
  },
  {
    {
      "unregistershortcut", // id
      "remove an operation from the shortcut manager", // description
    },
    &ShortcutManager::UnregisterShortcut, // method
  },
  {
    {
      "handleshortcutevent", // id
      "call the function or script attached to the given shortcut string", // description
    },
    &ShortcutManager::HandleShortcutEvent, // method
  },
  {
    {
      "setshortcutfunction", // id
      "Assign a callback function to the given shortcut operation", // description
    },
    &ShortcutManager::SetShortcutFunction, // method
  },
  {
    {
      "setshortcutscript", // id
      "Assign a script string to the given shortcut operation", // description
    },
    &ShortcutManager::SetShortcutScript, // method
  },
};

ComponentDesc ShortcutManager::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "shortcutmanager", // id
  "Shortcut Manager", // displayName
  "Registers keyboard shortcuts and handles shortcut events", // description

  nullptr, // properties
  epSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr, // events
};

ShortcutManager::ShortcutManager(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{}

epSharedString ShortcutManager::GetShortcutString(epString id) const
{
  const Shortcut *pSh = shortcutRegistry.Get(id);
  if (!pSh)
    return nullptr;
  return
    pSh->shortcutString;
}

bool ShortcutManager::SetShortcutString(epString id, epString shortcutString)
{
  Shortcut *sh = shortcutRegistry.Get(id);
  if (!sh)
  {
    LogWarning(2, "Can't bind shortcut \"{0}\" to operation \"{1}\" - \"{1}\" doesn't exist", shortcutString, id);
    return false;
  }

  sh->shortcutString = shortcutString;

  return true;
}

bool ShortcutManager::RegisterShortcut(epString id, epString shortcutString, bool bFailIfExists)
{
  if (bFailIfExists && shortcutRegistry.Get(id))
  {
    LogWarning(5, "Shortcut registration failed - \"{0}\" already exists", id);
    return false;
  }

  for (Shortcut &sh : shortcutRegistry)
  {
    if (!shortcutString.cmpIC(sh.shortcutString) && id.cmp(sh.id))
    {
      LogWarning(2, "Can't bind shortcut \"{0}\" to operation \"{1}\". Already bound to \"{2}\"", shortcutString, id, sh.id);
      return false;
    }
  }

  epMutableString<256> mShortcut;
  mShortcut.reserve(shortcutString.length);
  mShortcut.length = shortcutString.length;
  StripWhitespace(mShortcut, shortcutString);

  shortcutRegistry.Insert(id, Shortcut(id, shortcutString));

  return true;
}

epString ShortcutManager::StripWhitespace(epSlice<char> output, epString input)
{
  size_t len = 0;
  for (int i = 0; i < input.length; i++)
  {
    if (!isspace(input[i]))
      output[len++] = input[i];
  }
  return output.slice(0, len);
}

void ShortcutManager::UnregisterShortcut(epString id)
{
  shortcutRegistry.Remove(id);
}

bool ShortcutManager::HandleShortcutEvent(epString shortcutString)
{
  for (Shortcut &sh : shortcutRegistry)
  {
    if (!shortcutString.cmpIC(sh.shortcutString))
    {
      if (sh.func)
        sh.func();
      else if (!sh.script.empty())
        pKernel->Exec(sh.script);

      return true;
    }
  }

  return false;
}

bool ShortcutManager::SetShortcutFunction(epString id, epDelegate<void()> func)
{
  Shortcut *sh = shortcutRegistry.Get(id);
  if (!sh)
  {
    LogWarning(2, "Can't bind function to shortcut operation \"{0}\", operation doesn't exist", id);
    return false;
  }

  sh->func = func;
  sh->script = nullptr;

  return true;
}

bool ShortcutManager::SetShortcutScript(epString id, epString script)
{
  Shortcut *sh = shortcutRegistry.Get(id);
  if (!sh)
  {
    LogWarning(2, "Can't bind script to shortcut operation \"{0}\", operation doesn't exist", id);
    return false;
  }

  sh->script = script;
  sh->func = nullptr;

  return true;
}

} // namespace ep
