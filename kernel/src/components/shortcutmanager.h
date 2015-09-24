#pragma once
#ifndef EP_SHORTCUT_MANAGER_H
#define EP_SHORTCUT_MANAGER_H

#include "component.h"
#include "ep/epdelegate.h"

namespace ep
{

PROTOTYPE_COMPONENT(ShortcutManager);

class ShortcutManager : public Component
{
public:
  EP_COMPONENT(ShortcutManager);

  bool SetShortcutString(epString id, epString shortcutString);
  epSharedString GetShortcutString(epString id) const;
  bool RegisterShortcut(epString id, epString shortcutString, bool bFailIfExists = false);
  void UnregisterShortcut(epString id);
  bool HandleShortcutEvent(epString shortcutString);
  bool SetShortcutFunction(epString id, epDelegate<void()> func);
  bool SetShortcutScript(epString id, epString script);

protected:
  ShortcutManager(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);
  epString StripWhitespace(epSlice<char> output, epString input);

  struct Shortcut
  {
    Shortcut(epSharedString id, epSharedString shortcutString) : id(id), shortcutString(shortcutString) {}

    epSharedString id;
    epSharedString shortcutString;
    epSharedString script;
    epDelegate<void()> func;
  };

  epAVLTree<epSharedString, struct Shortcut> shortcutRegistry;
};

} //namespace ep

#endif // EP_SHORTCUT_MANAGER_H
